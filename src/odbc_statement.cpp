/*
  Copyright (c) 2013, Dan VerWeire<dverweire@gmail.com>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <string.h>
#include <v8.h>
#include <node.h>
#include <node_version.h>
#include <time.h>
#include <uv.h>

#include "odbc.h"
#include "odbc_connection.h"
#include "odbc_result.h"
#include "odbc_statement.h"

using namespace v8;
using namespace node;

Nan::Persistent<Function> ODBCStatement::constructor;

void ODBCStatement::Init(v8::Handle<Object> exports) {
  DEBUG_PRINTF("ODBCStatement::Init\n");
  Nan::HandleScope scope;

  Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);

  // Constructor Template
  
  t->SetClassName(Nan::New("ODBCStatement").ToLocalChecked());

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = t->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);
  
  // Prototype Methods
  Nan::SetPrototypeMethod(t, "execute", Execute);
  Nan::SetPrototypeMethod(t, "executeSync", ExecuteSync);
  
  Nan::SetPrototypeMethod(t, "executeDirect", ExecuteDirect);
  Nan::SetPrototypeMethod(t, "executeDirectSync", ExecuteDirectSync);
  
  Nan::SetPrototypeMethod(t, "executeNonQuery", ExecuteNonQuery);
  Nan::SetPrototypeMethod(t, "executeNonQuerySync", ExecuteNonQuerySync);
  
  Nan::SetPrototypeMethod(t, "prepare", Prepare);
  Nan::SetPrototypeMethod(t, "prepareSync", PrepareSync);
  
  Nan::SetPrototypeMethod(t, "bind", Bind);
  Nan::SetPrototypeMethod(t, "bindSync", BindSync);
  
  Nan::SetPrototypeMethod(t, "closeSync", CloseSync);

  // Attach the Database Constructor to the target object
  constructor.Reset(t->GetFunction());
  exports->Set(Nan::New("ODBCStatement").ToLocalChecked(), t->GetFunction());
}

ODBCStatement::~ODBCStatement() {
  this->Free();
}

void ODBCStatement::Free() {
  DEBUG_PRINTF("ODBCStatement::Free paramCount = %i, m_hSTMT =%X\n", paramCount, m_hSTMT);
  //if we previously had parameters, then be sure to free them
  if (paramCount) {
      FREE_PARAMS( params, paramCount ) ;
      DEBUG_PRINTF("ODBCStatement::Free - Params Freed.\n");
  }
  
  if (m_hSTMT) {
    SQLFreeHandle(SQL_HANDLE_STMT, m_hSTMT);
    m_hSTMT = (SQLHSTMT)NULL;
  }
    
  if (bufferLength > 0) {
      if(buffer) free(buffer);
      buffer = NULL;
      bufferLength = 0;
  }
  DEBUG_PRINTF("ODBCStatement::Free() Done.\n");
}

NAN_METHOD(ODBCStatement::New) {
  DEBUG_PRINTF("ODBCStatement::New\n");
  Nan::HandleScope scope;
  
  REQ_EXT_ARG(0, js_henv);
  REQ_EXT_ARG(1, js_hdbc);
  REQ_EXT_ARG(2, js_hstmt);
  
  SQLHENV hENV = static_cast<SQLHENV>((intptr_t)js_henv->Value());
  SQLHDBC hDBC = static_cast<SQLHDBC>((intptr_t)js_hdbc->Value());
  SQLHSTMT hSTMT = static_cast<SQLHSTMT>((intptr_t)js_hstmt->Value());
  
  //create a new OBCResult object
  ODBCStatement* stmt = new ODBCStatement(hENV, hDBC, hSTMT);
  
  //specify the buffer length
  stmt->bufferLength = MAX_VALUE_SIZE;
  
  //initialze a buffer for this object
  stmt->buffer = (uint16_t *) malloc(stmt->bufferLength+2);
  MEMCHECK( stmt->buffer );

  //set the initial colCount to 0
  stmt->colCount = 0;
  
  //initialize the paramCount
  stmt->paramCount = 0;
  stmt->params = 0;
  
  stmt->Wrap(info.Holder());
  
  info.GetReturnValue().Set(info.Holder());
}

/*
 * Execute
 */

NAN_METHOD(ODBCStatement::Execute) {
  DEBUG_PRINTF("ODBCStatement::Execute\n");
  
  Nan::HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  MEMCHECK( work_req );
  
  execute_work_data* data = 
    (execute_work_data *) calloc(1, sizeof(execute_work_data));
  MEMCHECK( data );

  data->cb = new Nan::Callback(cb);
  
  data->stmt = stmt;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(),
    work_req,
    UV_Execute,
    (uv_after_work_cb)UV_AfterExecute);

  stmt->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCStatement::UV_Execute(uv_work_t* req) {
  DEBUG_PRINTF("ODBCStatement::UV_Execute\n");
  
  execute_work_data* data = (execute_work_data *)(req->data);

  SQLRETURN ret;
  
  ret = SQLExecute(data->stmt->m_hSTMT); 

  data->result = ret;
}

void ODBCStatement::UV_AfterExecute(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecute\n");
  
  execute_work_data* data = (execute_work_data *)(req->data);
  Nan::HandleScope scope;
  int outParamCount = 0; // Non-zero tells its a SP with OUT param
  Local<Array> sp_result = Nan::New<Array>();
  
  //an easy reference to the statment object
  ODBCStatement* stmt = data->stmt->self();

  if (SQL_SUCCEEDED( data->result )) {
    for(int i = 0; i < stmt->paramCount; i++) { // For stored Procedure CALL
      if(stmt->params[i].paramtype % 2 == 0) {
        sp_result->Set(Nan::New(outParamCount), ODBC::GetOutputParameter(stmt->params[i]));
        outParamCount++;
      }
    }
  }
  if( stmt->paramCount ) {
    FREE_PARAMS( stmt->params, stmt->paramCount ) ;
  }
  
  //First thing, let's check if the execution of the query returned any errors 
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      SQL_HANDLE_STMT,
      stmt->m_hSTMT,
      data->cb);
  }
  else {
    Local<Value> info[4];
    bool* canFreeHandle = new bool(false);
    
    info[0] = Nan::New<External>((void*) (intptr_t) stmt->m_hENV);
    info[1] = Nan::New<External>((void*) (intptr_t) stmt->m_hDBC);
    info[2] = Nan::New<External>((void*) (intptr_t) stmt->m_hSTMT);
    info[3] = Nan::New<External>((void*)canFreeHandle);
    
    Local<Object> js_result = Nan::New<Function>(ODBCResult::constructor)->NewInstance(4, info);

    info[0] = Nan::Null();
    info[1] = js_result;

    if(outParamCount) {
        info[2] = sp_result; // Must a CALL stmt
    }
    else info[2] = Nan::Null();

    Nan::TryCatch try_catch;

    data->cb->Call(3, info);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
  }

  stmt->Unref();
  delete data->cb;
  
  free(data);
  free(req);
}

/*
 * ExecuteSync
 * 
 */

NAN_METHOD(ODBCStatement::ExecuteSync) {
  DEBUG_PRINTF("ODBCStatement::ExecuteSync\n");
  
  Nan::HandleScope scope;

  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  int outParamCount = 0;
  Local<Array> sp_result = Nan::New<Array>();

  SQLRETURN ret = SQLExecute(stmt->m_hSTMT); 

  if (SQL_SUCCEEDED(ret)) {
    for(int i = 0; i < stmt->paramCount; i++) { // For stored Procedure CALL
      if(stmt->params[i].paramtype % 2 == 0) {
        sp_result->Set(Nan::New(outParamCount), ODBC::GetOutputParameter(stmt->params[i]));
        outParamCount++;
      }
    }
  }
  if( stmt->paramCount ) {
    FREE_PARAMS( stmt->params, stmt->paramCount ) ;
  }
  
  if(ret == SQL_ERROR) {
    Nan::ThrowError(ODBC::GetSQLError(
      SQL_HANDLE_STMT,
      stmt->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCStatement::ExecuteSync"
    ));
    
    info.GetReturnValue().Set(Nan::Null());
  }
  else {
    Local<Value> result[4];
    bool* canFreeHandle = new bool(false);
    
    result[0] = Nan::New<External>((void*) (intptr_t) stmt->m_hENV);
    result[1] = Nan::New<External>((void*) (intptr_t) stmt->m_hDBC);
    result[2] = Nan::New<External>((void*) (intptr_t) stmt->m_hSTMT);
    result[3] = Nan::New<External>((void*)canFreeHandle);
    
    Local<Object> js_result = Nan::New(ODBCResult::constructor)->NewInstance(4, result);

    if( outParamCount ) // Its a CALL stmt with OUT params.
    {   // Return an array with outparams as second element. [result, outparams]
        Local<Array> resultset = Nan::New<Array>();
        resultset->Set(0, js_result);
        resultset->Set(1, sp_result);
        info.GetReturnValue().Set(resultset);
    }
    else
        info.GetReturnValue().Set(js_result);
  }
}

/*
 * ExecuteNonQuery
 */

NAN_METHOD(ODBCStatement::ExecuteNonQuery) {
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuery\n");
  
  Nan::HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  MEMCHECK( work_req );
  
  execute_work_data* data = 
    (execute_work_data *) calloc(1, sizeof(execute_work_data));
  MEMCHECK( data );

  data->cb = new Nan::Callback(cb);
  
  data->stmt = stmt;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(),
    work_req,
    UV_ExecuteNonQuery,
    (uv_after_work_cb)UV_AfterExecuteNonQuery);

  stmt->Ref();
  
  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCStatement::UV_ExecuteNonQuery(uv_work_t* req) {
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuery\n");
  
  execute_work_data* data = (execute_work_data *)(req->data);

  SQLRETURN ret;
  
  ret = SQLExecute(data->stmt->m_hSTMT); 

  data->result = ret;
}

void ODBCStatement::UV_AfterExecuteNonQuery(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuery\n");
  
  execute_work_data* data = (execute_work_data *)(req->data);
  
  Nan::HandleScope scope;
  
  //an easy reference to the statment object
  ODBCStatement* self = data->stmt->self();

  //First thing, let's check if the execution of the query returned any errors 
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      SQL_HANDLE_STMT,
      self->m_hSTMT,
      data->cb);
  }
  else {
    SQLLEN rowCount = 0;
    
    SQLRETURN ret = SQLRowCount(self->m_hSTMT, &rowCount);
    
    if (!SQL_SUCCEEDED(ret)) {
      rowCount = 0;
    }
    
    SQLFreeStmt(self->m_hSTMT, SQL_CLOSE);
    
    Local<Value> info[2];

    info[0] = Nan::Null();
    info[1] = Nan::New<Number>(rowCount);

    Nan::TryCatch try_catch;
    
    data->cb->Call(Nan::GetCurrentContext()->Global(), 2, info);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
  }

  self->Unref();
  delete data->cb;
  
  free(data);
  free(req);
}

/*
 * ExecuteNonQuerySync
 * 
 */

NAN_METHOD(ODBCStatement::ExecuteNonQuerySync) {
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuerySync\n");
  
  Nan::HandleScope scope;

  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  SQLRETURN ret = SQLExecute(stmt->m_hSTMT); 
  
  if(ret == SQL_ERROR) {
    Nan::ThrowError(ODBC::GetSQLError(
      SQL_HANDLE_STMT,
      stmt->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCStatement::ExecuteSync"
    ));
    
    info.GetReturnValue().Set(Nan::Null());
  }
  else {
    SQLLEN rowCount = 0;
    
    SQLRETURN ret = SQLRowCount(stmt->m_hSTMT, &rowCount);
    
    if (!SQL_SUCCEEDED(ret)) {
      rowCount = 0;
    }
    
    SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
    
    info.GetReturnValue().Set(Nan::New<Number>(rowCount));
  }
}

/*
 * ExecuteDirect
 * 
 */

NAN_METHOD(ODBCStatement::ExecuteDirect) {
  DEBUG_PRINTF("ODBCStatement::ExecuteDirect\n");
  
  Nan::HandleScope scope;

  REQ_STRO_ARG(0, sql);
  REQ_FUN_ARG(1, cb);

  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  MEMCHECK( work_req );
  
  execute_direct_work_data* data = 
    (execute_direct_work_data *) calloc(1, sizeof(execute_direct_work_data));
  MEMCHECK( data );

  data->cb = new Nan::Callback(cb);

  data->sqlLen = sql->Length();

#ifdef UNICODE
  data->sql = (uint16_t *) malloc((data->sqlLen * sizeof(uint16_t)) + sizeof(uint16_t));
  MEMCHECK( data->sql );
  sql->Write((uint16_t *) data->sql);
#else
  data->sql = (char *) malloc(data->sqlLen +1);
  MEMCHECK( data->sql );
  sql->WriteUtf8((char *) data->sql);
#endif

  data->stmt = stmt;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(),
    work_req, 
    UV_ExecuteDirect, 
    (uv_after_work_cb)UV_AfterExecuteDirect);

  stmt->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCStatement::UV_ExecuteDirect(uv_work_t* req) {
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteDirect\n");
  
  execute_direct_work_data* data = (execute_direct_work_data *)(req->data);

  SQLRETURN ret;
  
  ret = SQLExecDirect(
    data->stmt->m_hSTMT,
    (SQLTCHAR *) data->sql, 
    data->sqlLen);  

  data->result = ret;
}

void ODBCStatement::UV_AfterExecuteDirect(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteDirect\n");
  
  execute_direct_work_data* data = (execute_direct_work_data *)(req->data);
  
  Nan::HandleScope scope;
  
  //an easy reference to the statment object
  ODBCStatement* self = data->stmt->self();

  //First thing, let's check if the execution of the query returned any errors 
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      SQL_HANDLE_STMT,
      self->m_hSTMT,
      data->cb);
  }
  else {
    Local<Value> info[4];
    bool* canFreeHandle = new bool(false);
    
    info[0] = Nan::New<External>((void*) (intptr_t) self->m_hENV);
    info[1] = Nan::New<External>((void*) (intptr_t) self->m_hDBC);
    info[2] = Nan::New<External>((void*) (intptr_t) self->m_hSTMT);
    info[3] = Nan::New<External>((void*)canFreeHandle);
    
    //TODO persistent leak?
    Nan::Persistent<Object> js_result;
    js_result.Reset(Nan::New<Function>(ODBCResult::constructor)->NewInstance(4, info));

    info[0] = Nan::Null();
    info[1] = Nan::New(js_result);

    Nan::TryCatch try_catch;

    data->cb->Call(2, info);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
  }

  self->Unref();
  delete data->cb;
  
  free(data->sql);
  free(data);
  free(req);
}

/*
 * ExecuteDirectSync
 * 
 */

NAN_METHOD(ODBCStatement::ExecuteDirectSync) {
  DEBUG_PRINTF("ODBCStatement::ExecuteDirectSync\n");
  
  Nan::HandleScope scope;

#ifdef UNICODE
  REQ_WSTR_ARG(0, sql);
#else
  REQ_STR_ARG(0, sql);
#endif

  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  
  SQLRETURN ret = SQLExecDirect(
    stmt->m_hSTMT,
    (SQLTCHAR *) *sql, 
    sql.length());  

  if(ret == SQL_ERROR) {
    Nan::ThrowError(ODBC::GetSQLError(
      SQL_HANDLE_STMT,
      stmt->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCStatement::ExecuteDirectSync"
    ));
    
    info.GetReturnValue().Set(Nan::Null());
  }
  else {
   Local<Value> result[4];
    bool* canFreeHandle = new bool(false);
    
    result[0] = Nan::New<External>((void*) (intptr_t) stmt->m_hENV);
    result[1] = Nan::New<External>((void*) (intptr_t) stmt->m_hDBC);
    result[2] = Nan::New<External>((void*) (intptr_t) stmt->m_hSTMT);
    result[3] = Nan::New<External>((void*)canFreeHandle);
    
    //TODO persistent leak?
    Nan::Persistent<Object> js_result;
    js_result.Reset(Nan::New<Function>(ODBCResult::constructor)->NewInstance(4, result));
    
    info.GetReturnValue().Set(Nan::New(js_result));
	//info.GetReturnValue().Set(Nan::Null());
  }
}

/*
 * PrepareSync
 * 
 */

NAN_METHOD(ODBCStatement::PrepareSync) {
  DEBUG_PRINTF("ODBCStatement::PrepareSync\n");
  
  Nan::HandleScope scope;

  REQ_STRO_ARG(0, sql);

  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  SQLRETURN ret;

  int sqlLen = sql->Length() + 1;

#ifdef UNICODE
  uint16_t *sql2;
  sql2 = (uint16_t *) malloc(sqlLen * sizeof(uint16_t));
  MEMCHECK( sql2 );
  sql->Write(sql2);
#else
  char *sql2;
  sql2 = (char *) malloc(sqlLen);
  MEMCHECK( sql2 );
  sql->WriteUtf8(sql2);
#endif
  
  ret = SQLPrepare(
    stmt->m_hSTMT,
    (SQLTCHAR *) sql2, 
    sqlLen);
  
  if (SQL_SUCCEEDED(ret)) {
    info.GetReturnValue().Set(Nan::True());
  }
  else {
    Nan::ThrowError(ODBC::GetSQLError(
      SQL_HANDLE_STMT,
      stmt->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCStatement::PrepareSync"
    ));

    info.GetReturnValue().Set(Nan::False());
  }
}

/*
 * Prepare
 * 
 */

NAN_METHOD(ODBCStatement::Prepare) {
  DEBUG_PRINTF("ODBCStatement::Prepare\n");
  
  Nan::HandleScope scope;

  REQ_STRO_ARG(0, sql);
  REQ_FUN_ARG(1, cb);

  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  MEMCHECK( work_req );
  
  prepare_work_data* data = 
    (prepare_work_data *) calloc(1, sizeof(prepare_work_data));
  MEMCHECK( data );

  data->cb = new Nan::Callback(cb);

  data->sqlLen = sql->Length();

#ifdef UNICODE
  data->sql = (uint16_t *) malloc((data->sqlLen * sizeof(uint16_t)) + sizeof(uint16_t));
  MEMCHECK( data->sql );
  sql->Write((uint16_t *) data->sql);
#else
  data->sql = (char *) malloc(data->sqlLen +1);
  MEMCHECK( data->sql );
  sql->WriteUtf8((char *) data->sql);
#endif
  
  data->stmt = stmt;
  
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(), 
    work_req, 
    UV_Prepare, 
    (uv_after_work_cb)UV_AfterPrepare);

  stmt->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCStatement::UV_Prepare(uv_work_t* req) {
  DEBUG_PRINTF("ODBCStatement::UV_Prepare\n");
  
  prepare_work_data* data = (prepare_work_data *)(req->data);

  DEBUG_PRINTF("ODBCStatement::UV_Prepare m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    data->stmt->m_hENV,
    data->stmt->m_hDBC,
    data->stmt->m_hSTMT
  );
  
  SQLRETURN ret;
  
  ret = SQLPrepare(
    data->stmt->m_hSTMT,
    (SQLTCHAR *) data->sql, 
    data->sqlLen);

  data->result = ret;
}

void ODBCStatement::UV_AfterPrepare(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCStatement::UV_AfterPrepare\n");
  
  prepare_work_data* data = (prepare_work_data *)(req->data);
  
  DEBUG_PRINTF("ODBCStatement::UV_AfterPrepare m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    data->stmt->m_hENV,
    data->stmt->m_hDBC,
    data->stmt->m_hSTMT
  );
  
  Nan::HandleScope scope;

  //First thing, let's check if the execution of the query returned any errors 
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      SQL_HANDLE_STMT,
      data->stmt->m_hSTMT,
      data->cb);
  }
  else {
    Local<Value> info[2];

    info[0] = Nan::Null();
    info[1] = Nan::True();

    Nan::TryCatch try_catch;

    data->cb->Call( 2, info);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
  }
  
  data->stmt->Unref();
  delete data->cb;
  
  free(data->sql);
  free(data);
  free(req);
}

/*
 * BindSync
 * 
 */

NAN_METHOD(ODBCStatement::BindSync) 
{
  DEBUG_PRINTF("ODBCStatement::BindSync\n");
  Nan::HandleScope scope;

  if ( !info[0]->IsArray() ) {
    return Nan::ThrowTypeError("Argument 1 must be an Array");
  }
  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  
  DEBUG_PRINTF("ODBCStatement::BindSync m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    stmt->m_hENV,
    stmt->m_hDBC,
    stmt->m_hSTMT
  );
  
  //if we previously had parameters, then be sure to free them
  //before allocating more
  if (stmt->paramCount) {
      FREE_PARAMS( stmt->params, stmt->paramCount ) ;
  }
  
  stmt->params = ODBC::GetParametersFromArray(
    Local<Array>::Cast(info[0]), 
    &stmt->paramCount);
  
  SQLRETURN ret = SQL_SUCCESS;

  ret = ODBC::BindParameters( stmt->m_hSTMT, stmt->params, stmt->paramCount ) ;

  if (SQL_SUCCEEDED(ret)) {
    info.GetReturnValue().Set(Nan::True());
  }
  else {
    Nan::ThrowError(ODBC::GetSQLError(
      SQL_HANDLE_STMT,
      stmt->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCStatement::BindSync"
    ));
    
    info.GetReturnValue().Set(Nan::False());
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

/*
 * Bind
 * 
 */

NAN_METHOD(ODBCStatement::Bind) {
  DEBUG_PRINTF("ODBCStatement::Bind\n");
  
  Nan::HandleScope scope;

  if ( !info[0]->IsArray() ) {
    return Nan::ThrowError("Argument 1 must be an Array");
  }
  
  REQ_FUN_ARG(1, cb);

  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  MEMCHECK( work_req );
  
  bind_work_data* data = 
    (bind_work_data *) calloc(1, sizeof(bind_work_data));
  MEMCHECK( data );

  //if we previously had parameters, then be sure to free them
  //before allocating more
  if (stmt->paramCount) {
      FREE_PARAMS( stmt->params, stmt->paramCount ) ;
  }
  
  data->stmt = stmt;
  
  DEBUG_PRINTF("ODBCStatement::Bind m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    data->stmt->m_hENV,
    data->stmt->m_hDBC,
    data->stmt->m_hSTMT
  );
  
  data->cb = new Nan::Callback(cb);
  
  data->stmt->params = ODBC::GetParametersFromArray(
    Local<Array>::Cast(info[0]), 
    &data->stmt->paramCount);
  
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(), 
    work_req, 
    UV_Bind, 
    (uv_after_work_cb)UV_AfterBind);

  stmt->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCStatement::UV_Bind(uv_work_t* req) {
  DEBUG_PRINTF("ODBCStatement::UV_Bind\n");
  
  bind_work_data* data = (bind_work_data *)(req->data);

  DEBUG_PRINTF("ODBCStatement::UV_Bind m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    data->stmt->m_hENV,
    data->stmt->m_hDBC,
    data->stmt->m_hSTMT
  );
  
  data->result = ODBC::BindParameters( data->stmt->m_hSTMT, 
                 data->stmt->params, data->stmt->paramCount ) ;
}

void ODBCStatement::UV_AfterBind(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCStatement::UV_AfterBind\n");
  
  bind_work_data* data = (bind_work_data *)(req->data);
  
  Nan::HandleScope scope;
  
  //an easy reference to the statment object
  ODBCStatement* self = data->stmt->self();

  //Check if there were errors 
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      SQL_HANDLE_STMT,
      self->m_hSTMT,
      data->cb);
  }
  else {
    Local<Value> info[2];

    info[0] = Nan::Null();
    info[1] = Nan::True();

    Nan::TryCatch try_catch;

    data->cb->Call( 2, info);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
  }

  self->Unref();
  delete data->cb;
  
  free(data);
  free(req);
}

/*
 * CloseSync
 */

NAN_METHOD(ODBCStatement::CloseSync) {
  DEBUG_PRINTF("ODBCStatement::CloseSync\n");
  
  Nan::HandleScope scope;

  OPT_INT_ARG(0, closeOption, SQL_DESTROY);
  
  ODBCStatement* stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  
  DEBUG_PRINTF("ODBCStatement::CloseSync closeOption=%i\n", 
               closeOption);
  
  if (closeOption == SQL_DESTROY) {
    stmt->Free();
  }
  else {
    SQLFreeStmt(stmt->m_hSTMT, closeOption);
  }

  info.GetReturnValue().Set(Nan::True());
}

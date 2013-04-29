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

Persistent<FunctionTemplate> ODBCStatement::constructor_template;

void ODBCStatement::Init(v8::Handle<Object> target) {
  DEBUG_PRINTF("ODBCStatement::Init\n");
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  // Constructor Template
  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->SetClassName(String::NewSymbol("ODBCStatement"));

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = constructor_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);
  
  // Prototype Methods
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "execute", Execute);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "executeSync", ExecuteSync);
  
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "executeDirect", ExecuteDirect);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "executeDirectSync", ExecuteDirectSync);
  
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "prepare", Prepare);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "prepareSync", PrepareSync);
  
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "bind", Bind);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "bindSync", BindSync);
  
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "closeSync", CloseSync);

  // Attach the Database Constructor to the target object
  target->Set( v8::String::NewSymbol("ODBCStatement"),
               constructor_template->GetFunction());
  
  scope.Close(Undefined());
}

ODBCStatement::~ODBCStatement() {
  this->Free();
}

void ODBCStatement::Free() {
  if (m_hSTMT) {
    uv_mutex_lock(&ODBC::g_odbcMutex);
    
    SQLFreeHandle(SQL_HANDLE_STMT, m_hSTMT);
    m_hSTMT = NULL;
    
    uv_mutex_unlock(&ODBC::g_odbcMutex);
    
    if (bufferLength > 0) {
      free(buffer);
    }
  }
}

Handle<Value> ODBCStatement::New(const Arguments& args) {
  HandleScope scope;
  
  REQ_EXT_ARG(0, js_henv);
  REQ_EXT_ARG(1, js_hdbc);
  REQ_EXT_ARG(2, js_hstmt);
  
  HENV hENV = static_cast<HENV>(js_henv->Value());
  HDBC hDBC = static_cast<HDBC>(js_hdbc->Value());
  HSTMT hSTMT = static_cast<HSTMT>(js_hstmt->Value());
  
  //create a new OBCResult object
  ODBCStatement* stmt = new ODBCStatement(hENV, hDBC, hSTMT);
  
  //specify the buffer length
  stmt->bufferLength = MAX_VALUE_SIZE - 1;
  
  //initialze a buffer for this object
  stmt->buffer = (uint16_t *) malloc(stmt->bufferLength + 1);
  //TODO: make sure the malloc succeeded

  //set the initial colCount to 0
  stmt->colCount = 0;
  
  stmt->Wrap(args.Holder());
  
  return scope.Close(args.Holder());
}

/*
 * Execute
 */

Handle<Value> ODBCStatement::Execute(const Arguments& args) {
  DEBUG_PRINTF("ODBCStatement::Execute\n");
  
  HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCStatement* stmt = ObjectWrap::Unwrap<ODBCStatement>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  execute_work_data* data = 
    (execute_work_data *) calloc(1, sizeof(execute_work_data));

  data->cb = Persistent<Function>::New(cb);
  
  data->stmt = stmt;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(),
    work_req,
    UV_Execute,
    (uv_after_work_cb)UV_AfterExecute);

  stmt->Ref();

  return  scope.Close(Undefined());
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
  
  HandleScope scope;
  
  //an easy reference to the statment object
  ODBCStatement* self = data->stmt->self();

  //First thing, let's check if the execution of the query returned any errors 
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      self->m_hENV,
      self->m_hDBC,
      self->m_hSTMT,
      data->cb);
  }
  else {
    Local<Value> args[3];
    bool canFreeHandle = false;
    
    args[0] = External::New(self->m_hENV);
    args[1] = External::New(self->m_hDBC);
    args[2] = External::New(self->m_hSTMT);
    args[3] = External::New(&canFreeHandle);
    
    Persistent<Object> js_result(ODBCResult::constructor_template->
                              GetFunction()->NewInstance(4, args));

    args[0] = Local<Value>::New(Null());
    args[1] = Local<Object>::New(js_result);
    
    data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  }
  
  TryCatch try_catch;
  
  self->Unref();
  
  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
  
  data->cb.Dispose();
  
  if (data->stmt->paramCount) {
    Parameter prm;
    
    //free parameter memory
    for (int i = 0; i < data->stmt->paramCount; i++) {
      if (prm = data->stmt->params[i], prm.buffer != NULL) {
        switch (prm.c_type) {
          case SQL_C_CHAR:    free(prm.buffer);             break; 
          case SQL_C_SBIGINT: delete (int64_t *)prm.buffer; break;
          case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break;
          case SQL_C_BIT:     delete (bool    *)prm.buffer; break;
        }
      }
    }

    data->stmt->paramCount = 0;

    free(data->stmt->params);
  }
  
  free(data);
  free(req);
  
  scope.Close(Undefined());
}

/*
 * ExecuteSync
 * 
 */

Handle<Value> ODBCStatement::ExecuteSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCStatement::ExecuteSync\n");
  
  HandleScope scope;

  ODBCStatement* stmt = ObjectWrap::Unwrap<ODBCStatement>(args.Holder());

  SQLRETURN ret = SQLExecute(stmt->m_hSTMT); 

  if (stmt->paramCount) {
    Parameter prm;
    
    //free parameter memory
    for (int i = 0; i < stmt->paramCount; i++) {
      if (prm = stmt->params[i], prm.buffer != NULL) {
        switch (prm.c_type) {
          case SQL_C_CHAR:    free(prm.buffer);             break; 
          case SQL_C_SBIGINT: delete (int64_t *)prm.buffer; break;
          case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break;
          case SQL_C_BIT:     delete (bool    *)prm.buffer; break;
        }
      }
    }

    stmt->paramCount = 0;

    free(stmt->params);
  }
  
  if(ret == SQL_ERROR) {
    ThrowException(ODBC::GetSQLError(
      stmt->m_hENV,
      stmt->m_hDBC,
      stmt->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCStatement::ExecuteSync"
    ));
    
    return scope.Close(Null());
  }
  else {
    Local<Value> args[3];
    bool canFreeHandle = false;
    
    args[0] = External::New(stmt->m_hENV);
    args[1] = External::New(stmt->m_hDBC);
    args[2] = External::New(stmt->m_hSTMT);
    args[3] = External::New(&canFreeHandle);
    
    Local<Object> js_result(ODBCResult::constructor_template->
                              GetFunction()->NewInstance(4, args));
    
    return scope.Close(js_result);
  }
}

/*
 * ExecuteDirect
 * 
 */

Handle<Value> ODBCStatement::ExecuteDirect(const Arguments& args) {
  DEBUG_PRINTF("ODBCStatement::ExecuteDirect\n");
  
  HandleScope scope;

  REQ_STR_ARG(0, sql);
  REQ_FUN_ARG(1, cb);

  ODBCStatement* stmt = ObjectWrap::Unwrap<ODBCStatement>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  execute_direct_work_data* data = 
    (execute_direct_work_data *) calloc(1, sizeof(execute_direct_work_data));

  data->sql = (char *) malloc(sql.length() +1);
  data->cb = Persistent<Function>::New(cb);
  
  strcpy(data->sql, *sql);
  
  data->stmt = stmt;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(),
    work_req, 
    UV_ExecuteDirect, 
    (uv_after_work_cb)UV_AfterExecuteDirect);

  stmt->Ref();

  return  scope.Close(Undefined());
}

void ODBCStatement::UV_ExecuteDirect(uv_work_t* req) {
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteDirect\n");
  
  execute_direct_work_data* data = (execute_direct_work_data *)(req->data);

  SQLRETURN ret;
  
  ret = SQLExecDirect(
    data->stmt->m_hSTMT,
    (SQLCHAR *) data->sql, 
    strlen(data->sql));  

  data->result = ret;
}

void ODBCStatement::UV_AfterExecuteDirect(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteDirect\n");
  
  execute_direct_work_data* data = (execute_direct_work_data *)(req->data);
  
  HandleScope scope;
  
  //an easy reference to the statment object
  ODBCStatement* self = data->stmt->self();

  //First thing, let's check if the execution of the query returned any errors 
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      self->m_hENV,
      self->m_hDBC,
      self->m_hSTMT,
      data->cb);
  }
  else {
    Local<Value> args[3];
    bool canFreeHandle = false;
    
    args[0] = External::New(self->m_hENV);
    args[1] = External::New(self->m_hDBC);
    args[2] = External::New(self->m_hSTMT);
    args[3] = External::New(&canFreeHandle);
    
    Persistent<Object> js_result(ODBCResult::constructor_template->
                              GetFunction()->NewInstance(4, args));

    args[0] = Local<Value>::New(Null());
    args[1] = Local<Object>::New(js_result);
    
    data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  }
  
  TryCatch try_catch;
  
  self->Unref();
  
  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
  
  data->cb.Dispose();
  
  free(data->sql);
  free(data);
  free(req);
  
  scope.Close(Undefined());
}

/*
 * ExecuteDirectSync
 * 
 */

Handle<Value> ODBCStatement::ExecuteDirectSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCStatement::ExecuteDirectSync\n");
  
  HandleScope scope;

  REQ_STR_ARG(0, sql);

  ODBCStatement* stmt = ObjectWrap::Unwrap<ODBCStatement>(args.Holder());
  
  SQLRETURN ret = SQLExecDirect(
    stmt->m_hSTMT,
    (SQLCHAR *) *sql, 
    sql.length());  

  if(ret == SQL_ERROR) {
    ThrowException(ODBC::GetSQLError(
      stmt->m_hENV,
      stmt->m_hDBC,
      stmt->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCStatement::ExecuteDirectSync"
    ));
    
    return scope.Close(Null());
  }
  else {
    Local<Value> args[3];
    bool canFreeHandle = false;
    
    args[0] = External::New(stmt->m_hENV);
    args[1] = External::New(stmt->m_hDBC);
    args[2] = External::New(stmt->m_hSTMT);
    args[3] = External::New(&canFreeHandle);
    
    Persistent<Object> js_result(ODBCResult::constructor_template->
                              GetFunction()->NewInstance(4, args));
    
    return scope.Close(js_result);
  }
}

/*
 * PrepareSync
 * 
 */

Handle<Value> ODBCStatement::PrepareSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCStatement::PrepareSync\n");
  
  HandleScope scope;

  REQ_STR_ARG(0, sql);

  ODBCStatement* stmt = ObjectWrap::Unwrap<ODBCStatement>(args.Holder());

  SQLRETURN ret;
  char *sql2;
  
  sql2 = (char *) malloc(sql.length() +1);
  strcpy(sql2, *sql);
  
  ret = SQLPrepare(
    stmt->m_hSTMT,
    (SQLCHAR *) sql2, 
    strlen(sql2));
  
  if (SQL_SUCCEEDED(ret)) {
    return  scope.Close(True());
  }
  else {
    //TODO: throw an error object
    return  scope.Close(False());
  }
}

/*
 * Prepare
 * 
 */

Handle<Value> ODBCStatement::Prepare(const Arguments& args) {
  DEBUG_PRINTF("ODBCStatement::Prepare\n");
  
  HandleScope scope;

  REQ_STR_ARG(0, sql);
  REQ_FUN_ARG(1, cb);

  ODBCStatement* stmt = ObjectWrap::Unwrap<ODBCStatement>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  prepare_work_data* data = 
    (prepare_work_data *) calloc(1, sizeof(prepare_work_data));

  data->sql = (char *) malloc(sql.length() +1);
  data->cb = Persistent<Function>::New(cb);
  
  strcpy(data->sql, *sql);
  
  data->stmt = stmt;
  
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(), 
    work_req, 
    UV_Prepare, 
    (uv_after_work_cb)UV_AfterPrepare);

  stmt->Ref();

  return  scope.Close(Undefined());
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
    (SQLCHAR *) data->sql, 
    strlen(data->sql));

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
  
  HandleScope scope;

  //First thing, let's check if the execution of the query returned any errors 
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      data->stmt->m_hENV,
      data->stmt->m_hDBC,
      data->stmt->m_hSTMT,
      data->cb);
  }
  else {
    Local<Value> args[2];

    args[0] = Local<Value>::New(Null());
    args[1] = Local<Value>::New(True());
    
    data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  }
  
  TryCatch try_catch;
  
  data->stmt->Unref();
  
  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
  
  data->cb.Dispose();
  
  free(data->sql);
  free(data);
  free(req);
  
  scope.Close(Undefined());
}

/*
 * BindSync
 * 
 */

Handle<Value> ODBCStatement::BindSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCStatement::BindSync\n");
  
  HandleScope scope;

  if ( !args[0]->IsArray() ) {
    return ThrowException(Exception::TypeError(
              String::New("Argument 1 must be an Array"))
    );
  }

  ODBCStatement* stmt = ObjectWrap::Unwrap<ODBCStatement>(args.Holder());
  
  DEBUG_PRINTF("ODBCStatement::BindSync m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    stmt->m_hENV,
    stmt->m_hDBC,
    stmt->m_hSTMT
  );
  
  stmt->params = ODBC::GetParametersFromArray(
    Local<Array>::Cast(args[0]), 
    &stmt->paramCount);
  
  SQLRETURN ret = SQL_SUCCESS;
  Parameter prm;
  
  for (int i = 0; i < stmt->paramCount; i++) {
    prm = stmt->params[i];
    
    DEBUG_PRINTF(
      "ODBCStatement::BindSync - param[%i]: c_type=%i type=%i "
      "buffer_length=%i size=%i length=%i &length=%X decimals=%i value=%s\n",
      i, prm.c_type, prm.type, prm.buffer_length, prm.size, prm.length, 
      &stmt->params[i].length, prm.decimals, prm.buffer
    );

    ret = SQLBindParameter(
      stmt->m_hSTMT,        //StatementHandle
      i + 1,                      //ParameterNumber
      SQL_PARAM_INPUT,            //InputOutputType
      prm.c_type,                 //ValueType
      prm.type,                   //ParameterType
      prm.size,                   //ColumnSize
      prm.decimals,               //DecimalDigits
      prm.buffer,                 //ParameterValuePtr
      prm.buffer_length,          //BufferLength
      //using &prm.length did not work here...
      &stmt->params[i].length);   //StrLen_or_IndPtr

    if (ret == SQL_ERROR) {
      break;
    }
  }

  if (SQL_SUCCEEDED(ret)) {
    return  scope.Close(True());
  }
  else {
    ThrowException(ODBC::GetSQLError(
      stmt->m_hENV,
      stmt->m_hDBC,
      stmt->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCStatement::BindSync"
    ));
    
    return  scope.Close(False());
  }

  return  scope.Close(Undefined());
}

/*
 * Bind
 * 
 */

Handle<Value> ODBCStatement::Bind(const Arguments& args) {
  DEBUG_PRINTF("ODBCStatement::Bind\n");
  
  HandleScope scope;

  if ( !args[0]->IsArray() ) {
    return ThrowException(Exception::TypeError(
              String::New("Argument 1 must be an Array"))
    );
  }
  
  REQ_FUN_ARG(1, cb);

  ODBCStatement* stmt = ObjectWrap::Unwrap<ODBCStatement>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  bind_work_data* data = 
    (bind_work_data *) calloc(1, sizeof(bind_work_data));

  data->stmt = stmt;
  
  DEBUG_PRINTF("ODBCStatement::Bind m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    data->stmt->m_hENV,
    data->stmt->m_hDBC,
    data->stmt->m_hSTMT
  );
  
  data->cb = Persistent<Function>::New(cb);
  
  data->stmt->params = ODBC::GetParametersFromArray(
    Local<Array>::Cast(args[0]), 
    &data->stmt->paramCount);
  
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(), 
    work_req, 
    UV_Bind, 
    (uv_after_work_cb)UV_AfterBind);

  stmt->Ref();

  return  scope.Close(Undefined());
}

void ODBCStatement::UV_Bind(uv_work_t* req) {
  DEBUG_PRINTF("ODBCStatement::UV_Bind\n");
  
  bind_work_data* data = (bind_work_data *)(req->data);

  DEBUG_PRINTF("ODBCStatement::UV_Bind m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    data->stmt->m_hENV,
    data->stmt->m_hDBC,
    data->stmt->m_hSTMT
  );
  
  SQLRETURN ret = SQL_SUCCESS;
  Parameter prm;
  
  for (int i = 0; i < data->stmt->paramCount; i++) {
    prm = data->stmt->params[i];
    
    DEBUG_PRINTF(
      "ODBCStatement::UV_Bind - param[%i]: c_type=%i type=%i "
      "buffer_length=%i size=%i length=%i &length=%X decimals=%i value=%s\n",
      i, prm.c_type, prm.type, prm.buffer_length, prm.size, prm.length, 
      &data->stmt->params[i].length, prm.decimals, prm.buffer
    );

    ret = SQLBindParameter(
      data->stmt->m_hSTMT,        //StatementHandle
      i + 1,                      //ParameterNumber
      SQL_PARAM_INPUT,            //InputOutputType
      prm.c_type,                 //ValueType
      prm.type,                   //ParameterType
      prm.size,                   //ColumnSize
      prm.decimals,               //DecimalDigits
      prm.buffer,                 //ParameterValuePtr
      prm.buffer_length,          //BufferLength
      //using &prm.length did not work here...
      &data->stmt->params[i].length);   //StrLen_or_IndPtr

    if (ret == SQL_ERROR) {
      break;
    }
  }

  data->result = ret;
}

void ODBCStatement::UV_AfterBind(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCStatement::UV_AfterBind\n");
  
  bind_work_data* data = (bind_work_data *)(req->data);
  
  HandleScope scope;
  
  //an easy reference to the statment object
  ODBCStatement* self = data->stmt->self();

  //Check if there were errors 
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      self->m_hENV,
      self->m_hDBC,
      self->m_hSTMT,
      data->cb);
  }
  else {
    Local<Value> args[2];

    args[0] = Local<Value>::New(Null());
    args[1] = Local<Value>::New(True());
    
    data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  }
  
  TryCatch try_catch;
  
  self->Unref();
  
  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
  
  data->cb.Dispose();
  
  free(data);
  free(req);
  
  scope.Close(Undefined());
}

/*
 * CloseSync
 */

Handle<Value> ODBCStatement::CloseSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCStatement::CloseSync\n");
  
  HandleScope scope;

  ODBCStatement* stmt = ObjectWrap::Unwrap<ODBCStatement>(args.Holder());
  
  stmt->Free();

  return  scope.Close(True());
}

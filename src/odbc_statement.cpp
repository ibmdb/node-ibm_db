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
  DEBUG_PRINTF("ODBCStatement::Free\n");
  //if we previously had parameters, then be sure to free them
  if (paramCount) {
    int count = paramCount;
    paramCount = 0;

    Parameter prm;

    //free parameter memory
    for (int i = 0; i < count; i++) {
      if (prm = params[i], prm.buffer != NULL) {
        switch (prm.c_type) {
          case SQL_C_WCHAR:   free(prm.buffer);             break;
          case SQL_C_CHAR:    free(prm.buffer);             break;
          case SQL_C_SBIGINT: delete (int64_t *)prm.buffer; break;
          case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break;
          case SQL_C_BIT:     delete (bool    *)prm.buffer; break;
        }
      }
    }

    free(params);
  }

  if (m_hSTMT) {
    uv_mutex_lock(&ODBC::g_odbcMutex);

    SQLFreeHandle(SQL_HANDLE_STMT, m_hSTMT);
    m_hSTMT = (SQLHSTMT) NULL;

    uv_mutex_unlock(&ODBC::g_odbcMutex);

    if (bufferLength > 0) {
      free(buffer);
    }
  }
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
  stmt->bufferLength = MAX_VALUE_SIZE - 1;

  //initialze a buffer for this object
  stmt->buffer = (uint16_t *) malloc(stmt->bufferLength + 1);
  //TODO: make sure the malloc succeeded

  //set the initial colCount to 0
  stmt->colCount = 0;

  //initialize the paramCount
  stmt->paramCount = 0;

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

  execute_work_data* data =
    (execute_work_data *) calloc(1, sizeof(execute_work_data));

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

    // TODO is this object being cleared anywhere? Memory leak?
    Nan::Persistent<Object> js_result;
    js_result.Reset(Nan::New(ODBCResult::constructor)->NewInstance(4, info));

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
    Local<Value> result[4];
    bool* canFreeHandle = new bool(false);

    result[0] = Nan::New<External>((void*) (intptr_t) stmt->m_hENV);
    result[1] = Nan::New<External>((void*) (intptr_t) stmt->m_hDBC);
    result[2] = Nan::New<External>((void*) (intptr_t) stmt->m_hSTMT);
    result[3] = Nan::New<External>((void*)canFreeHandle);

    Local<Object> js_result = Nan::New(ODBCResult::constructor)->NewInstance(4, result);

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

  execute_work_data* data =
    (execute_work_data *) calloc(1, sizeof(execute_work_data));

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

    uv_mutex_lock(&ODBC::g_odbcMutex);
    SQLFreeStmt(self->m_hSTMT, SQL_CLOSE);
    uv_mutex_unlock(&ODBC::g_odbcMutex);

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

    uv_mutex_lock(&ODBC::g_odbcMutex);
    SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
    uv_mutex_unlock(&ODBC::g_odbcMutex);

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

  execute_direct_work_data* data =
    (execute_direct_work_data *) calloc(1, sizeof(execute_direct_work_data));

  data->cb = new Nan::Callback(cb);

  data->sqlLen = sql->Length();

#ifdef UNICODE
  data->sql = (uint16_t *) malloc((data->sqlLen * sizeof(uint16_t)) + sizeof(uint16_t));
  sql->Write((uint16_t *) data->sql);
#else
  data->sql = (char *) malloc(data->sqlLen +1);
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
  sql->Write(sql2);
#else
  char *sql2;
  sql2 = (char *) malloc(sqlLen);
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

  prepare_work_data* data =
    (prepare_work_data *) calloc(1, sizeof(prepare_work_data));

  data->cb = new Nan::Callback(cb);

  data->sqlLen = sql->Length();

#ifdef UNICODE
  data->sql = (uint16_t *) malloc((data->sqlLen * sizeof(uint16_t)) + sizeof(uint16_t));
  sql->Write((uint16_t *) data->sql);
#else
  data->sql = (char *) malloc(data->sqlLen +1);
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

NAN_METHOD(ODBCStatement::BindSync) {
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
    int count = stmt->paramCount;
    stmt->paramCount = 0;

    Parameter prm;

    //free parameter memory
    for (int i = 0; i < count; i++) {
      if (prm = stmt->params[i], prm.buffer != NULL) {
        switch (prm.c_type) {
          case SQL_C_WCHAR:   free(prm.buffer);             break;
          case SQL_C_CHAR:    free(prm.buffer);             break;
          case SQL_C_SBIGINT: delete (int64_t *)prm.buffer; break;
          case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break;
          case SQL_C_BIT:     delete (bool    *)prm.buffer; break;
        }
      }
    }

    free(stmt->params);
  }

  stmt->params = ODBC::GetParametersFromArray(
    Local<Array>::Cast(info[0]),
    &stmt->paramCount);

  SQLRETURN ret = SQL_SUCCESS;
  Parameter prm;

  for (int i = 0; i < stmt->paramCount; i++) {
    prm = stmt->params[i];

    DEBUG_PRINTF(
      "ODBCStatement::BindSync - param[%i]: c_type=%i type=%i "
      "buffer_length=%i size=%i length=%i &length=%X decimals=%i value=%s\n",
      i, prm.c_type, prm.type, prm.buffer_length, prm.size, prm.length,
      &stmt->params[i].length, prm.decimals,
      ((prm.length <= 0)? "" : prm.buffer)
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

  // info.GetReturnValue().Set(Nan::Undefined());
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

  bind_work_data* data =
    (bind_work_data *) calloc(1, sizeof(bind_work_data));

  //if we previously had parameters, then be sure to free them
  //before allocating more
  if (stmt->paramCount) {
    int count = stmt->paramCount;
    stmt->paramCount = 0;

    Parameter prm;

    //free parameter memory
    for (int i = 0; i < count; i++) {
      if (prm = stmt->params[i], prm.buffer != NULL) {
        switch (prm.c_type) {
          case SQL_C_WCHAR:   free(prm.buffer);             break;
          case SQL_C_CHAR:    free(prm.buffer);             break;
          case SQL_C_SBIGINT: delete (int64_t *)prm.buffer; break;
          case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break;
          case SQL_C_BIT:     delete (bool    *)prm.buffer; break;
        }
      }
    }

    free(stmt->params);
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
    uv_mutex_lock(&ODBC::g_odbcMutex);

    SQLFreeStmt(stmt->m_hSTMT, closeOption);

    uv_mutex_unlock(&ODBC::g_odbcMutex);
  }

  info.GetReturnValue().Set(Nan::True());
}

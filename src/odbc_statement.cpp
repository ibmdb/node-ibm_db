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

NAN_MODULE_INIT(ODBCStatement::Init)
{
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

  Nan::SetPrototypeMethod(t, "setAttr", SetAttr);
  Nan::SetPrototypeMethod(t, "setAttrSync", SetAttrSync);

  Nan::SetPrototypeMethod(t, "close", Close);
  Nan::SetPrototypeMethod(t, "closeSync", CloseSync);

  // Attach the Database Constructor to the target object
  constructor.Reset(Nan::GetFunction(t).ToLocalChecked());
  Nan::Set(target, Nan::New("ODBCStatement").ToLocalChecked(),
           Nan::GetFunction(t).ToLocalChecked());
}

ODBCStatement::~ODBCStatement()
{
  DEBUG_PRINTF("ODBCStatement::~ODBCStatement m_hSTMT =%X\n", m_hSTMT);
  this->Free();
}

void ODBCStatement::Free()
{
  DEBUG_PRINTF("ODBCStatement::Free - Entry: paramCount = %i, m_hSTMT =%X\n",
               paramCount, m_hSTMT);
  // if we previously had parameters, then be sure to free them
  if (paramCount)
  {
    FREE_PARAMS(params, paramCount);
    DEBUG_PRINTF("ODBCStatement::Free - Params Freed.\n");
  }

  if (m_hSTMT)
  {
    SQLFreeHandle(SQL_HANDLE_STMT, m_hSTMT);
    DEBUG_PRINTF("ODBCStatement::Free SQLFreeHandle called for m_hSTMT =%X\n", m_hSTMT);
    m_hSTMT = (SQLHSTMT)NULL;
  }

  if (buffer != NULL)
  {
    free((uint16_t *)buffer);
    buffer = NULL;
  }
  if (bufferLength > 0)
  {
    bufferLength = 0;
  }
  DEBUG_PRINTF("ODBCStatement::Free - Exit\n");
}

NAN_METHOD(ODBCStatement::New)
{
  DEBUG_PRINTF("ODBCStatement::New - Entry\n");
  Nan::HandleScope scope;

  REQ_EXT_ARG(0, js_henv);
  REQ_EXT_ARG(1, js_hdbc);
  REQ_EXT_ARG(2, js_hstmt);

  SQLHENV hENV = (SQLHENV)((intptr_t)js_henv->Value());
  SQLHDBC hDBC = (SQLHDBC)((intptr_t)js_hdbc->Value());
  SQLHSTMT hSTMT = (SQLHSTMT)((intptr_t)js_hstmt->Value());

  // create a new OBCResult object
  ODBCStatement *stmt = new ODBCStatement(hENV, hDBC, hSTMT);

  // specify the buffer length
  stmt->bufferLength = MAX_VALUE_SIZE;
  stmt->buffer = NULL; // Will get allocated in ODBC::GetColumnValue if required

  // set the initial colCount to 0
  stmt->colCount = 0;

  // initialize the paramCount
  stmt->paramCount = 0;
  stmt->params = 0;

  stmt->Wrap(info.Holder());

  info.GetReturnValue().Set(info.Holder());
  DEBUG_PRINTF("ODBCStatement::New - Exit\n");
}

/*
 * Execute
 */

NAN_METHOD(ODBCStatement::Execute)
{
  DEBUG_PRINTF("ODBCStatement::Execute - Entry\n");

  Nan::HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  execute_work_data *data =
      (execute_work_data *)calloc(1, sizeof(execute_work_data));
  if (!data)
    free(work_req);
  MEMCHECK(data);

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
  DEBUG_PRINTF("ODBCStatement::Execute - Exit\n");
}

void ODBCStatement::UV_Execute(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_Execute - Entry\n");

  execute_work_data *data = (execute_work_data *)(req->data);

  SQLRETURN ret;

  ret = SQLExecute(data->stmt->m_hSTMT);

  data->result = ret;
  DEBUG_PRINTF("ODBCStatement::UV_Execute - Exit\n");
}

void ODBCStatement::UV_AfterExecute(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecute - Entry\n");

  execute_work_data *data = (execute_work_data *)(req->data);
  Nan::HandleScope scope;
  int outParamCount = 0; // Non-zero tells its a SP with OUT param
  Local<Array> sp_result = Nan::New<Array>();

  // an easy reference to the statment object
  ODBCStatement *stmt = data->stmt->self();

  if (SQL_SUCCEEDED(data->result))
  {
    for (int i = 0; i < stmt->paramCount; i++)
    { // For stored Procedure CALL
      if (stmt->params[i].paramtype % 2 == 0)
      {
        Nan::Set(sp_result, Nan::New(outParamCount), ODBC::GetOutputParameter(&stmt->params[i]));
        outParamCount++;
      }
    }
  }
  if (stmt->paramCount)
  {
    FREE_PARAMS(stmt->params, stmt->paramCount);
  }

  // First thing, let's check if the execution of the query returned any errors
  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(
        SQL_HANDLE_STMT,
        stmt->m_hSTMT,
        data->cb);
  }
  else
  {
    Local<Value> info[4];
    bool *canFreeHandle = new bool(false);

    info[0] = Nan::New<External>((void *)(intptr_t)stmt->m_hENV);
    info[1] = Nan::New<External>((void *)(intptr_t)stmt->m_hDBC);
    info[2] = Nan::New<External>((void *)(intptr_t)stmt->m_hSTMT);
    info[3] = Nan::New<External>((void *)canFreeHandle);

    Local<Object> js_result = Nan::NewInstance(Nan::New(ODBCResult::constructor), 4, info).ToLocalChecked();

    info[0] = Nan::Null();
    info[1] = js_result;

    if (outParamCount)
    {
      info[2] = sp_result; // Must a CALL stmt
    }
    else
      info[2] = Nan::Null();

    Nan::TryCatch try_catch;

    data->cb->Call(3, info);

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  stmt->Unref();
  delete data->cb;

  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecute - Exit\n");
}

/*
 * ExecuteSync
 *
 */

NAN_METHOD(ODBCStatement::ExecuteSync)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteSync - Entry\n");

  Nan::HandleScope scope;

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  int outParamCount = 0;
  Local<Array> sp_result = Nan::New<Array>();

  SQLRETURN ret = SQLExecute(stmt->m_hSTMT);

  if (SQL_SUCCEEDED(ret))
  {
    for (int i = 0; i < stmt->paramCount; i++)
    { // For stored Procedure CALL
      if (stmt->params[i].paramtype % 2 == 0)
      {
        Nan::Set(sp_result, Nan::New(outParamCount), ODBC::GetOutputParameter(&stmt->params[i]));
        outParamCount++;
      }
    }
  }
  if (stmt->paramCount)
  {
    FREE_PARAMS(stmt->params, stmt->paramCount);
  }

  if (ret == SQL_ERROR)
  {
    Nan::ThrowError(ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        stmt->m_hSTMT,
        (char *)"[node-odbc] Error in ODBCStatement::ExecuteSync"));

    info.GetReturnValue().Set(Nan::Null());
  }
  else
  {
    Local<Value> result[4];
    bool *canFreeHandle = new bool(false);

    result[0] = Nan::New<External>((void *)(intptr_t)stmt->m_hENV);
    result[1] = Nan::New<External>((void *)(intptr_t)stmt->m_hDBC);
    result[2] = Nan::New<External>((void *)(intptr_t)stmt->m_hSTMT);
    result[3] = Nan::New<External>((void *)canFreeHandle);

    Local<Object> js_result = Nan::NewInstance(Nan::New(ODBCResult::constructor), 4, result).ToLocalChecked();

    if (outParamCount) // Its a CALL stmt with OUT params.
    {                  // Return an array with outparams as second element. [result, outparams]
      Local<Array> resultset = Nan::New<Array>();
      Nan::Set(resultset, 0, js_result);
      Nan::Set(resultset, 1, sp_result);
      info.GetReturnValue().Set(resultset);
    }
    else
      info.GetReturnValue().Set(js_result);
  }
  DEBUG_PRINTF("ODBCStatement::ExecuteSync - Exit\n");
}

/*
 * ExecuteNonQuery
 */

NAN_METHOD(ODBCStatement::ExecuteNonQuery)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuery - Entry\n");

  Nan::HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  execute_work_data *data =
      (execute_work_data *)calloc(1, sizeof(execute_work_data));
  if (!data)
    free(work_req);
  MEMCHECK(data);

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
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuery - Exit\n");
}

void ODBCStatement::UV_ExecuteNonQuery(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteNonQuery - Entry\n");

  execute_work_data *data = (execute_work_data *)(req->data);

  SQLRETURN ret;

  ret = SQLExecute(data->stmt->m_hSTMT);

  data->result = ret;
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteNonQuery - Exit\n");
}

void ODBCStatement::UV_AfterExecuteNonQuery(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteNonQuery - Entry\n");

  execute_work_data *data = (execute_work_data *)(req->data);

  Nan::HandleScope scope;
  SQLLEN rowCount = 0;
  SQLRETURN ret = data->result;
  Local<Value> info[2];
  int warning = 0;

  // an easy reference to the statment object
  ODBCStatement *self = data->stmt->self();

  // Store warning message of SQLExecute now only.
  if (ret > SQL_SUCCESS && ret != SQL_NO_DATA_FOUND)
  {
    info[0] = ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        self->m_hSTMT,
        (char *)"[node-ibm_db] Warning in ODBCStatement::UV_AfterExecuteNonQuery");
    warning = 1;
  }
  if ((ret == SQL_SUCCESS) || (ret == SQL_SUCCESS_WITH_INFO))
  {
    ret = SQLRowCount(self->m_hSTMT, &rowCount);
  }

  if (ret < SQL_SUCCESS)
  {
    ODBC::CallbackSQLError(
        SQL_HANDLE_STMT,
        self->m_hSTMT,
        data->cb);
  }
  else
  {
    if ((ret > SQL_SUCCESS) && (warning == 0) && (ret != SQL_NO_DATA_FOUND))
    {
      info[0] = ODBC::GetSQLError(
          SQL_HANDLE_STMT,
          self->m_hSTMT,
          (char *)"[node-ibm_db] Warning in ODBCStatement::UV_AfterExecuteNonQuery for SQLRowCount.");
    }
    else if (!warning)
    {
      info[0] = Nan::Null();
    }
    info[1] = Nan::New<Number>(rowCount);

    Nan::TryCatch try_catch;

    data->cb->Call(Nan::GetCurrentContext()->Global(), 2, info);

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  SQLFreeStmt(self->m_hSTMT, SQL_CLOSE);
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteNonQuery SQLFreeStmt called for m_hSTMT = %X\n", self->m_hSTMT);

  self->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteNonQuery - Exit\n");
}

/*
 * ExecuteNonQuerySync
 *
 */

NAN_METHOD(ODBCStatement::ExecuteNonQuerySync)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuerySync - Entry\n");

  Nan::HandleScope scope;
  SQLLEN rowCount = 0;
  SQLRETURN ret = SQL_SUCCESS;

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  ret = SQLExecute(stmt->m_hSTMT);

  if ((ret == SQL_SUCCESS) || (ret == SQL_SUCCESS_WITH_INFO))
  {
    ret = SQLRowCount(stmt->m_hSTMT, &rowCount);
  }

  if (ret < SQL_SUCCESS)
  {
    Nan::ThrowError(ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        stmt->m_hSTMT,
        (char *)"[node-ibm_db] Error in ODBCStatement::ExecuteNonQuerySync"));

    SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
    info.GetReturnValue().Set(Nan::Null());
  }
  else
  {
    SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
    info.GetReturnValue().Set(Nan::New<Number>(rowCount));
  }
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuerySync - Exit\n");
}

/*
 * ExecuteDirect
 *
 */

NAN_METHOD(ODBCStatement::ExecuteDirect)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteDirect - Entry\n");

  Nan::HandleScope scope;

  REQ_STRO_ARG(0, sql);
  REQ_FUN_ARG(1, cb);
  int len = sql.length();
  void *cppSQL = NULL;
  GETCPPSTR(cppSQL, sql, len);

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  if (!work_req)
    free(cppSQL);
  MEMCHECK(work_req);

  execute_direct_work_data *data =
      (execute_direct_work_data *)calloc(1, sizeof(execute_direct_work_data));
  if (!data)
  {
    free(cppSQL);
    free(work_req);
  }
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->sql = cppSQL;
  data->sqlLen = len;
  data->stmt = stmt;
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_ExecuteDirect,
      (uv_after_work_cb)UV_AfterExecuteDirect);

  stmt->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBCStatement::ExecuteDirect - Exit\n");
}

void ODBCStatement::UV_ExecuteDirect(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteDirect - Entry\n");

  execute_direct_work_data *data = (execute_direct_work_data *)(req->data);

  SQLRETURN ret;

  ret = SQLExecDirect(
      data->stmt->m_hSTMT,
      (SQLTCHAR *)data->sql,
      data->sqlLen);

  data->result = ret;
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteDirect - Exit\n");
}

void ODBCStatement::UV_AfterExecuteDirect(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteDirect - Entry\n");

  execute_direct_work_data *data = (execute_direct_work_data *)(req->data);

  Nan::HandleScope scope;

  // an easy reference to the statment object
  ODBCStatement *self = data->stmt->self();

  // First thing, let's check if the execution of the query returned any errors
  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(
        SQL_HANDLE_STMT,
        self->m_hSTMT,
        data->cb);
  }
  else
  {
    Local<Value> info[4];
    bool *canFreeHandle = new bool(false);

    info[0] = Nan::New<External>((void *)(intptr_t)self->m_hENV);
    info[1] = Nan::New<External>((void *)(intptr_t)self->m_hDBC);
    info[2] = Nan::New<External>((void *)(intptr_t)self->m_hSTMT);
    info[3] = Nan::New<External>((void *)canFreeHandle);

    // TODO persistent leak?
    Nan::Persistent<Object> js_result;
    js_result.Reset(Nan::NewInstance(Nan::New(ODBCResult::constructor), 4, info).ToLocalChecked());

    info[0] = Nan::Null();
    info[1] = Nan::New(js_result);

    Nan::TryCatch try_catch;

    data->cb->Call(2, info);

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  self->Unref();
  delete data->cb;

  free(data->sql);
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteDirect - Exit\n");
}

/*
 * ExecuteDirectSync
 *
 */

NAN_METHOD(ODBCStatement::ExecuteDirectSync)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteDirectSync - Entry\n");

  Nan::HandleScope scope;

#ifdef UNICODE
  REQ_WSTR_ARG(0, sql);
#else
  REQ_STR_ARG(0, sql);
#endif

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  SQLRETURN ret = SQLExecDirect(
      stmt->m_hSTMT,
      (SQLTCHAR *)*sql,
      sql.length());

  if (ret == SQL_ERROR)
  {
    Nan::ThrowError(ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        stmt->m_hSTMT,
        (char *)"[node-odbc] Error in ODBCStatement::ExecuteDirectSync"));

    info.GetReturnValue().Set(Nan::Null());
  }
  else
  {
    Local<Value> result[4];
    bool *canFreeHandle = new bool(false);

    result[0] = Nan::New<External>((void *)(intptr_t)stmt->m_hENV);
    result[1] = Nan::New<External>((void *)(intptr_t)stmt->m_hDBC);
    result[2] = Nan::New<External>((void *)(intptr_t)stmt->m_hSTMT);
    result[3] = Nan::New<External>((void *)canFreeHandle);

    // TODO persistent leak?
    Nan::Persistent<Object> js_result;
    js_result.Reset(Nan::NewInstance(Nan::New(ODBCResult::constructor), 4, result).ToLocalChecked());

    info.GetReturnValue().Set(Nan::New(js_result));
    // info.GetReturnValue().Set(Nan::Null());
  }
  DEBUG_PRINTF("ODBCStatement::ExecuteDirectSync - Exit\n");
}

/*
 * PrepareSync
 *
 */

NAN_METHOD(ODBCStatement::PrepareSync)
{
  DEBUG_PRINTF("ODBCStatement::PrepareSync - Entry\n");

  Nan::HandleScope scope;

  REQ_STRO_ARG(0, sql);

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  SQLRETURN ret;
  int sqlLen = sql.length();
  void *sql2 = NULL;
  GETCPPSTR(sql2, sql, sqlLen);

  DEBUG_PRINTF("ODBCStatement::PrepareSync:  sqlLen=%i, sql=%s, hSTMT=%X\n", sqlLen, (char *)sql2, stmt->m_hSTMT);

  ret = SQLPrepare(
      stmt->m_hSTMT,
      (SQLTCHAR *)sql2,
      sqlLen);

  free(sql2);
  if (SQL_SUCCEEDED(ret))
  {
    info.GetReturnValue().Set(Nan::True());
  }
  else
  {
    Nan::ThrowError(ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        stmt->m_hSTMT,
        (char *)"[node-odbc] Error in ODBCStatement::PrepareSync"));

    info.GetReturnValue().Set(Nan::False());
  }
  DEBUG_PRINTF("ODBCStatement::PrepareSync - Exit\n");
}

/*
 * Prepare
 *
 */

NAN_METHOD(ODBCStatement::Prepare)
{
  DEBUG_PRINTF("ODBCStatement::Prepare - Entry\n");

  Nan::HandleScope scope;

  REQ_STRO_ARG(0, sql);
  REQ_FUN_ARG(1, cb);
  int sqlLen = sql.length();
  void *cppSQL = NULL;
  GETCPPSTR(cppSQL, sql, sqlLen); // Memory get alloced using malloc

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  if (!work_req)
    free(cppSQL);
  MEMCHECK(work_req);

  prepare_work_data *data =
      (prepare_work_data *)calloc(1, sizeof(prepare_work_data));
  if (!data)
  { // Failed to allocate memory, free allocated resources.
    free(cppSQL);
    free(work_req); // Below MEMCHECK macro will log error and return;
  }
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->sql = cppSQL;
  data->sqlLen = sqlLen;
  data->stmt = stmt;
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_Prepare,
      (uv_after_work_cb)UV_AfterPrepare);

  stmt->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBCStatement::Prepare - Exit\n");
}

void ODBCStatement::UV_Prepare(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_Prepare - Entry\n");

  prepare_work_data *data = (prepare_work_data *)(req->data);

  DEBUG_PRINTF("ODBCStatement::UV_Prepare m_hENV=%X m_hDBC=%X m_hSTMT=%X\n",
               data->stmt->m_hENV,
               data->stmt->m_hDBC,
               data->stmt->m_hSTMT);

  SQLRETURN ret;

  ret = SQLPrepare(
      data->stmt->m_hSTMT,
      (SQLTCHAR *)data->sql,
      data->sqlLen);

  data->result = ret;
  DEBUG_PRINTF("ODBCStatement::UV_Prepare - Exit\n");
}

void ODBCStatement::UV_AfterPrepare(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterPrepare - Entry\n");

  prepare_work_data *data = (prepare_work_data *)(req->data);

  DEBUG_PRINTF("ODBCStatement::UV_AfterPrepare m_hENV=%X m_hDBC=%X m_hSTMT=%X\n",
               data->stmt->m_hENV,
               data->stmt->m_hDBC,
               data->stmt->m_hSTMT);

  Nan::HandleScope scope;

  // First thing, let's check if the execution of the query returned any errors
  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(
        SQL_HANDLE_STMT,
        data->stmt->m_hSTMT,
        data->cb);
  }
  else
  {
    Local<Value> info[2];

    info[0] = Nan::Null();
    info[1] = Nan::True();

    Nan::TryCatch try_catch;

    data->cb->Call(2, info);

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  data->stmt->Unref();
  delete data->cb;

  free(data->sql);
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterPrepare - Exit\n");
}

/*
 * BindSync
 *
 */

NAN_METHOD(ODBCStatement::BindSync)
{
  DEBUG_PRINTF("ODBCStatement::BindSync - Entry\n");
  Nan::HandleScope scope;

  if (!info[0]->IsArray())
  {
    return Nan::ThrowTypeError("Argument 1 must be an Array");
  }
  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  DEBUG_PRINTF("ODBCStatement::BindSync m_hENV=%X m_hDBC=%X m_hSTMT=%X\n",
               stmt->m_hENV,
               stmt->m_hDBC,
               stmt->m_hSTMT);

  // if we previously had parameters, then be sure to free them
  // before allocating more
  if (stmt->paramCount)
  {
    FREE_PARAMS(stmt->params, stmt->paramCount);
  }

  stmt->params = ODBC::GetParametersFromArray(
      Local<Array>::Cast(info[0]),
      &stmt->paramCount);

  SQLRETURN ret = SQL_SUCCESS;

  ret = ODBC::BindParameters(stmt->m_hSTMT, stmt->params, stmt->paramCount);

  if (SQL_SUCCEEDED(ret))
  {
    info.GetReturnValue().Set(Nan::True());
  }
  else
  {
    Nan::ThrowError(ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        stmt->m_hSTMT,
        (char *)"[node-odbc] Error in ODBCStatement::BindSync"));

    info.GetReturnValue().Set(Nan::False());
  }

  // info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBCStatement::BindSync - Exit\n");
}

/*
 * Bind
 *
 */

NAN_METHOD(ODBCStatement::Bind)
{
  DEBUG_PRINTF("ODBCStatement::Bind - Entry\n");

  Nan::HandleScope scope;

  if (!info[0]->IsArray())
  {
    return Nan::ThrowError("Argument 1 must be an Array");
  }

  REQ_FUN_ARG(1, cb);

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  bind_work_data *data =
      (bind_work_data *)calloc(1, sizeof(bind_work_data));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  // if we previously had parameters, then be sure to free them
  // before allocating more
  if (stmt->paramCount)
  {
    FREE_PARAMS(stmt->params, stmt->paramCount);
  }

  data->stmt = stmt;

  DEBUG_PRINTF("ODBCStatement::Bind m_hENV=%X m_hDBC=%X m_hSTMT=%X\n",
               data->stmt->m_hENV,
               data->stmt->m_hDBC,
               data->stmt->m_hSTMT);

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
  DEBUG_PRINTF("ODBCStatement::Bind - Exit\n");
}

void ODBCStatement::UV_Bind(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_Bind - Entry\n");

  bind_work_data *data = (bind_work_data *)(req->data);

  DEBUG_PRINTF("ODBCStatement::UV_Bind m_hENV=%X m_hDBC=%X m_hSTMT=%X\n",
               data->stmt->m_hENV,
               data->stmt->m_hDBC,
               data->stmt->m_hSTMT);

  data->result = ODBC::BindParameters(data->stmt->m_hSTMT,
                                      data->stmt->params, data->stmt->paramCount);
  DEBUG_PRINTF("ODBCStatement::UV_Bind - Exit\n");
}

void ODBCStatement::UV_AfterBind(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterBind - Entry\n");

  bind_work_data *data = (bind_work_data *)(req->data);

  Nan::HandleScope scope;

  // an easy reference to the statment object
  ODBCStatement *self = data->stmt->self();

  // Check if there were errors
  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(
        SQL_HANDLE_STMT,
        self->m_hSTMT,
        data->cb);
  }
  else
  {
    Local<Value> info[2];

    info[0] = Nan::Null();
    info[1] = Nan::True();

    Nan::TryCatch try_catch;

    data->cb->Call(2, info);

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  self->Unref();
  delete data->cb;

  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterBind - Exit\n");
}

/*
 * SetAttrSync
 *
 */

NAN_METHOD(ODBCStatement::SetAttrSync)
{
  DEBUG_PRINTF("ODBCStatement::SetAttrSync - Entry\n");
  Nan::HandleScope scope;

  REQ_ARGS(2);
  REQ_INT_ARG(0, attr);
  SQLPOINTER valuePtr = NULL;
  SQLINTEGER stringLength = SQL_IS_INTEGER;

  if (info[1]->IsInt32())
  {
    valuePtr = (SQLPOINTER)(Nan::To<v8::Int32>(info[1]).ToLocalChecked()->Value());
  }
  else if (info[1]->IsNull())
  {
    valuePtr = (SQLPOINTER)0;
  }
  else if (info[1]->IsString())
  {
    Nan::Utf8String value(info[1]);
    stringLength = value.length();
    GETCPPSTR(valuePtr, value, stringLength);
  }
  else
  {
    return Nan::ThrowTypeError("Unsupported Statement Attribute Value.");
  }

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());
  DEBUG_PRINTF("ODBCStatement::SetAttrSync: hENV=%X, hDBC=%X, hSTMT=%X, "
               "Attribute=%d, value=%i, length=%d\n",
               stmt->m_hENV, stmt->m_hDBC, stmt->m_hSTMT, attr, valuePtr, stringLength);

  SQLRETURN ret = SQLSetStmtAttr(stmt->m_hSTMT, attr, valuePtr, stringLength);

  if (stringLength != SQL_IS_INTEGER)
  {
    FREE(valuePtr);
  }

  if (SQL_SUCCEEDED(ret))
  {
    info.GetReturnValue().Set(Nan::True());
  }
  else
  {
    Nan::ThrowError(ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        stmt->m_hSTMT,
        (char *)"[node-odbc] Error in ODBCStatement::SetAttrSync"));

    info.GetReturnValue().Set(Nan::False());
  }

  DEBUG_PRINTF("ODBCStatement::SetAttrSync - Exit\n");
}

/*
 * SetAttr
 *
 */

NAN_METHOD(ODBCStatement::SetAttr)
{
  DEBUG_PRINTF("ODBCStatement::SetAttr - Entry\n");

  Nan::HandleScope scope;
  SQLPOINTER valuePtr = NULL;
  SQLINTEGER stringLength = SQL_IS_INTEGER;
  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  REQ_ARGS(3);
  REQ_INT_ARG(0, attr);
  REQ_FUN_ARG(2, cb);

  if (info[1]->IsInt32())
  {
    valuePtr = (SQLPOINTER)(Nan::To<v8::Int32>(info[1]).ToLocalChecked()->Value());
  }
  else if (info[1]->IsNull())
  {
    valuePtr = (SQLPOINTER)0;
  }
  else if (info[1]->IsString())
  {
    Nan::Utf8String value(info[1]);
    stringLength = value.length();
    GETCPPSTR(valuePtr, value, stringLength);
  }
  else
  {
    return Nan::ThrowTypeError("Unsupported Statement Attribute Value.");
  }

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  if (!work_req && info[1]->IsString())
  {
    free(valuePtr);
  }
  MEMCHECK(work_req);

  setattr_work_data *data =
      (setattr_work_data *)calloc(1, sizeof(setattr_work_data));
  if (!data)
  {
    if (info[1]->IsString())
      free(valuePtr);
    free(work_req);
  }
  MEMCHECK(data);

  data->stmt = stmt;
  data->attr = attr;
  data->valuePtr = valuePtr;
  data->stringLength = stringLength;

  DEBUG_PRINTF("ODBCStatement::SetAttr: hENV=%X, hDBC=%X, hSTMT=%X, "
               "Attribute=%d, value=%i, length=%d\n",
               data->stmt->m_hENV, data->stmt->m_hDBC, data->stmt->m_hSTMT,
               data->attr, data->valuePtr, data->stringLength);

  data->cb = new Nan::Callback(cb);
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_SetAttr,
      (uv_after_work_cb)UV_AfterSetAttr);

  stmt->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBCStatement::SetAttr - Exit\n");
}

void ODBCStatement::UV_SetAttr(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_SetAttr - Entry\n");

  setattr_work_data *data = (setattr_work_data *)(req->data);

  data->result = SQLSetStmtAttr(data->stmt->m_hSTMT,
                                data->attr,
                                data->valuePtr,
                                data->stringLength);

  DEBUG_PRINTF("ODBCStatement::UV_SetAttr - Exit\n");
}

void ODBCStatement::UV_AfterSetAttr(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterSetAttr - Entry\n");

  setattr_work_data *data = (setattr_work_data *)(req->data);

  Nan::HandleScope scope;

  // an easy reference to the statment object
  ODBCStatement *self = data->stmt->self();

  // Check if there were errors
  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(
        SQL_HANDLE_STMT,
        self->m_hSTMT,
        data->cb);
  }
  else
  {
    Local<Value> info[2];

    info[0] = Nan::Null();
    info[1] = Nan::True();

    Nan::TryCatch try_catch;

    data->cb->Call(2, info);

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  self->Unref();
  delete data->cb;
  if (data->stringLength != SQL_IS_INTEGER)
  {
    FREE(data->valuePtr);
  }

  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterSetAttr - Exit\n");
}

/*
 * Close
 * stmt.close([sql_close,] function(err) {})
 */

NAN_METHOD(ODBCStatement::Close)
{
  DEBUG_PRINTF("ODBCStatement::Close- Entry\n");

  Nan::HandleScope scope;

  Local<Function> cb;
  SQLUSMALLINT closeOption = SQL_DROP;

  if (info.Length() == 2)
  {
    if (!info[0]->IsInt32())
    {
      return Nan::ThrowTypeError("Argument 0 must be an Integer.");
    }
    else if (!info[1]->IsFunction())
    {
      return Nan::ThrowTypeError("Argument 1 must be an Function.");
    }
    closeOption = (SQLUSMALLINT)Nan::To<v8::Int32>(info[0]).ToLocalChecked()->Value();
    cb = Local<Function>::Cast(info[1]);
  }
  else if (info.Length() == 1)
  {
    if (!info[0]->IsFunction())
    {
      return Nan::ThrowTypeError("ODBCStatement::Close(): Argument 0 must be a Function.");
    }
    cb = Local<Function>::Cast(info[0]);
  }

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  close_statement_work_data *data = (close_statement_work_data *)(calloc(1, sizeof(close_statement_work_data)));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->stmt = stmt;
  work_req->data = data;
  data->closeOption = closeOption;

  DEBUG_PRINTF("ODBCStatement::Close closeOption=%d ,stmt=%X\n", closeOption, stmt->m_hSTMT);

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_Close,
      (uv_after_work_cb)UV_AfterClose);

  stmt->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBCStatement::Close - Exit\n");
}
void ODBCStatement::UV_Close(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_Close- Entry\n");

  close_statement_work_data *data = (close_statement_work_data *)(req->data);

  ODBCStatement *stmt = data->stmt;

  DEBUG_PRINTF("ODBCStatement::UV_Close m_hSTMT=%X\n", data->stmt->m_hSTMT);

  SQLRETURN ret;

  if (data->closeOption == SQL_DROP)
  {
    stmt->Free();
    data->result = 0;
  }
  else
  {
    ret = SQLFreeStmt((SQLHSTMT)stmt->m_hSTMT, data->closeOption);
    data->result = ret;
  }

  DEBUG_PRINTF("ODBCStatement::UV_Close - Exit m_hSTMT=%X, result=%i\n", stmt->m_hSTMT, data->result);
}

void ODBCStatement::UV_AfterClose(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterClose- Entry\n");

  Nan::HandleScope scope;

  close_statement_work_data *data = (close_statement_work_data *)(req->data);

  ODBCStatement *self = data->stmt->self();

  if (data->result != SQL_SUCCESS)
  {
    ODBC::CallbackSQLError(
        SQL_HANDLE_STMT,
        data->stmt->m_hSTMT,
        data->cb);
  }
  else
  {
    Local<Value> info[1];

    info[0] = Nan::Null();
    Nan::TryCatch try_catch;
    data->cb->Call(1, info);

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  self->Unref();
  delete data->cb;

  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterClose - Exit\n");
}

/*
 * CloseSync
 */

NAN_METHOD(ODBCStatement::CloseSync)
{
  DEBUG_PRINTF("ODBCStatement::CloseSync - Entry\n");

  Nan::HandleScope scope;

  OPT_INT_ARG(0, closeOption, SQL_DROP);

  ODBCStatement *stmt = Nan::ObjectWrap::Unwrap<ODBCStatement>(info.Holder());

  DEBUG_PRINTF("ODBCStatement::CloseSync closeOption=%i\n",
               closeOption);

  if (closeOption == SQL_DROP)
  {
    stmt->Free();
  }
  else
  {
    SQLFreeStmt(stmt->m_hSTMT, closeOption);
  }

  info.GetReturnValue().Set(Nan::True());
  DEBUG_PRINTF("ODBCStatement::CloseSync - Exit\n");
}

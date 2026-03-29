/*
  Copyright (c) 2013, Dan VerWeire <dverweire@gmail.com>

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
#include <time.h>
#include <uv.h>

#include "odbc.h"
#include "odbc_connection.h"
#include "odbc_result.h"
#include "odbc_statement.h"

Napi::FunctionReference ODBCStatement::constructor;

Napi::Object ODBCStatement::Init(Napi::Env env, Napi::Object exports)
{
  DEBUG_PRINTF("ODBCStatement::Init\n");

  Napi::Function func = DefineClass(env, "ODBCStatement", {
    InstanceMethod("execute", &ODBCStatement::Execute, NAPI_METHOD_ATTR),
    InstanceMethod("executeSync", &ODBCStatement::ExecuteSync, NAPI_METHOD_ATTR),
    InstanceMethod("executeDirect", &ODBCStatement::ExecuteDirect, NAPI_METHOD_ATTR),
    InstanceMethod("executeDirectSync", &ODBCStatement::ExecuteDirectSync, NAPI_METHOD_ATTR),
    InstanceMethod("executeNonQuery", &ODBCStatement::ExecuteNonQuery, NAPI_METHOD_ATTR),
    InstanceMethod("executeNonQuerySync", &ODBCStatement::ExecuteNonQuerySync, NAPI_METHOD_ATTR),
    InstanceMethod("prepare", &ODBCStatement::Prepare, NAPI_METHOD_ATTR),
    InstanceMethod("prepareSync", &ODBCStatement::PrepareSync, NAPI_METHOD_ATTR),
    InstanceMethod("bind", &ODBCStatement::Bind, NAPI_METHOD_ATTR),
    InstanceMethod("bindSync", &ODBCStatement::BindSync, NAPI_METHOD_ATTR),
    InstanceMethod("setAttr", &ODBCStatement::SetAttr, NAPI_METHOD_ATTR),
    InstanceMethod("setAttrSync", &ODBCStatement::SetAttrSync, NAPI_METHOD_ATTR),
    InstanceMethod("close", &ODBCStatement::Close, NAPI_METHOD_ATTR),
    InstanceMethod("closeSync", &ODBCStatement::CloseSync, NAPI_METHOD_ATTR),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("ODBCStatement", func);
  return exports;
}

ODBCStatement::~ODBCStatement()
{
  DEBUG_PRINTF("ODBCStatement::~ODBCStatement m_hSTMT=%X\n", m_hSTMT);
  this->Free();
}

void ODBCStatement::Free()
{
  DEBUG_PRINTF("ODBCStatement::Free - Entry: paramCount=%i, m_hSTMT=%X\n", paramCount, m_hSTMT);
  if (paramCount) { FREE_PARAMS(params, paramCount); }
  if (m_hSTMT) {
    if (!g_shuttingDown) {
      SQLFreeHandle(SQL_HANDLE_STMT, m_hSTMT);
    }
    m_hSTMT = (SQLHSTMT)NULL;
  }
  if (buffer != NULL) { free((uint16_t *)buffer); buffer = NULL; }
  bufferLength = 0;
  DEBUG_PRINTF("ODBCStatement::Free - Exit\n");
}

ODBCStatement::ODBCStatement(const Napi::CallbackInfo &info) : Napi::ObjectWrap<ODBCStatement>(info)
{
  DEBUG_PRINTF("ODBCStatement::New - Entry\n");
  Napi::Env env = info.Env();

  if (info.Length() < 3 || !info[0].IsExternal() || !info[1].IsExternal() || !info[2].IsExternal())
  {
    Napi::TypeError::New(env, "ODBCStatement::New requires 3 external arguments").ThrowAsJavaScriptException();
    return;
  }

  m_hENV = (SQLHENV)((intptr_t)info[0].As<Napi::External<void>>().Data());
  m_hDBC = (SQLHDBC)((intptr_t)info[1].As<Napi::External<void>>().Data());
  m_hSTMT = (SQLHSTMT)((intptr_t)info[2].As<Napi::External<void>>().Data());

  bufferLength = MAX_VALUE_SIZE;
  buffer = NULL;
  colCount = 0;
  paramCount = 0;
  params = 0;
  DEBUG_PRINTF("ODBCStatement::New - Exit\n");
}

/*
 * Execute
 */
Napi::Value ODBCStatement::Execute(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::Execute - Entry\n");
  Napi::Env env = info.Env();
  REQ_FUN_ARG(0, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  execute_work_data *data = (execute_work_data *)calloc(1, sizeof(execute_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->stmt = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Execute, (uv_after_work_cb)UV_AfterExecute);
  this->Ref();
  return env.Undefined();
}

void ODBCStatement::UV_Execute(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_Execute - Entry\n");
  execute_work_data *data = (execute_work_data *)(req->data);

  SQLRETURN ret = SQLExecute(data->stmt->m_hSTMT);
  if (ret == SQL_NEED_DATA)
    ret = ODBC::PutDataLoop(data->stmt->m_hSTMT, data->stmt->params, data->stmt->paramCount);

  data->result = ret;
  DEBUG_PRINTF("ODBCStatement::UV_Execute - Exit\n");
}

void ODBCStatement::UV_AfterExecute(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecute - Entry\n");
  execute_work_data *data = (execute_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  ODBCStatement *stmt = data->stmt->self();
  int outParamCount = 0;
  Napi::Array sp_result = Napi::Array::New(env);

  if (SQL_SUCCEEDED(data->result))
  {
    for (int i = 0; i < stmt->paramCount; i++)
    {
      if (stmt->params[i].paramtype % 2 == 0)
      {
        sp_result.Set(outParamCount, ODBC::GetOutputParameter(env, &stmt->params[i]));
        outParamCount++;
      }
    }
  }
  if (stmt->paramCount) { FREE_PARAMS(stmt->params, stmt->paramCount); }

  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, stmt->m_hSTMT, data->cb);
  }
  else
  {
    bool *canFreeHandle = new bool(false);
    Napi::Value args[4] = {
      Napi::External<void>::New(env, (void *)(intptr_t)stmt->m_hENV),
      Napi::External<void>::New(env, (void *)(intptr_t)stmt->m_hDBC),
      Napi::External<void>::New(env, (void *)(intptr_t)stmt->m_hSTMT),
      Napi::External<void>::New(env, (void *)canFreeHandle)
    };
    Napi::Object js_result = ODBCResult::constructor.New({args[0], args[1], args[2], args[3]});
    data->cb->Call({env.Null(), js_result, outParamCount ? (Napi::Value)sp_result : env.Null()});
  }

  stmt->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecute - Exit\n");
}

/*
 * ExecuteSync
 */
Napi::Value ODBCStatement::ExecuteSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteSync - Entry\n");
  Napi::Env env = info.Env();
  int outParamCount = 0;
  Napi::Array sp_result = Napi::Array::New(env);

  SQLRETURN ret = SQLExecute(m_hSTMT);
  if (ret == SQL_NEED_DATA)
    ret = ODBC::PutDataLoop(m_hSTMT, params, paramCount);

  if (SQL_SUCCEEDED(ret))
  {
    for (int i = 0; i < paramCount; i++)
    {
      if (params[i].paramtype % 2 == 0)
      {
        sp_result.Set(outParamCount, ODBC::GetOutputParameter(env, &params[i]));
        outParamCount++;
      }
    }
  }
  if (paramCount) { FREE_PARAMS(params, paramCount); }

  if (ret == SQL_ERROR)
  {
    Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
      (char *)"[node-odbc] Error in ODBCStatement::ExecuteSync").ToString()).ThrowAsJavaScriptException();
    return env.Null();
  }
  else
  {
    bool *canFreeHandle = new bool(false);
    Napi::Value result[4] = {
      Napi::External<void>::New(env, (void *)(intptr_t)m_hENV),
      Napi::External<void>::New(env, (void *)(intptr_t)m_hDBC),
      Napi::External<void>::New(env, (void *)(intptr_t)m_hSTMT),
      Napi::External<void>::New(env, (void *)canFreeHandle)
    };
    Napi::Object js_result = ODBCResult::constructor.New({result[0], result[1], result[2], result[3]});

    if (outParamCount)
    {
      Napi::Array resultset = Napi::Array::New(env);
      resultset.Set((uint32_t)0, js_result);
      resultset.Set((uint32_t)1, sp_result);
      return resultset;
    }
    return js_result;
  }
}

/*
 * ExecuteNonQuery
 */
Napi::Value ODBCStatement::ExecuteNonQuery(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuery - Entry\n");
  Napi::Env env = info.Env();
  REQ_FUN_ARG(0, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  execute_work_data *data = (execute_work_data *)calloc(1, sizeof(execute_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->stmt = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_ExecuteNonQuery, (uv_after_work_cb)UV_AfterExecuteNonQuery);
  this->Ref();
  return env.Undefined();
}

void ODBCStatement::UV_ExecuteNonQuery(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteNonQuery - Entry\n");
  execute_work_data *data = (execute_work_data *)(req->data);

  SQLRETURN ret = SQLExecute(data->stmt->m_hSTMT);
  if (ret == SQL_NEED_DATA)
    ret = ODBC::PutDataLoop(data->stmt->m_hSTMT, data->stmt->params, data->stmt->paramCount);

  data->result = ret;
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteNonQuery - Exit\n");
}

void ODBCStatement::UV_AfterExecuteNonQuery(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteNonQuery - Entry\n");
  execute_work_data *data = (execute_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  ODBCStatement *self = data->stmt->self();
  SQLLEN rowCount = 0;
  SQLRETURN ret = data->result;
  Napi::Value warning;
  bool hasWarning = false;

  if (ret > SQL_SUCCESS && ret != SQL_NO_DATA_FOUND)
  {
    warning = ODBC::GetSQLError(env, SQL_HANDLE_STMT, self->m_hSTMT,
      (char *)"[node-ibm_db] Warning in ODBCStatement::UV_AfterExecuteNonQuery");
    hasWarning = true;
  }
  if ((ret == SQL_SUCCESS) || (ret == SQL_SUCCESS_WITH_INFO))
    ret = SQLRowCount(self->m_hSTMT, &rowCount);

  if (ret < SQL_SUCCESS)
  {
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, self->m_hSTMT, data->cb);
  }
  else
  {
    Napi::Value err;
    if ((ret > SQL_SUCCESS) && !hasWarning && (ret != SQL_NO_DATA_FOUND))
      err = ODBC::GetSQLError(env, SQL_HANDLE_STMT, self->m_hSTMT,
        (char *)"[node-ibm_db] Warning in ODBCStatement::UV_AfterExecuteNonQuery for SQLRowCount.");
    else if (hasWarning)
      err = warning;
    else
      err = env.Null();

    data->cb->Call({err, Napi::Number::New(env, (double)rowCount)});
  }

  SQLFreeStmt(self->m_hSTMT, SQL_CLOSE);
  self->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteNonQuery - Exit\n");
}

/*
 * ExecuteNonQuerySync
 */
Napi::Value ODBCStatement::ExecuteNonQuerySync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteNonQuerySync - Entry\n");
  Napi::Env env = info.Env();
  SQLLEN rowCount = 0;

  SQLRETURN ret = SQLExecute(m_hSTMT);
  if (ret == SQL_NEED_DATA)
    ret = ODBC::PutDataLoop(m_hSTMT, params, paramCount);

  if ((ret == SQL_SUCCESS) || (ret == SQL_SUCCESS_WITH_INFO))
    ret = SQLRowCount(m_hSTMT, &rowCount);

  if (ret < SQL_SUCCESS)
  {
    Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
      (char *)"[node-ibm_db] Error in ODBCStatement::ExecuteNonQuerySync").ToString()).ThrowAsJavaScriptException();
    SQLFreeStmt(m_hSTMT, SQL_CLOSE);
    return env.Null();
  }

  SQLFreeStmt(m_hSTMT, SQL_CLOSE);
  return Napi::Number::New(env, (double)rowCount);
}

/*
 * ExecuteDirect
 */
Napi::Value ODBCStatement::ExecuteDirect(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteDirect - Entry\n");
  Napi::Env env = info.Env();

  REQ_STRO_ARG(0, sql);
  REQ_FUN_ARG(1, cb);
  int len = (int)sql.length();
  void *cppSQL = NULL;
  GETCPPSTR(cppSQL, sql, len);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  if (!work_req) free(cppSQL);
  MEMCHECK(work_req);
  execute_direct_work_data *data = (execute_direct_work_data *)calloc(1, sizeof(execute_direct_work_data));
  if (!data) { free(cppSQL); free(work_req); }
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->sql = cppSQL;
  data->sqlLen = len;
  data->stmt = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_ExecuteDirect, (uv_after_work_cb)UV_AfterExecuteDirect);
  this->Ref();
  return env.Undefined();
}

void ODBCStatement::UV_ExecuteDirect(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteDirect - Entry\n");
  execute_direct_work_data *data = (execute_direct_work_data *)(req->data);
  data->result = SQLExecDirect(data->stmt->m_hSTMT, (SQLTCHAR *)data->sql, data->sqlLen);
  DEBUG_PRINTF("ODBCStatement::UV_ExecuteDirect - Exit\n");
}

void ODBCStatement::UV_AfterExecuteDirect(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterExecuteDirect - Entry\n");
  execute_direct_work_data *data = (execute_direct_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);
  ODBCStatement *self = data->stmt->self();

  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, self->m_hSTMT, data->cb);
  }
  else
  {
    bool *canFreeHandle = new bool(false);
    Napi::Value args[4] = {
      Napi::External<void>::New(env, (void *)(intptr_t)self->m_hENV),
      Napi::External<void>::New(env, (void *)(intptr_t)self->m_hDBC),
      Napi::External<void>::New(env, (void *)(intptr_t)self->m_hSTMT),
      Napi::External<void>::New(env, (void *)canFreeHandle)
    };
    Napi::Object js_result = ODBCResult::constructor.New({args[0], args[1], args[2], args[3]});
    data->cb->Call({env.Null(), js_result});
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
 */
Napi::Value ODBCStatement::ExecuteDirectSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ExecuteDirectSync - Entry\n");
  Napi::Env env = info.Env();

  REQ_STRO_ARG(0, sql);
  int len = (int)sql.length();
  void *cppSQL = NULL;
  GETCPPSTR(cppSQL, sql, len);

  SQLRETURN ret = SQLExecDirect(m_hSTMT, (SQLTCHAR *)cppSQL, len);
  free(cppSQL);

  if (ret == SQL_ERROR)
  {
    Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
      (char *)"[node-odbc] Error in ODBCStatement::ExecuteDirectSync").ToString()).ThrowAsJavaScriptException();
    return env.Null();
  }

  bool *canFreeHandle = new bool(false);
  Napi::Value result[4] = {
    Napi::External<void>::New(env, (void *)(intptr_t)m_hENV),
    Napi::External<void>::New(env, (void *)(intptr_t)m_hDBC),
    Napi::External<void>::New(env, (void *)(intptr_t)m_hSTMT),
    Napi::External<void>::New(env, (void *)canFreeHandle)
  };
  return ODBCResult::constructor.New({result[0], result[1], result[2], result[3]});
}

/*
 * PrepareSync
 */
Napi::Value ODBCStatement::PrepareSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::PrepareSync - Entry\n");
  Napi::Env env = info.Env();

  REQ_STRO_ARG(0, sql);
  int sqlLen = (int)sql.length();
  void *sql2 = NULL;
  GETCPPSTR(sql2, sql, sqlLen);

  SQLRETURN ret = SQLPrepare(m_hSTMT, (SQLTCHAR *)sql2, sqlLen);
  free(sql2);

  if (SQL_SUCCEEDED(ret))
    return Napi::Boolean::New(env, true);

  Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
    (char *)"[node-odbc] Error in ODBCStatement::PrepareSync").ToString()).ThrowAsJavaScriptException();
  return Napi::Boolean::New(env, false);
}

/*
 * Prepare
 */
Napi::Value ODBCStatement::Prepare(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::Prepare - Entry\n");
  Napi::Env env = info.Env();

  REQ_STRO_ARG(0, sql);
  REQ_FUN_ARG(1, cb);
  int sqlLen = (int)sql.length();
  void *cppSQL = NULL;
  GETCPPSTR(cppSQL, sql, sqlLen);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  if (!work_req) free(cppSQL);
  MEMCHECK(work_req);
  prepare_work_data *data = (prepare_work_data *)calloc(1, sizeof(prepare_work_data));
  if (!data) { free(cppSQL); free(work_req); }
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->sql = cppSQL;
  data->sqlLen = sqlLen;
  data->stmt = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Prepare, (uv_after_work_cb)UV_AfterPrepare);
  this->Ref();
  return env.Undefined();
}

void ODBCStatement::UV_Prepare(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_Prepare - Entry\n");
  prepare_work_data *data = (prepare_work_data *)(req->data);
  data->result = SQLPrepare(data->stmt->m_hSTMT, (SQLTCHAR *)data->sql, data->sqlLen);
  DEBUG_PRINTF("ODBCStatement::UV_Prepare - Exit\n");
}

void ODBCStatement::UV_AfterPrepare(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterPrepare - Entry\n");
  prepare_work_data *data = (prepare_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (data->result == SQL_ERROR)
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, data->stmt->m_hSTMT, data->cb);
  else
    data->cb->Call({env.Null(), Napi::Boolean::New(env, true)});

  data->stmt->Unref();
  delete data->cb;
  free(data->sql);
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterPrepare - Exit\n");
}

/*
 * BindSync
 */
Napi::Value ODBCStatement::BindSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::BindSync - Entry\n");
  Napi::Env env = info.Env();

  if (!info[0].IsArray())
  {
    Napi::TypeError::New(env, "Argument 1 must be an Array").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (paramCount) { FREE_PARAMS(params, paramCount); }
  params = ODBC::GetParametersFromArray(env, info[0].As<Napi::Array>(), &paramCount);
  SQLRETURN ret = ODBC::BindParameters(m_hSTMT, params, paramCount);

  if (SQL_SUCCEEDED(ret))
    return Napi::Boolean::New(env, true);

  Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
    (char *)"[node-odbc] Error in ODBCStatement::BindSync").ToString()).ThrowAsJavaScriptException();
  return Napi::Boolean::New(env, false);
}

/*
 * Bind
 */
Napi::Value ODBCStatement::Bind(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::Bind - Entry\n");
  Napi::Env env = info.Env();

  if (!info[0].IsArray())
  {
    Napi::TypeError::New(env, "Argument 1 must be an Array").ThrowAsJavaScriptException();
    return env.Undefined();
  }
  REQ_FUN_ARG(1, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  bind_work_data *data = (bind_work_data *)calloc(1, sizeof(bind_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  if (paramCount) { FREE_PARAMS(params, paramCount); }

  data->stmt = this;
  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->env = env;

  params = ODBC::GetParametersFromArray(env, info[0].As<Napi::Array>(), &paramCount);
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Bind, (uv_after_work_cb)UV_AfterBind);
  this->Ref();
  return env.Undefined();
}

void ODBCStatement::UV_Bind(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_Bind - Entry\n");
  bind_work_data *data = (bind_work_data *)(req->data);
  data->result = ODBC::BindParameters(data->stmt->m_hSTMT, data->stmt->params, data->stmt->paramCount);
  DEBUG_PRINTF("ODBCStatement::UV_Bind - Exit\n");
}

void ODBCStatement::UV_AfterBind(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterBind - Entry\n");
  bind_work_data *data = (bind_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);
  ODBCStatement *self = data->stmt->self();

  if (data->result == SQL_ERROR)
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, self->m_hSTMT, data->cb);
  else
    data->cb->Call({env.Null(), Napi::Boolean::New(env, true)});

  self->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterBind - Exit\n");
}

/*
 * SetAttrSync
 */
Napi::Value ODBCStatement::SetAttrSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::SetAttrSync - Entry\n");
  Napi::Env env = info.Env();
  REQ_ARGS(2);
  REQ_INT_ARG(0, attr);

  SQLPOINTER valuePtr = NULL;
  SQLINTEGER stringLength = SQL_IS_INTEGER;

  if (info[1].IsNumber())
    valuePtr = reinterpret_cast<SQLPOINTER>(static_cast<intptr_t>(info[1].As<Napi::Number>().Int32Value()));
  else if (info[1].IsNull())
    valuePtr = (SQLPOINTER)0;
  else if (info[1].IsString())
  {
    std::string value = info[1].As<Napi::String>().Utf8Value();
    stringLength = (SQLINTEGER)value.length();
    GETCPPSTR(valuePtr, value, stringLength);
  }
  else
  {
    Napi::TypeError::New(env, "Unsupported Statement Attribute Value.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  SQLRETURN ret = SQLSetStmtAttr(m_hSTMT, attr, valuePtr, stringLength);

  if (stringLength != SQL_IS_INTEGER) { FREE(valuePtr); }

  if (SQL_SUCCEEDED(ret))
    return Napi::Boolean::New(env, true);

  Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
    (char *)"[node-odbc] Error in ODBCStatement::SetAttrSync").ToString()).ThrowAsJavaScriptException();
  return Napi::Boolean::New(env, false);
}

/*
 * SetAttr
 */
Napi::Value ODBCStatement::SetAttr(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::SetAttr - Entry\n");
  Napi::Env env = info.Env();
  REQ_ARGS(3);
  REQ_INT_ARG(0, attr);
  REQ_FUN_ARG(2, cb);

  SQLPOINTER valuePtr = NULL;
  SQLINTEGER stringLength = SQL_IS_INTEGER;

  if (info[1].IsNumber())
    valuePtr = reinterpret_cast<SQLPOINTER>(static_cast<intptr_t>(info[1].As<Napi::Number>().Int32Value()));
  else if (info[1].IsNull())
    valuePtr = (SQLPOINTER)0;
  else if (info[1].IsString())
  {
    std::string value = info[1].As<Napi::String>().Utf8Value();
    stringLength = (SQLINTEGER)value.length();
    GETCPPSTR(valuePtr, value, stringLength);
  }
  else
  {
    Napi::TypeError::New(env, "Unsupported Statement Attribute Value.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  if (!work_req && info[1].IsString()) free(valuePtr);
  MEMCHECK(work_req);
  stmt_setattr_work_data *data = (stmt_setattr_work_data *)calloc(1, sizeof(stmt_setattr_work_data));
  if (!data) { if (info[1].IsString()) free(valuePtr); free(work_req); }
  MEMCHECK(data);

  data->stmt = this;
  data->attr = attr;
  data->valuePtr = valuePtr;
  data->stringLength = stringLength;
  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_SetAttr, (uv_after_work_cb)UV_AfterSetAttr);
  this->Ref();
  return env.Undefined();
}

void ODBCStatement::UV_SetAttr(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_SetAttr - Entry\n");
  stmt_setattr_work_data *data = (stmt_setattr_work_data *)(req->data);
  data->result = SQLSetStmtAttr(data->stmt->m_hSTMT, data->attr, data->valuePtr, data->stringLength);
  DEBUG_PRINTF("ODBCStatement::UV_SetAttr - Exit\n");
}

void ODBCStatement::UV_AfterSetAttr(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterSetAttr - Entry\n");
  stmt_setattr_work_data *data = (stmt_setattr_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);
  ODBCStatement *self = data->stmt->self();

  if (data->result == SQL_ERROR)
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, self->m_hSTMT, data->cb);
  else
    data->cb->Call({env.Null(), Napi::Boolean::New(env, true)});

  self->Unref();
  delete data->cb;
  if (data->stringLength != SQL_IS_INTEGER) { FREE(data->valuePtr); }
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterSetAttr - Exit\n");
}

/*
 * Close
 */
Napi::Value ODBCStatement::Close(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::Close - Entry\n");
  Napi::Env env = info.Env();
  Napi::Function cb;
  SQLUSMALLINT closeOption = SQL_DROP;

  if (info.Length() == 2)
  {
    if (!info[0].IsNumber())
    {
      Napi::TypeError::New(env, "Argument 0 must be an Integer.").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    if (!info[1].IsFunction())
    {
      Napi::TypeError::New(env, "Argument 1 must be a Function.").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    closeOption = (SQLUSMALLINT)info[0].As<Napi::Number>().Int32Value();
    cb = info[1].As<Napi::Function>();
  }
  else if (info.Length() == 1)
  {
    if (!info[0].IsFunction())
    {
      Napi::TypeError::New(env, "ODBCStatement::Close(): Argument 0 must be a Function.").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    cb = info[0].As<Napi::Function>();
  }

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  stmt_close_work_data *data = (stmt_close_work_data *)(calloc(1, sizeof(stmt_close_work_data)));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->stmt = this;
  data->closeOption = closeOption;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Close, (uv_after_work_cb)UV_AfterClose);
  this->Ref();
  return env.Undefined();
}

void ODBCStatement::UV_Close(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_Close - Entry\n");
  stmt_close_work_data *data = (stmt_close_work_data *)(req->data);
  ODBCStatement *stmt = data->stmt;

  if (data->closeOption == SQL_DROP)
  {
    stmt->Free();
    data->result = 0;
  }
  else
  {
    data->result = SQLFreeStmt((SQLHSTMT)stmt->m_hSTMT, data->closeOption);
  }
  DEBUG_PRINTF("ODBCStatement::UV_Close - Exit\n");
}

void ODBCStatement::UV_AfterClose(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterClose - Entry\n");
  stmt_close_work_data *data = (stmt_close_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);
  ODBCStatement *self = data->stmt->self();

  if (data->result != SQL_SUCCESS)
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, data->stmt->m_hSTMT, data->cb);
  else
    data->cb->Call({env.Null()});

  self->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterClose - Exit\n");
}

/*
 * CloseSync
 */
Napi::Value ODBCStatement::CloseSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::CloseSync - Entry\n");
  Napi::Env env = info.Env();
  SQLUSMALLINT closeOption = SQL_DROP;

  if (info.Length() > 0 && info[0].IsNumber())
    closeOption = (SQLUSMALLINT)info[0].As<Napi::Number>().Int32Value();

  if (closeOption == SQL_DROP)
    this->Free();
  else
    SQLFreeStmt(m_hSTMT, closeOption);

  return Napi::Boolean::New(env, true);
}

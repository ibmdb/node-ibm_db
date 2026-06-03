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
    InstanceMethod("primaryKeys", &ODBCStatement::PrimaryKeys, NAPI_METHOD_ATTR),
    InstanceMethod("primaryKeysSync", &ODBCStatement::PrimaryKeysSync, NAPI_METHOD_ATTR),
    InstanceMethod("foreignKeys", &ODBCStatement::ForeignKeys, NAPI_METHOD_ATTR),
    InstanceMethod("foreignKeysSync", &ODBCStatement::ForeignKeysSync, NAPI_METHOD_ATTR),
    InstanceMethod("procedures", &ODBCStatement::Procedures, NAPI_METHOD_ATTR),
    InstanceMethod("proceduresSync", &ODBCStatement::ProceduresSync, NAPI_METHOD_ATTR),
    InstanceMethod("procedureColumns", &ODBCStatement::ProcedureColumns, NAPI_METHOD_ATTR),
    InstanceMethod("procedureColumnsSync", &ODBCStatement::ProcedureColumnsSync, NAPI_METHOD_ATTR),
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
    SQLFreeHandle(SQL_HANDLE_STMT, m_hSTMT);
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
    napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
      (char *)"[node-odbc] Error in ODBCStatement::ExecuteSync"));
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
    napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
      (char *)"[node-ibm_db] Error in ODBCStatement::ExecuteNonQuerySync"));
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
    napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
      (char *)"[node-odbc] Error in ODBCStatement::ExecuteDirectSync"));
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

  napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
    (char *)"[node-odbc] Error in ODBCStatement::PrepareSync"));
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

  napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
    (char *)"[node-odbc] Error in ODBCStatement::BindSync"));
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

  napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
    (char *)"[node-odbc] Error in ODBCStatement::SetAttrSync"));
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

/*
 * PrimaryKeys
 */
Napi::Value ODBCStatement::PrimaryKeys(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::PrimaryKeys - Entry\n");
  Napi::Env env = info.Env();
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;
  primary_keys_work_data *data = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, table);
  REQ_FUN_ARG(3, cb);

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK2(work_req, errmsg);
  data = (primary_keys_work_data *)calloc(1, sizeof(primary_keys_work_data));
  MEMCHECK2(data, errmsg);

  data->catalog = NULL; data->schema = NULL; data->table = NULL;

  int len;
  len = (int)catalog.length(); GETCPPSTR2(data->catalog, catalog, len, errmsg);
  len = (int)schema.length();  GETCPPSTR2(data->schema,  schema,  len, errmsg);
  len = (int)table.length();   GETCPPSTR2(data->table,   table,   len, errmsg);

  data->cb   = new Napi::FunctionReference(Napi::Persistent(cb));
  data->stmt = this;
  data->env  = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_PrimaryKeys, (uv_after_work_cb)UV_AfterPrimaryKeys);
  this->Ref();

exit:
  if (errmsg)
  {
    if (data) { FREE(data->catalog); FREE(data->schema); FREE(data->table); free(data); }
    free(work_req);
    Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
  }
  return env.Undefined();
}

void ODBCStatement::UV_PrimaryKeys(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_PrimaryKeys - Entry\n");
  primary_keys_work_data *data = (primary_keys_work_data *)(req->data);

  data->result = SQLPrimaryKeys(data->stmt->m_hSTMT,
#ifdef __MVS__
    NULL, 0,
#else
    (SQLTCHAR *)data->catalog, SQL_NTS,
#endif
    (SQLTCHAR *)data->schema, SQL_NTS,
    (SQLTCHAR *)data->table,  SQL_NTS);

  FREE(data->catalog); FREE(data->schema); FREE(data->table);
  DEBUG_PRINTF("ODBCStatement::UV_PrimaryKeys - Exit\n");
}

void ODBCStatement::UV_AfterPrimaryKeys(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterPrimaryKeys - Entry\n");
  primary_keys_work_data *data = (primary_keys_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  ODBCStatement *stmt = data->stmt->self();

  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, stmt->m_hSTMT, data->cb);
    SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
  }
  else
  {
    {
      Napi::Array rows = ODBC::GetAllRecordsSync(env, stmt->m_hENV, stmt->m_hDBC,
                                                 stmt->m_hSTMT, NULL, MAX_VALUE_SIZE);
      SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
      data->cb->Call({env.Null(), rows});
    }
  }
  PropagateCallbackException(env);

  stmt->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterPrimaryKeys - Exit\n");
}

/*
 * PrimaryKeysSync
 */
Napi::Value ODBCStatement::PrimaryKeysSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::PrimaryKeysSync - Entry\n");
  Napi::Env env = info.Env();
  void *cppCatalog = NULL, *cppSchema = NULL, *cppTable = NULL;
  int len;
  const char *errmsg = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, table);

  len = (int)catalog.length(); GETCPPSTR2(cppCatalog, catalog, len, errmsg);
  len = (int)schema.length();  GETCPPSTR2(cppSchema,  schema,  len, errmsg);
  len = (int)table.length();   GETCPPSTR2(cppTable,   table,   len, errmsg);

  {
    SQLRETURN ret = SQLPrimaryKeys(m_hSTMT,
#ifdef __MVS__
      NULL, 0,
#else
      (SQLTCHAR *)cppCatalog, SQL_NTS,
#endif
      (SQLTCHAR *)cppSchema, SQL_NTS,
      (SQLTCHAR *)cppTable,  SQL_NTS);

    FREE(cppCatalog); FREE(cppSchema); FREE(cppTable);

    if (ret == SQL_ERROR)
    {
      napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
        (char *)"[node-ibm_db] Error in ODBCStatement::PrimaryKeysSync"));
      SQLFreeStmt(m_hSTMT, SQL_CLOSE);
      return env.Null();
    }
  }

  {
    Napi::Array rows = ODBC::GetAllRecordsSync(env, m_hENV, m_hDBC, m_hSTMT, NULL, MAX_VALUE_SIZE);
    SQLFreeStmt(m_hSTMT, SQL_CLOSE);
    DEBUG_PRINTF("ODBCStatement::PrimaryKeysSync - Exit\n");
    return rows;
  }

exit:
  FREE(cppCatalog); FREE(cppSchema); FREE(cppTable);
  if (errmsg) Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
  return env.Null();
}

/*
 * ForeignKeys
 */
Napi::Value ODBCStatement::ForeignKeys(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ForeignKeys - Entry\n");
  Napi::Env env = info.Env();
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;
  foreign_keys_work_data *data = NULL;

  REQ_STRO_OR_NULL_ARG(0, pkCatalog);
  REQ_STRO_OR_NULL_ARG(1, pkSchema);
  REQ_STRO_OR_NULL_ARG(2, pkTable);
  REQ_STRO_OR_NULL_ARG(3, fkCatalog);
  REQ_STRO_OR_NULL_ARG(4, fkSchema);
  REQ_STRO_OR_NULL_ARG(5, fkTable);
  REQ_FUN_ARG(6, cb);

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK2(work_req, errmsg);
  data = (foreign_keys_work_data *)calloc(1, sizeof(foreign_keys_work_data));
  MEMCHECK2(data, errmsg);

  data->pkCatalog = NULL; data->pkSchema = NULL; data->pkTable = NULL;
  data->fkCatalog = NULL; data->fkSchema = NULL; data->fkTable = NULL;

  int len;
  len = (int)pkCatalog.length(); GETCPPSTR2(data->pkCatalog, pkCatalog, len, errmsg);
  len = (int)pkSchema.length();  GETCPPSTR2(data->pkSchema,  pkSchema,  len, errmsg);
  len = (int)pkTable.length();   GETCPPSTR2(data->pkTable,   pkTable,   len, errmsg);
  len = (int)fkCatalog.length(); GETCPPSTR2(data->fkCatalog, fkCatalog, len, errmsg);
  len = (int)fkSchema.length();  GETCPPSTR2(data->fkSchema,  fkSchema,  len, errmsg);
  len = (int)fkTable.length();   GETCPPSTR2(data->fkTable,   fkTable,   len, errmsg);

  data->cb   = new Napi::FunctionReference(Napi::Persistent(cb));
  data->stmt = this;
  data->env  = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_ForeignKeys, (uv_after_work_cb)UV_AfterForeignKeys);
  this->Ref();

exit:
  if (errmsg)
  {
    if (data) { FREE(data->pkCatalog); FREE(data->pkSchema); FREE(data->pkTable);
                FREE(data->fkCatalog); FREE(data->fkSchema); FREE(data->fkTable); free(data); }
    free(work_req);
    Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
  }
  return env.Undefined();
}

void ODBCStatement::UV_ForeignKeys(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_ForeignKeys - Entry\n");
  foreign_keys_work_data *data = (foreign_keys_work_data *)(req->data);

  data->result = SQLForeignKeys(data->stmt->m_hSTMT,
#ifdef __MVS__
    NULL, 0,
#else
    (SQLTCHAR *)data->pkCatalog, SQL_NTS,
#endif
    (SQLTCHAR *)data->pkSchema,  SQL_NTS,
    (SQLTCHAR *)data->pkTable,   SQL_NTS,
#ifdef __MVS__
    NULL, 0,
#else
    (SQLTCHAR *)data->fkCatalog, SQL_NTS,
#endif
    (SQLTCHAR *)data->fkSchema,  SQL_NTS,
    (SQLTCHAR *)data->fkTable,   SQL_NTS);

  FREE(data->pkCatalog); FREE(data->pkSchema); FREE(data->pkTable);
  FREE(data->fkCatalog); FREE(data->fkSchema); FREE(data->fkTable);
  DEBUG_PRINTF("ODBCStatement::UV_ForeignKeys - Exit\n");
}

void ODBCStatement::UV_AfterForeignKeys(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterForeignKeys - Entry\n");
  foreign_keys_work_data *data = (foreign_keys_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  ODBCStatement *stmt = data->stmt->self();

  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, stmt->m_hSTMT, data->cb);
    SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
  }
  else
  {
    {
      Napi::Array rows = ODBC::GetAllRecordsSync(env, stmt->m_hENV, stmt->m_hDBC,
                                                 stmt->m_hSTMT, NULL, MAX_VALUE_SIZE);
      SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
      data->cb->Call({env.Null(), rows});
    }
  }
  PropagateCallbackException(env);

  stmt->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterForeignKeys - Exit\n");
}

/*
 * ForeignKeysSync
 */
Napi::Value ODBCStatement::ForeignKeysSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ForeignKeysSync - Entry\n");
  Napi::Env env = info.Env();
  void *cppPkCatalog = NULL, *cppPkSchema = NULL, *cppPkTable = NULL;
  void *cppFkCatalog = NULL, *cppFkSchema = NULL, *cppFkTable = NULL;
  int len;
  const char *errmsg = NULL;

  REQ_STRO_OR_NULL_ARG(0, pkCatalog);
  REQ_STRO_OR_NULL_ARG(1, pkSchema);
  REQ_STRO_OR_NULL_ARG(2, pkTable);
  REQ_STRO_OR_NULL_ARG(3, fkCatalog);
  REQ_STRO_OR_NULL_ARG(4, fkSchema);
  REQ_STRO_OR_NULL_ARG(5, fkTable);

  len = (int)pkCatalog.length(); GETCPPSTR2(cppPkCatalog, pkCatalog, len, errmsg);
  len = (int)pkSchema.length();  GETCPPSTR2(cppPkSchema,  pkSchema,  len, errmsg);
  len = (int)pkTable.length();   GETCPPSTR2(cppPkTable,   pkTable,   len, errmsg);
  len = (int)fkCatalog.length(); GETCPPSTR2(cppFkCatalog, fkCatalog, len, errmsg);
  len = (int)fkSchema.length();  GETCPPSTR2(cppFkSchema,  fkSchema,  len, errmsg);
  len = (int)fkTable.length();   GETCPPSTR2(cppFkTable,   fkTable,   len, errmsg);

  {
    SQLRETURN ret = SQLForeignKeys(m_hSTMT,
#ifdef __MVS__
      NULL, 0,
#else
      (SQLTCHAR *)cppPkCatalog, SQL_NTS,
#endif
      (SQLTCHAR *)cppPkSchema, SQL_NTS,
      (SQLTCHAR *)cppPkTable,  SQL_NTS,
#ifdef __MVS__
      NULL, 0,
#else
      (SQLTCHAR *)cppFkCatalog, SQL_NTS,
#endif
      (SQLTCHAR *)cppFkSchema, SQL_NTS,
      (SQLTCHAR *)cppFkTable,  SQL_NTS);

    FREE(cppPkCatalog); FREE(cppPkSchema); FREE(cppPkTable);
    FREE(cppFkCatalog); FREE(cppFkSchema); FREE(cppFkTable);

    if (ret == SQL_ERROR)
    {
      napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
        (char *)"[node-ibm_db] Error in ODBCStatement::ForeignKeysSync"));
      SQLFreeStmt(m_hSTMT, SQL_CLOSE);
      return env.Null();
    }
  }

  {
    Napi::Array rows = ODBC::GetAllRecordsSync(env, m_hENV, m_hDBC, m_hSTMT, NULL, MAX_VALUE_SIZE);
    SQLFreeStmt(m_hSTMT, SQL_CLOSE);
    DEBUG_PRINTF("ODBCStatement::ForeignKeysSync - Exit\n");
    return rows;
  }

exit:
  FREE(cppPkCatalog); FREE(cppPkSchema); FREE(cppPkTable);
  FREE(cppFkCatalog); FREE(cppFkSchema); FREE(cppFkTable);
  if (errmsg) Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
  return env.Null();
}

/*
 * Procedures
 */
Napi::Value ODBCStatement::Procedures(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::Procedures - Entry\n");
  Napi::Env env = info.Env();
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;
  procedures_work_data *data = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, procedure);
  REQ_FUN_ARG(3, cb);

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK2(work_req, errmsg);
  data = (procedures_work_data *)calloc(1, sizeof(procedures_work_data));
  MEMCHECK2(data, errmsg);

  data->catalog = NULL; data->schema = NULL; data->procedure = NULL;

  int len;
  len = (int)catalog.length();   GETCPPSTR2(data->catalog,   catalog,   len, errmsg);
  len = (int)schema.length();    GETCPPSTR2(data->schema,    schema,    len, errmsg);
  len = (int)procedure.length(); GETCPPSTR2(data->procedure, procedure, len, errmsg);

  data->cb   = new Napi::FunctionReference(Napi::Persistent(cb));
  data->stmt = this;
  data->env  = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Procedures, (uv_after_work_cb)UV_AfterProcedures);
  this->Ref();

exit:
  if (errmsg)
  {
    if (data) { FREE(data->catalog); FREE(data->schema); FREE(data->procedure); free(data); }
    free(work_req);
    Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
  }
  return env.Undefined();
}

void ODBCStatement::UV_Procedures(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_Procedures - Entry\n");
  procedures_work_data *data = (procedures_work_data *)(req->data);

  data->result = SQLProcedures(data->stmt->m_hSTMT,
#ifdef __MVS__
    NULL, 0,
#else
    (SQLTCHAR *)data->catalog, SQL_NTS,
#endif
    (SQLTCHAR *)data->schema, SQL_NTS,
    (SQLTCHAR *)data->procedure, SQL_NTS);

  FREE(data->catalog); FREE(data->schema); FREE(data->procedure);
  DEBUG_PRINTF("ODBCStatement::UV_Procedures - Exit\n");
}

void ODBCStatement::UV_AfterProcedures(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterProcedures - Entry\n");
  procedures_work_data *data = (procedures_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  ODBCStatement *stmt = data->stmt->self();

  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, stmt->m_hSTMT, data->cb);
    SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
  }
  else
  {
    {
      Napi::Array rows = ODBC::GetAllRecordsSync(env, stmt->m_hENV, stmt->m_hDBC,
                                                 stmt->m_hSTMT, NULL, MAX_VALUE_SIZE);
      SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
      data->cb->Call({env.Null(), rows});
    }
  }
  PropagateCallbackException(env);

  stmt->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterProcedures - Exit\n");
}

/*
 * ProceduresSync
 */
Napi::Value ODBCStatement::ProceduresSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ProceduresSync - Entry\n");
  Napi::Env env = info.Env();
  void *cppCatalog = NULL, *cppSchema = NULL, *cppProcedure = NULL;
  int len;
  const char *errmsg = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, procedure);

  len = (int)catalog.length();   GETCPPSTR2(cppCatalog,   catalog,   len, errmsg);
  len = (int)schema.length();    GETCPPSTR2(cppSchema,    schema,    len, errmsg);
  len = (int)procedure.length(); GETCPPSTR2(cppProcedure, procedure, len, errmsg);

  {
    SQLRETURN ret = SQLProcedures(m_hSTMT,
#ifdef __MVS__
      NULL, 0,
#else
      (SQLTCHAR *)cppCatalog, SQL_NTS,
#endif
      (SQLTCHAR *)cppSchema, SQL_NTS,
      (SQLTCHAR *)cppProcedure, SQL_NTS);

    FREE(cppCatalog); FREE(cppSchema); FREE(cppProcedure);

    if (ret == SQL_ERROR)
    {
      napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
        (char *)"[node-ibm_db] Error in ODBCStatement::ProceduresSync"));
      SQLFreeStmt(m_hSTMT, SQL_CLOSE);
      return env.Null();
    }
  }

  {
    Napi::Array rows = ODBC::GetAllRecordsSync(env, m_hENV, m_hDBC, m_hSTMT, NULL, MAX_VALUE_SIZE);
    SQLFreeStmt(m_hSTMT, SQL_CLOSE);
    DEBUG_PRINTF("ODBCStatement::ProceduresSync - Exit\n");
    return rows;
  }

exit:
  FREE(cppCatalog); FREE(cppSchema); FREE(cppProcedure);
  if (errmsg) Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
  return env.Null();
}

/*
 * ProcedureColumns
 */
Napi::Value ODBCStatement::ProcedureColumns(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ProcedureColumns - Entry\n");
  Napi::Env env = info.Env();
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;
  procedure_columns_work_data *data = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, procedure);
  REQ_STRO_OR_NULL_ARG(3, column);
  REQ_FUN_ARG(4, cb);

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK2(work_req, errmsg);
  data = (procedure_columns_work_data *)calloc(1, sizeof(procedure_columns_work_data));
  MEMCHECK2(data, errmsg);

  data->catalog = NULL; data->schema = NULL; data->procedure = NULL; data->column = NULL;

  int len;
  len = (int)catalog.length();   GETCPPSTR2(data->catalog,   catalog,   len, errmsg);
  len = (int)schema.length();    GETCPPSTR2(data->schema,    schema,    len, errmsg);
  len = (int)procedure.length(); GETCPPSTR2(data->procedure, procedure, len, errmsg);
  len = (int)column.length();    GETCPPSTR2(data->column,    column,    len, errmsg);

  data->cb   = new Napi::FunctionReference(Napi::Persistent(cb));
  data->stmt = this;
  data->env  = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_ProcedureColumns, (uv_after_work_cb)UV_AfterProcedureColumns);
  this->Ref();

exit:
  if (errmsg)
  {
    if (data) { FREE(data->catalog); FREE(data->schema); FREE(data->procedure); FREE(data->column); free(data); }
    free(work_req);
    Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
  }
  return env.Undefined();
}

void ODBCStatement::UV_ProcedureColumns(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCStatement::UV_ProcedureColumns - Entry\n");
  procedure_columns_work_data *data = (procedure_columns_work_data *)(req->data);

  data->result = SQLProcedureColumns(data->stmt->m_hSTMT,
#ifdef __MVS__
    NULL, 0,
#else
    (SQLTCHAR *)data->catalog, SQL_NTS,
#endif
    (SQLTCHAR *)data->schema, SQL_NTS,
    (SQLTCHAR *)data->procedure, SQL_NTS,
    (SQLTCHAR *)data->column, SQL_NTS);

  FREE(data->catalog); FREE(data->schema); FREE(data->procedure); FREE(data->column);
  DEBUG_PRINTF("ODBCStatement::UV_ProcedureColumns - Exit\n");
}

void ODBCStatement::UV_AfterProcedureColumns(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCStatement::UV_AfterProcedureColumns - Entry\n");
  procedure_columns_work_data *data = (procedure_columns_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  ODBCStatement *stmt = data->stmt->self();

  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, stmt->m_hSTMT, data->cb);
    SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
  }
  else
  {
    {
      Napi::Array rows = ODBC::GetAllRecordsSync(env, stmt->m_hENV, stmt->m_hDBC,
                                                 stmt->m_hSTMT, NULL, MAX_VALUE_SIZE);
      SQLFreeStmt(stmt->m_hSTMT, SQL_CLOSE);
      data->cb->Call({env.Null(), rows});
    }
  }
  PropagateCallbackException(env);

  stmt->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCStatement::UV_AfterProcedureColumns - Exit\n");
}

/*
 * ProcedureColumnsSync
 */
Napi::Value ODBCStatement::ProcedureColumnsSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCStatement::ProcedureColumnsSync - Entry\n");
  Napi::Env env = info.Env();
  void *cppCatalog = NULL, *cppSchema = NULL, *cppProcedure = NULL, *cppColumn = NULL;
  int len;
  const char *errmsg = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, procedure);
  REQ_STRO_OR_NULL_ARG(3, column);

  len = (int)catalog.length();   GETCPPSTR2(cppCatalog,   catalog,   len, errmsg);
  len = (int)schema.length();    GETCPPSTR2(cppSchema,    schema,    len, errmsg);
  len = (int)procedure.length(); GETCPPSTR2(cppProcedure, procedure, len, errmsg);
  len = (int)column.length();    GETCPPSTR2(cppColumn,    column,    len, errmsg);

  {
    SQLRETURN ret = SQLProcedureColumns(m_hSTMT,
#ifdef __MVS__
      NULL, 0,
#else
      (SQLTCHAR *)cppCatalog, SQL_NTS,
#endif
      (SQLTCHAR *)cppSchema, SQL_NTS,
      (SQLTCHAR *)cppProcedure, SQL_NTS,
      (SQLTCHAR *)cppColumn, SQL_NTS);

    FREE(cppCatalog); FREE(cppSchema); FREE(cppProcedure); FREE(cppColumn);

    if (ret == SQL_ERROR)
    {
      napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
        (char *)"[node-ibm_db] Error in ODBCStatement::ProcedureColumnsSync"));
      SQLFreeStmt(m_hSTMT, SQL_CLOSE);
      return env.Null();
    }
  }

  {
    Napi::Array rows = ODBC::GetAllRecordsSync(env, m_hENV, m_hDBC, m_hSTMT, NULL, MAX_VALUE_SIZE);
    SQLFreeStmt(m_hSTMT, SQL_CLOSE);
    DEBUG_PRINTF("ODBCStatement::ProcedureColumnsSync - Exit\n");
    return rows;
  }

exit:
  FREE(cppCatalog); FREE(cppSchema); FREE(cppProcedure); FREE(cppColumn);
  if (errmsg) Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
  return env.Null();
}

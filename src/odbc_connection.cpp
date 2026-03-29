/*
  Copyright (c) 2013, Dan VerWeire <dverweire@gmail.com>
  Copyright (c) 2010, Lee Smith<notwink@gmail.com>

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

Napi::FunctionReference* ODBCConnection::constructor = nullptr;

Napi::Object ODBCConnection::Init(Napi::Env env, Napi::Object exports)
{
  DEBUG_PRINTF("ODBCConnection::Init\n");

  Napi::Function func = DefineClass(env, "ODBCConnection", {
    InstanceMethod("open", &ODBCConnection::Open, NAPI_METHOD_ATTR),
    InstanceMethod("openSync", &ODBCConnection::OpenSync, NAPI_METHOD_ATTR),
    InstanceMethod("close", &ODBCConnection::Close, NAPI_METHOD_ATTR),
    InstanceMethod("closeSync", &ODBCConnection::CloseSync, NAPI_METHOD_ATTR),
    InstanceMethod("createDbSync", &ODBCConnection::CreateDbSync, NAPI_METHOD_ATTR),
    InstanceMethod("dropDbSync", &ODBCConnection::DropDbSync, NAPI_METHOD_ATTR),
    InstanceMethod("createStatement", &ODBCConnection::CreateStatement, NAPI_METHOD_ATTR),
    InstanceMethod("createStatementSync", &ODBCConnection::CreateStatementSync, NAPI_METHOD_ATTR),
    InstanceMethod("query", &ODBCConnection::Query, NAPI_METHOD_ATTR),
    InstanceMethod("querySync", &ODBCConnection::QuerySync, NAPI_METHOD_ATTR),
    InstanceMethod("beginTransaction", &ODBCConnection::BeginTransaction, NAPI_METHOD_ATTR),
    InstanceMethod("beginTransactionSync", &ODBCConnection::BeginTransactionSync, NAPI_METHOD_ATTR),
    InstanceMethod("endTransaction", &ODBCConnection::EndTransaction, NAPI_METHOD_ATTR),
    InstanceMethod("endTransactionSync", &ODBCConnection::EndTransactionSync, NAPI_METHOD_ATTR),
    InstanceMethod("setIsolationLevel", &ODBCConnection::SetIsolationLevel, NAPI_METHOD_ATTR),
    InstanceMethod("getInfo", &ODBCConnection::GetInfo, NAPI_METHOD_ATTR),
    InstanceMethod("getInfoSync", &ODBCConnection::GetInfoSync, NAPI_METHOD_ATTR),
    InstanceMethod("getTypeInfo", &ODBCConnection::GetTypeInfo, NAPI_METHOD_ATTR),
    InstanceMethod("getTypeInfoSync", &ODBCConnection::GetTypeInfoSync, NAPI_METHOD_ATTR),
    InstanceMethod("getFunctions", &ODBCConnection::GetFunctions, NAPI_METHOD_ATTR),
    InstanceMethod("getFunctionsSync", &ODBCConnection::GetFunctionsSync, NAPI_METHOD_ATTR),
    InstanceMethod("setAttr", &ODBCConnection::SetAttr, NAPI_METHOD_ATTR),
    InstanceMethod("setAttrSync", &ODBCConnection::SetAttrSync, NAPI_METHOD_ATTR),
    InstanceMethod("columns", &ODBCConnection::Columns, NAPI_METHOD_ATTR),
    InstanceMethod("tables", &ODBCConnection::Tables, NAPI_METHOD_ATTR),
    InstanceAccessor("connected", &ODBCConnection::ConnectedGetter, nullptr, napi_enumerable),
    InstanceAccessor("connectTimeout", &ODBCConnection::ConnectTimeoutGetter, &ODBCConnection::ConnectTimeoutSetter, napi_enumerable),
    InstanceAccessor("systemNaming", &ODBCConnection::SystemNamingGetter, &ODBCConnection::SystemNamingSetter, napi_enumerable),
  });

  constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  constructor->SuppressDestruct();
  exports.Set("ODBCConnection", func);
  return exports;
}

ODBCConnection::~ODBCConnection()
{
  DEBUG_PRINTF("ODBCConnection::~ODBCConnection\n");
  this->Free();
}

void ODBCConnection::Free()
{
  DEBUG_PRINTF("ODBCConnection::Free m_hDBC = %i \n", m_hDBC);
  if (m_hDBC)
  {
    if (!g_shuttingDown) {
      SQLDisconnect(m_hDBC);
      SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
    }
    m_hDBC = (SQLHDBC)NULL;
  }
}

ODBCConnection::ODBCConnection(const Napi::CallbackInfo &info) : Napi::ObjectWrap<ODBCConnection>(info)
{
  Napi::Env env = info.Env();
  DEBUG_PRINTF("ODBCConnection::New - Entry\n");

  if (info.Length() < 2 || !info[0].IsExternal() || !info[1].IsExternal())
  {
    Napi::TypeError::New(env, "ODBCConnection::New requires 2 external arguments").ThrowAsJavaScriptException();
    return;
  }

  m_hENV = (SQLHENV)((intptr_t)info[0].As<Napi::External<void>>().Data());
  m_hDBC = (SQLHDBC)((intptr_t)info[1].As<Napi::External<void>>().Data());
  connectTimeout = DEFAULT_CONNECTION_TIMEOUT;
  systemNaming = false;
  connected = false;
  statements = 0;
  canHaveMoreResults = 0;

  DEBUG_PRINTF("ODBCConnection::New - Exit\n");
}

Napi::Value ODBCConnection::ConnectedGetter(const Napi::CallbackInfo &info)
{
  return Napi::Boolean::New(info.Env(), connected);
}

Napi::Value ODBCConnection::ConnectTimeoutGetter(const Napi::CallbackInfo &info)
{
  return Napi::Number::New(info.Env(), connectTimeout);
}

void ODBCConnection::ConnectTimeoutSetter(const Napi::CallbackInfo &info, const Napi::Value &value)
{
  if (value.IsNumber()) connectTimeout = value.As<Napi::Number>().Int32Value();
}

Napi::Value ODBCConnection::SystemNamingGetter(const Napi::CallbackInfo &info)
{
  return Napi::Boolean::New(info.Env(), systemNaming);
}

void ODBCConnection::SystemNamingSetter(const Napi::CallbackInfo &info, const Napi::Value &value)
{
  if (value.IsBoolean()) systemNaming = value.As<Napi::Boolean>().Value();
}

/*
 * Open
 */
Napi::Value ODBCConnection::Open(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCConnection::Open - Entry\n");
  Napi::Env env = info.Env();
  int len = 0;
  const char *errmsg = NULL;

  REQ_STRO_ARG(0, connection);
  REQ_FUN_ARG(1, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  open_connection_work_data *data = (open_connection_work_data *)calloc(1, sizeof(open_connection_work_data));
  MEMCHECK2(data, errmsg);

  len = (int)connection.length();
  data->connectionLength = len + 1;
  GETCPPSTR2(data->connection, connection, len, errmsg);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Open, (uv_after_work_cb)UV_AfterOpen);
  this->Ref();

exit:
  DEBUG_PRINTF("ODBCConnection::Open - Exit\n");
  if (errmsg)
  {
    free(work_req);
    free(data);
    Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  return env.Undefined();
}

void ODBCConnection::UV_Open(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_Open - Entry\n");
  open_connection_work_data *data = (open_connection_work_data *)(req->data);
  ODBCConnection *self = data->conn->self();
  SetConnectionAttributes(self);

  int ret = SQLDriverConnect(self->m_hDBC, NULL, (SQLTCHAR *)data->connection,
                             data->connectionLength, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

  if (SQL_SUCCEEDED(ret))
  {
    SQLHSTMT hStmt;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, self->m_hDBC, &hStmt);
    ret = SQLGetFunctions(self->m_hDBC, SQL_API_SQLMORERESULTS, &self->canHaveMoreResults);
    if (!SQL_SUCCEEDED(ret)) self->canHaveMoreResults = 0;
    ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  }
  data->result = ret;
  DEBUG_PRINTF("ODBCConnection::UV_Open - Exit\n");
}

void ODBCConnection::UV_AfterOpen(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterOpen - Entry\n");
  open_connection_work_data *data = (open_connection_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (data->result)
  {
    Napi::Value objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, data->conn->self()->m_hDBC);
    data->conn->Unref();
    data->cb->Call({objError});
  }
  else
  {
    data->conn->self()->connected = true;
    data->conn->Unref();
    data->cb->Call({env.Null()});
  }
  PropagateCallbackException(env);

  delete data->cb;
  FREE(data->connection);
  FREE(data);
  FREE(req);
  DEBUG_PRINTF("ODBCConnection::UV_AfterOpen - Exit\n");
}

void ODBCConnection::SetConnectionAttributes(ODBCConnection *conn)
{
  SQLUINTEGER timeOut = conn->connectTimeout;
  if (timeOut > 32767) timeOut = DEFAULT_CONNECTION_TIMEOUT;
  if (timeOut > 0)
  {
    SQLSetConnectAttr(conn->m_hDBC, SQL_ATTR_LOGIN_TIMEOUT,
                      (SQLPOINTER)(intptr_t)timeOut, sizeof(timeOut));
  }
#if !defined(__MVS__)
  if (conn->systemNaming)
  {
    SQLSetConnectAttr(conn->m_hDBC, SQL_ATTR_DBC_SYS_NAMING,
                      (SQLPOINTER)SQL_TRUE, SQL_IS_INTEGER);
  }
#endif
}

/*
 * OpenSync
 */
Napi::Value ODBCConnection::OpenSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCConnection::OpenSync - Entry\n");
  Napi::Env env = info.Env();

  REQ_STRO_ARG(0, connection);
  int connectionLength = (int)connection.length();

  if (connectionLength <= 0)
  {
    Napi::TypeError::New(env, "Connection String must be a non-empty string").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  void *connectionString = NULL;
  GETCPPSTR(connectionString, connection, connectionLength);

  SetConnectionAttributes(this);

  SQLRETURN ret = SQLDriverConnect(m_hDBC, NULL, (SQLTCHAR *)connectionString,
                                   connectionLength + 1, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

  if (!SQL_SUCCEEDED(ret))
  {
    Napi::Value objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, self()->m_hDBC);
    FREE(connectionString);
    Napi::Error::New(env, objError.ToString()).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  SQLHSTMT hStmt;
  SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, &hStmt);
  ret = SQLGetFunctions(m_hDBC, SQL_API_SQLMORERESULTS, &canHaveMoreResults);
  if (!SQL_SUCCEEDED(ret)) canHaveMoreResults = 0;
  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  self()->connected = true;

  FREE(connectionString);
  DEBUG_PRINTF("ODBCConnection::OpenSync - Exit\n");
  return Napi::Boolean::New(env, true);
}

/*
 * Close
 */
Napi::Value ODBCConnection::Close(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCConnection::Close\n");
  Napi::Env env = info.Env();
  REQ_FUN_ARG(0, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  close_connection_work_data *data = (close_connection_work_data *)(calloc(1, sizeof(close_connection_work_data)));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Close, (uv_after_work_cb)UV_AfterClose);
  this->Ref();
  return env.Undefined();
}

void ODBCConnection::UV_Close(uv_work_t *req)
{
  close_connection_work_data *data = (close_connection_work_data *)(req->data);
  data->conn->Free();
  data->conn->connected = false;
  data->result = 0;
}

void ODBCConnection::UV_AfterClose(uv_work_t *req, int status)
{
  close_connection_work_data *data = (close_connection_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (data->result)
    data->cb->Call({Napi::Error::New(env, "Error closing database").Value()});
  else
    data->cb->Call({env.Null()});
  PropagateCallbackException(env);

  data->conn->Unref();
  delete data->cb;
  free(data);
  free(req);
}

/*
 * CloseSync
 */
Napi::Value ODBCConnection::CloseSync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  this->Free();
  this->connected = false;
  return Napi::Boolean::New(env, true);
}

/*
 * CreateDbSync
 */
Napi::Value ODBCConnection::CreateDbSync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
#ifdef __MVS__
  return Napi::Boolean::New(env, false);
#else
  const char *errmsg = NULL;
  int dbNameLength = 0, codeSetLength = 0, modeLength = 0;
  void *databaseNameString = NULL, *codeSetString = NULL, *modeString = NULL;
  SQLRETURN ret = SQL_SUCCESS;
  bool err = false;
  Napi::Value objError;

  REQ_ARGS(3);
  REQ_STRO_ARG(0, dbName);
  dbNameLength = (int)dbName.length();
  if (dbNameLength <= 0)
  {
    Napi::TypeError::New(env, "Database name must be a String.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  REQ_STRO_OR_NULL_ARG(1, codeSet);
  codeSetLength = (int)codeSet.length();
  REQ_STRO_OR_NULL_ARG(2, mode);
  modeLength = (int)mode.length();

  GETCPPSTR2(databaseNameString, dbName, dbNameLength, errmsg);
  if (!info[1].IsNull()) { GETCPPSTR2(codeSetString, codeSet, codeSetLength, errmsg); }
  if (!info[2].IsNull()) { GETCPPSTR2(modeString, mode, modeLength, errmsg); }

  ret = SQLCreateDb(m_hDBC, (SQLTCHAR *)databaseNameString, dbNameLength + 1,
                              (SQLTCHAR *)codeSetString, codeSetLength + 1,
                              (SQLTCHAR *)modeString, modeLength + 1);
  if (!SQL_SUCCEEDED(ret)) { err = true; objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, self()->m_hDBC); }
  else
  {
    ret = SQLDisconnect(m_hDBC);
    if (!SQL_SUCCEEDED(ret)) { err = true; objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, self()->m_hDBC); }
    ret = SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
    if (!SQL_SUCCEEDED(ret) && !err) { err = true; objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, self()->m_hDBC); }
  }

exit:
  FREE(databaseNameString); FREE(codeSetString); FREE(modeString);
  if (errmsg) { Napi::Error::New(env, errmsg).ThrowAsJavaScriptException(); return env.Undefined(); }
  if (err) { Napi::Error::New(env, objError.ToString()).ThrowAsJavaScriptException(); return env.Undefined(); }
  return Napi::Boolean::New(env, true);
#endif
}

/*
 * DropDbSync
 */
Napi::Value ODBCConnection::DropDbSync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
#ifdef __MVS__
  return Napi::Boolean::New(env, false);
#else
  REQ_STRO_ARG(0, dbName);
  int dbNameLength = (int)dbName.length();
  if (dbNameLength <= 0) { Napi::TypeError::New(env, "Database name must be a string.").ThrowAsJavaScriptException(); return env.Undefined(); }
  void *databaseNameString = NULL;
  GETCPPSTR(databaseNameString, dbName, dbNameLength);

  SQLRETURN ret = SQLDropDb(m_hDBC, (SQLTCHAR *)databaseNameString, dbNameLength + 1);
  bool err = false; Napi::Value objError;
  if (!SQL_SUCCEEDED(ret)) { err = true; objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, self()->m_hDBC); }
  else
  {
    ret = SQLDisconnect(m_hDBC);
    if (!SQL_SUCCEEDED(ret)) { err = true; objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, self()->m_hDBC); }
    ret = SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
    if (!SQL_SUCCEEDED(ret) && !err) { err = true; objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, self()->m_hDBC); }
  }
  FREE(databaseNameString);
  if (err) { Napi::Error::New(env, objError.ToString()).ThrowAsJavaScriptException(); return env.Undefined(); }
  return Napi::Boolean::New(env, true);
#endif
}

/*
 * CreateStatementSync
 */
Napi::Value ODBCConnection::CreateStatementSync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  SQLHSTMT hSTMT;
  SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, &hSTMT);

  Napi::Value params[3] = {
    Napi::External<void>::New(env, (void *)(intptr_t)m_hENV),
    Napi::External<void>::New(env, (void *)(intptr_t)m_hDBC),
    Napi::External<void>::New(env, (void *)(intptr_t)hSTMT)
  };
  return ODBCStatement::constructor->New({params[0], params[1], params[2]});
}

/*
 * CreateStatement
 */
Napi::Value ODBCConnection::CreateStatement(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_FUN_ARG(0, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  create_statement_work_data *data = (create_statement_work_data *)(calloc(1, sizeof(create_statement_work_data)));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_CreateStatement, (uv_after_work_cb)UV_AfterCreateStatement);
  this->Ref();
  return env.Undefined();
}

void ODBCConnection::UV_CreateStatement(uv_work_t *req)
{
  create_statement_work_data *data = (create_statement_work_data *)(req->data);
  SQLAllocHandle(SQL_HANDLE_STMT, data->conn->m_hDBC, &data->hSTMT);
}

void ODBCConnection::UV_AfterCreateStatement(uv_work_t *req, int status)
{
  create_statement_work_data *data = (create_statement_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  Napi::Value args[3] = {
    Napi::External<void>::New(env, (void *)(intptr_t)data->conn->m_hENV),
    Napi::External<void>::New(env, (void *)(intptr_t)data->conn->m_hDBC),
    Napi::External<void>::New(env, (void *)(intptr_t)data->hSTMT)
  };
  Napi::Object js_result = ODBCStatement::constructor->New({args[0], args[1], args[2]});
  data->cb->Call({env.Null(), js_result});
  PropagateCallbackException(env);

  data->conn->Unref();
  delete data->cb;
  free(data);
  free(req);
}

/*
 * Query
 */
Napi::Value ODBCConnection::Query(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCConnection::Query - Entry\n");
  Napi::Env env = info.Env();
  Napi::Function cb;
  int len = 0;
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;

  query_work_data *data = (query_work_data *)calloc(1, sizeof(query_work_data));
  MEMCHECK2(data, errmsg);
  data->sql = NULL;
  data->params = NULL;
  data->paramCount = 0;
  data->noResultObject = false;
  data->arraySize = 0;

  if (info.Length() == 3)
  {
    if (!info[0].IsString()) { free(data); Napi::TypeError::New(env, "Argument 0 must be a String.").ThrowAsJavaScriptException(); return env.Undefined(); }
    if (!info[1].IsArray()) { free(data); Napi::TypeError::New(env, "Argument 1 must be an Array.").ThrowAsJavaScriptException(); return env.Undefined(); }
    if (!info[2].IsFunction()) { free(data); Napi::TypeError::New(env, "Argument 2 must be a Function.").ThrowAsJavaScriptException(); return env.Undefined(); }

    std::string sql = info[0].As<Napi::String>().Utf8Value();
    len = (int)sql.length();
    GETCPPSTR2(data->sql, sql, len, errmsg);
    data->params = ODBC::GetParametersFromArray(env, info[1].As<Napi::Array>(), &data->paramCount);
    cb = info[2].As<Napi::Function>();
  }
  else if (info.Length() == 2)
  {
    if (!info[1].IsFunction()) { free(data); Napi::TypeError::New(env, "Argument 1 must be a Function.").ThrowAsJavaScriptException(); return env.Undefined(); }
    cb = info[1].As<Napi::Function>();

    if (info[0].IsString())
    {
      std::string sql2 = info[0].As<Napi::String>().Utf8Value();
      len = (int)sql2.length();
      GETCPPSTR2(data->sql, sql2, len, errmsg);
      data->paramCount = 0;
    }
    else if (info[0].IsObject())
    {
      Napi::Object obj = info[0].As<Napi::Object>();

      if (obj.Has("sql") && obj.Get("sql").IsString())
      {
        std::string sql3 = obj.Get("sql").As<Napi::String>().Utf8Value();
        len = (int)sql3.length();
        GETCPPSTR2(data->sql, sql3, len, errmsg);
      }
      if (obj.Has("params") && obj.Get("params").IsArray())
      {
        data->params = ODBC::GetParametersFromArray(env, obj.Get("params").As<Napi::Array>(), &data->paramCount);
      }
      if (obj.Has("noResults") && obj.Get("noResults").IsBoolean())
      {
        data->noResultObject = obj.Get("noResults").As<Napi::Boolean>().Value();
      }
      if (obj.Has("ArraySize") && obj.Get("ArraySize").IsNumber())
      {
        data->arraySize = obj.Get("ArraySize").As<Napi::Number>().Int32Value();
      }
    }
    else
    {
      free(data);
      Napi::TypeError::New(env, "Argument 0 must be a String or an Object.").ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }
  else
  {
    free(data);
    Napi::TypeError::New(env, "Requires either 2 or 3 Arguments.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (len <= 0) { free(data); Napi::TypeError::New(env, "SQL statement is missing.").ThrowAsJavaScriptException(); return env.Undefined(); }

  data->sqlLen = len;
  data->sqlSize = (len + 1) * sizeof(UNICHAR);

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK2(work_req, errmsg);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Query, (uv_after_work_cb)UV_AfterQuery);
  this->Ref();

exit:
  if (errmsg)
  {
    if (data) { free(data->sql); free(data); }
    free(work_req);
    Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  return env.Undefined();
}

void ODBCConnection::UV_Query(uv_work_t *req)
{
  query_work_data *data = (query_work_data *)(req->data);
  SQLRETURN ret;

  SQLAllocHandle(SQL_HANDLE_STMT, data->conn->m_hDBC, &data->hSTMT);

  if (!data->paramCount)
  {
    ret = SQLExecDirect(data->hSTMT, (SQLTCHAR *)data->sql, data->sqlLen);
  }
  else
  {
    ret = SQLPrepare(data->hSTMT, (SQLTCHAR *)data->sql, data->sqlLen);
    if (SQL_SUCCEEDED(ret))
    {
      if (data->arraySize > 0)
      {
        SQLSetStmtAttr(data->hSTMT, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0);
        SQLSetStmtAttr(data->hSTMT, SQL_ATTR_PARAMSET_SIZE,
                       reinterpret_cast<SQLPOINTER>(static_cast<intptr_t>(data->arraySize)), 0);
      }
      ret = ODBC::BindParameters(data->hSTMT, data->params, data->paramCount);
      if (SQL_SUCCEEDED(ret))
      {
        ret = SQLExecute(data->hSTMT);
        if (ret == SQL_NEED_DATA)
          ret = ODBC::PutDataLoop(data->hSTMT, data->params, data->paramCount);
      }
    }
  }
  data->result = ret;
}

void ODBCConnection::UV_AfterQuery(uv_work_t *req, int status)
{
  query_work_data *data = (query_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  Napi::Array sp_result = Napi::Array::New(env);
  int outParamCount = 0;

  if (SQL_SUCCEEDED(data->result))
  {
    for (int i = 0; i < data->paramCount; i++)
    {
      if (data->params[i].paramtype % 2 == 0)
      {
        sp_result.Set(outParamCount, ODBC::GetOutputParameter(env, &data->params[i]));
        outParamCount++;
      }
    }
  }

  if (data->result == SQL_NO_DATA_FOUND) data->result = SQL_SUCCESS;

  if ((data->result >= SQL_SUCCESS) && (data->noResultObject))
  {
    SQLFreeHandle(SQL_HANDLE_STMT, data->hSTMT);
    data->hSTMT = (SQLHSTMT)NULL;

    if (outParamCount)
      data->cb->Call({env.Null(), sp_result});
    else
      data->cb->Call({env.Null(), env.Null()});
  }
  else
  {
    if (data->result != SQL_SUCCESS)
    {
      Napi::Value err = ODBC::GetSQLError(env, SQL_HANDLE_STMT, data->hSTMT, (char *)"[ibm_db] SQL_ERROR");
      SQLFreeHandle(SQL_HANDLE_STMT, data->hSTMT);
      data->hSTMT = (SQLHSTMT)NULL;
      data->cb->Call({err, env.Null(), outParamCount ? (Napi::Value)sp_result : env.Null()});
    }
    else
    {
      bool *canFreeHandle = new bool(true);
      Napi::Value js_args[4] = {
        Napi::External<void>::New(env, (void *)(intptr_t)data->conn->m_hENV),
        Napi::External<void>::New(env, (void *)(intptr_t)data->conn->m_hDBC),
        Napi::External<void>::New(env, (void *)(intptr_t)data->hSTMT),
        Napi::External<void>::New(env, (void *)canFreeHandle)
      };
      Napi::Object js_result = ODBCResult::constructor->New({js_args[0], js_args[1], js_args[2], js_args[3]});
      data->cb->Call({env.Null(), js_result, outParamCount ? (Napi::Value)sp_result : env.Null()});
    }
  }
  PropagateCallbackException(env);

  data->conn->Unref();
  delete data->cb;
  if (data->paramCount) { FREE_PARAMS(data->params, data->paramCount); }
  FREE(data->sql); FREE(data->catalog); FREE(data->schema);
  FREE(data->table); FREE(data->type); FREE(data->column);
  FREE(data); FREE(req);
}

/*
 * QuerySync
 */
Napi::Value ODBCConnection::QuerySync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCConnection::QuerySync - Entry\n");
  Napi::Env env = info.Env();
  void *sql = NULL;
  Parameter *params = NULL;
  SQLRETURN ret;
  SQLHSTMT hSTMT;
  int paramCount = 0, outParamCount = 0;
  Napi::Array sp_result = Napi::Array::New(env);
  bool noResultObject = false;
  int32_t arraySize = 0;
  int len = 0;

  if (info.Length() == 2)
  {
    if (!info[0].IsString()) { Napi::TypeError::New(env, "Argument 0 must be a String.").ThrowAsJavaScriptException(); return env.Undefined(); }
    if (!info[1].IsArray()) { Napi::TypeError::New(env, "Argument 1 must be an Array.").ThrowAsJavaScriptException(); return env.Undefined(); }
    std::string sql1 = info[0].As<Napi::String>().Utf8Value();
    len = (int)sql1.length();
    GETCPPSTR(sql, sql1, len);
    params = ODBC::GetParametersFromArray(env, info[1].As<Napi::Array>(), &paramCount);
  }
  else if (info.Length() == 1)
  {
    if (info[0].IsString())
    {
      std::string sql2 = info[0].As<Napi::String>().Utf8Value();
      len = (int)sql2.length();
      GETCPPSTR(sql, sql2, len);
    }
    else if (info[0].IsObject())
    {
      Napi::Object obj = info[0].As<Napi::Object>();
      if (obj.Has("sql") && obj.Get("sql").IsString()) {
        std::string sql3 = obj.Get("sql").As<Napi::String>().Utf8Value();
        len = (int)sql3.length();
        GETCPPSTR(sql, sql3, len);
      }
      if (obj.Has("params") && obj.Get("params").IsArray())
        params = ODBC::GetParametersFromArray(env, obj.Get("params").As<Napi::Array>(), &paramCount);
      if (obj.Has("noResults") && obj.Get("noResults").IsBoolean())
        noResultObject = obj.Get("noResults").As<Napi::Boolean>().Value();
      if (obj.Has("ArraySize") && obj.Get("ArraySize").IsNumber())
        arraySize = obj.Get("ArraySize").As<Napi::Number>().Int32Value();
    }
    else
    {
      Napi::TypeError::New(env, "Argument 0 must be a String or an Object.").ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }
  else
  {
    Napi::TypeError::New(env, "Requires 1 or 2 Arguments.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (len <= 0) { Napi::TypeError::New(env, "SQL statement is missing.").ThrowAsJavaScriptException(); return env.Undefined(); }

  ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, &hSTMT);
  if (!SQL_SUCCEEDED(ret)) { /* fall through */ }
  else if (!paramCount)
  {
    ret = SQLExecDirect(hSTMT, (SQLTCHAR *)sql, len);
  }
  else
  {
    ret = SQLPrepare(hSTMT, (SQLTCHAR *)sql, len);
    if (SQL_SUCCEEDED(ret))
    {
      if (arraySize > 0) {
        SQLSetStmtAttr(hSTMT, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0);
        SQLSetStmtAttr(hSTMT, SQL_ATTR_PARAMSET_SIZE, reinterpret_cast<SQLPOINTER>(static_cast<intptr_t>(arraySize)), 0);
      }
      ret = ODBC::BindParameters(hSTMT, params, paramCount);
      if (SQL_SUCCEEDED(ret))
      {
        ret = SQLExecute(hSTMT);
        if (ret == SQL_NEED_DATA) ret = ODBC::PutDataLoop(hSTMT, params, paramCount);
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
      }
    }
  }

  FREE(sql);
  FREE_PARAMS(params, paramCount);

  if (ret == SQL_NO_DATA_FOUND) ret = SQL_SUCCESS;

  if (ret != SQL_SUCCESS)
  {
    Napi::Value err = ODBC::GetSQLError(env, SQL_HANDLE_STMT, hSTMT, (char *)"[node-ibm_db] Error in ODBCConnection::QuerySync while executing query.");
    SQLFreeHandle(SQL_HANDLE_STMT, hSTMT);
    napi_throw(env, err);
    return env.Undefined();
  }
  else if (noResultObject)
  {
    SQLFreeHandle(SQL_HANDLE_STMT, hSTMT);
    if (outParamCount) {
      Napi::Array resultset = Napi::Array::New(env);
      resultset.Set((uint32_t)0, env.Null());
      resultset.Set((uint32_t)1, sp_result);
      return resultset;
    }
    return env.Null();
  }
  else
  {
    bool *canFreeHandle = new bool(true);
    Napi::Value result[4] = {
      Napi::External<void>::New(env, (void *)(intptr_t)m_hENV),
      Napi::External<void>::New(env, (void *)(intptr_t)m_hDBC),
      Napi::External<void>::New(env, (void *)(intptr_t)hSTMT),
      Napi::External<void>::New(env, (void *)canFreeHandle)
    };
    Napi::Object js_result = ODBCResult::constructor->New({result[0], result[1], result[2], result[3]});

    if (outParamCount) {
      Napi::Array resultset = Napi::Array::New(env);
      resultset.Set((uint32_t)0, js_result);
      resultset.Set((uint32_t)1, sp_result);
      return resultset;
    }
    return js_result;
  }
}

/*
 * Tables
 */
Napi::Value ODBCConnection::Tables(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  int len = 0;
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;
  query_work_data *data = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, table);
  REQ_STRO_OR_NULL_ARG(3, type);
  REQ_FUN_ARG(4, cb);

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK2(work_req, errmsg);
  data = (query_work_data *)calloc(1, sizeof(query_work_data));
  MEMCHECK2(data, errmsg);

  data->sql = NULL; data->catalog = NULL; data->schema = NULL;
  data->table = NULL; data->type = NULL; data->column = NULL;

  len = (int)catalog.length(); GETCPPSTR2(data->catalog, catalog, len, errmsg);
  len = (int)schema.length(); GETCPPSTR2(data->schema, schema, len, errmsg);
  len = (int)table.length(); GETCPPSTR2(data->table, table, len, errmsg);
  len = (int)type.length(); GETCPPSTR2(data->type, type, len, errmsg);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Tables, (uv_after_work_cb)UV_AfterQuery);
  this->Ref();

exit:
  if (errmsg)
  {
    if (data) { FREE(data->catalog); FREE(data->schema); FREE(data->table); FREE(data->type); free(data); }
    free(work_req);
    Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  return env.Undefined();
}

void ODBCConnection::UV_Tables(uv_work_t *req)
{
  query_work_data *data = (query_work_data *)(req->data);
  SQLAllocHandle(SQL_HANDLE_STMT, data->conn->m_hDBC, &data->hSTMT);

  SQLRETURN ret = SQLTables(data->hSTMT,
#ifdef __MVS__
    NULL, 0,
#else
    (SQLTCHAR *)data->catalog, SQL_NTS,
#endif
    (SQLTCHAR *)data->schema, SQL_NTS,
    (SQLTCHAR *)data->table, SQL_NTS,
    (SQLTCHAR *)data->type, SQL_NTS);

  data->result = ret;
  FREE(data->catalog); FREE(data->schema); FREE(data->table); FREE(data->type);
}

/*
 * Columns
 */
Napi::Value ODBCConnection::Columns(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  int len = 0;
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;
  query_work_data *data = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, table);
  REQ_STRO_OR_NULL_ARG(3, column);
  REQ_FUN_ARG(4, cb);

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  data = (query_work_data *)calloc(1, sizeof(query_work_data));
  MEMCHECK2(data, errmsg);

  data->sql = NULL; data->catalog = NULL; data->schema = NULL;
  data->table = NULL; data->type = NULL; data->column = NULL;

  len = (int)catalog.length(); GETCPPSTR2(data->catalog, catalog, len, errmsg);
  len = (int)schema.length(); GETCPPSTR2(data->schema, schema, len, errmsg);
  len = (int)table.length(); GETCPPSTR2(data->table, table, len, errmsg);
  len = (int)column.length(); GETCPPSTR2(data->column, column, len, errmsg);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Columns, (uv_after_work_cb)UV_AfterQuery);
  this->Ref();

exit:
  if (errmsg)
  {
    if (data) { FREE(data->catalog); FREE(data->schema); FREE(data->table); FREE(data->column); free(data); }
    free(work_req);
    Napi::Error::New(env, errmsg).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  return env.Undefined();
}

void ODBCConnection::UV_Columns(uv_work_t *req)
{
  query_work_data *data = (query_work_data *)(req->data);
  SQLAllocHandle(SQL_HANDLE_STMT, data->conn->m_hDBC, &data->hSTMT);

  SQLRETURN ret = SQLColumns(data->hSTMT,
#ifdef __MVS__
    NULL, 0,
#else
    (SQLTCHAR *)data->catalog, SQL_NTS,
#endif
    (SQLTCHAR *)data->schema, SQL_NTS,
    (SQLTCHAR *)data->table, SQL_NTS,
    (SQLTCHAR *)data->column, SQL_NTS);

  data->result = ret;
  FREE(data->catalog); FREE(data->schema); FREE(data->table); FREE(data->column);
}

/*
 * BeginTransactionSync
 */
Napi::Value ODBCConnection::BeginTransactionSync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  SQLRETURN ret = SQLSetConnectAttr(m_hDBC, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_NTS);
  if (!SQL_SUCCEEDED(ret))
  {
    Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_DBC, m_hDBC).ToString()).ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);
  }
  return Napi::Boolean::New(env, true);
}

/*
 * BeginTransaction
 */
Napi::Value ODBCConnection::BeginTransaction(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_FUN_ARG(0, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  query_work_data *data = (query_work_data *)calloc(1, sizeof(query_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_BeginTransaction, (uv_after_work_cb)UV_AfterBeginTransaction);
  return env.Undefined();
}

void ODBCConnection::UV_BeginTransaction(uv_work_t *req)
{
  query_work_data *data = (query_work_data *)(req->data);
  data->result = SQLSetConnectAttr(data->conn->self()->m_hDBC, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_NTS);
}

void ODBCConnection::UV_AfterBeginTransaction(uv_work_t *req, int status)
{
  open_connection_work_data *data = (open_connection_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (!SQL_SUCCEEDED(data->result))
    data->cb->Call({ODBC::GetSQLError(env, SQL_HANDLE_DBC, data->conn->self()->m_hDBC)});
  else
    data->cb->Call({env.Null()});
  PropagateCallbackException(env);

  delete data->cb;
  free(data);
  free(req);
}

/*
 * EndTransactionSync
 */
Napi::Value ODBCConnection::EndTransactionSync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_BOOL_ARG(0, rollback);

  SQLSMALLINT completionType = rollback ? SQL_ROLLBACK : SQL_COMMIT;
  bool error = false;
  Napi::Value objError;

  SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_hDBC, completionType);
  if (!SQL_SUCCEEDED(ret)) { error = true; objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, m_hDBC); }

  ret = SQLSetConnectAttr(m_hDBC, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_NTS);
  if (!SQL_SUCCEEDED(ret) && !error) { error = true; objError = ODBC::GetSQLError(env, SQL_HANDLE_DBC, m_hDBC); }

  if (error)
  {
    Napi::Error::New(env, objError.ToString()).ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);
  }
  return Napi::Boolean::New(env, true);
}

/*
 * EndTransaction
 */
Napi::Value ODBCConnection::EndTransaction(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_BOOL_ARG(0, rollback);
  REQ_FUN_ARG(1, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  query_work_data *data = (query_work_data *)calloc(1, sizeof(query_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->completionType = rollback ? SQL_ROLLBACK : SQL_COMMIT;
  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_EndTransaction, (uv_after_work_cb)UV_AfterEndTransaction);
  return env.Undefined();
}

void ODBCConnection::UV_EndTransaction(uv_work_t *req)
{
  query_work_data *data = (query_work_data *)(req->data);
  SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, data->conn->m_hDBC, data->completionType);
  data->result = ret;
  ret = SQLSetConnectAttr(data->conn->m_hDBC, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_NTS);
  if (!SQL_SUCCEEDED(ret) && SQL_SUCCEEDED(data->result)) data->result = ret;
}

void ODBCConnection::UV_AfterEndTransaction(uv_work_t *req, int status)
{
  open_connection_work_data *data = (open_connection_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (!SQL_SUCCEEDED(data->result))
    data->cb->Call({ODBC::GetSQLError(env, SQL_HANDLE_DBC, data->conn->self()->m_hDBC)});
  else
    data->cb->Call({env.Null()});
  PropagateCallbackException(env);

  delete data->cb;
  free(data);
  free(req);
}

/*
 * SetIsolationLevel
 */
Napi::Value ODBCConnection::SetIsolationLevel(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  SQLUINTEGER isolationLevel = SQL_TXN_READ_COMMITTED;

  if (info.Length() > 0 && info[0].IsNumber())
    isolationLevel = info[0].As<Napi::Number>().Uint32Value();
  else if (info.Length() > 0)
  {
    Napi::TypeError::New(env, "Argument #0 must be an integer.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  SQLRETURN ret = SQLSetConnectAttr(m_hDBC, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)(intptr_t)isolationLevel, SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
  {
    Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_DBC, m_hDBC).ToString()).ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);
  }
  return Napi::Boolean::New(env, true);
}

/*
 * GetInfoSync
 */
Napi::Value ODBCConnection::GetInfoSync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_ARGS(2);
  REQ_INT_ARG(0, infotype);
  REQ_INT_ARG(1, infolen);

  SQLPOINTER rgbInfo = malloc(infolen);
  MEMCHECK(rgbInfo);
  SQLSMALLINT cbInfo = 0;
  SQLRETURN ret = SQLGetInfo(m_hDBC, infotype, rgbInfo, infolen, &cbInfo);

  if (!SQL_SUCCEEDED(ret))
  {
    free(rgbInfo);
    Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_DBC, m_hDBC).ToString()).ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);
  }

  Napi::Value value = getInfoValue(env, (SQLUSMALLINT)infotype, rgbInfo);
  free(rgbInfo);
  return value;
}

/*
 * GetInfo
 */
Napi::Value ODBCConnection::GetInfo(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_ARGS(3);
  REQ_INT_ARG(0, infotype);
  REQ_INT_ARG(1, infolen);
  REQ_FUN_ARG(2, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  getinfo_work_data *data = (getinfo_work_data *)calloc(1, sizeof(getinfo_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->infoType = infotype;
  data->buffLen = infolen;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_GetInfo, (uv_after_work_cb)UV_AfterGetInfo);
  this->Ref();
  return env.Undefined();
}

void ODBCConnection::UV_GetInfo(uv_work_t *req)
{
  getinfo_work_data *data = (getinfo_work_data *)(req->data);
  SQLSMALLINT valueLen = 0;
  data->buffer = malloc(data->buffLen);
  if (!data->buffer) { data->rc = SQL_ERROR; return; }
  data->rc = SQLGetInfo(data->conn->m_hDBC, data->infoType, data->buffer, data->buffLen, &valueLen);
  data->valueLen = valueLen;
}

void ODBCConnection::UV_AfterGetInfo(uv_work_t *req, int status)
{
  getinfo_work_data *data = (getinfo_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (data->rc != SQL_SUCCESS)
    data->cb->Call({ODBC::GetSQLError(env, SQL_HANDLE_DBC, data->conn->m_hDBC, (char *)"[node-ibm_db] SQL_ERROR"), env.Null()});
  else
    data->cb->Call({env.Null(), getInfoValue(env, data->infoType, data->buffer)});
  PropagateCallbackException(env);

  data->conn->Unref();
  delete data->cb;
  free(data->buffer);
  free(data);
  free(req);
}

/*
 * GetTypeInfoSync
 */
Napi::Value ODBCConnection::GetTypeInfoSync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_ARGS(1);
  REQ_INT_ARG(0, dataType);

  SQLHSTMT hSTMT;
  SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, &hSTMT);
  SQLRETURN ret = SQLGetTypeInfo(hSTMT, (SQLSMALLINT)dataType);

  if (!SQL_SUCCEEDED(ret))
  {
    Napi::Value err = ODBC::GetSQLError(env, SQL_HANDLE_STMT, hSTMT);
    SQLFreeHandle(SQL_HANDLE_STMT, hSTMT);
    Napi::Error::New(env, err.ToString()).ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);
  }

  bool *canFreeHandle = new bool(true);
  Napi::Value args[4] = {
    Napi::External<void>::New(env, (void *)(intptr_t)m_hENV),
    Napi::External<void>::New(env, (void *)(intptr_t)m_hDBC),
    Napi::External<void>::New(env, (void *)(intptr_t)hSTMT),
    Napi::External<void>::New(env, (void *)canFreeHandle)
  };
  return ODBCResult::constructor->New({args[0], args[1], args[2], args[3]});
}

Napi::Value ODBCConnection::GetTypeInfo(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_ARGS(2);
  REQ_INT_ARG(0, dataType);
  REQ_FUN_ARG(1, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  gettypeinfo_work_data *data = (gettypeinfo_work_data *)calloc(1, sizeof(gettypeinfo_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->dataType = (SQLSMALLINT)dataType;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_GetTypeInfo, (uv_after_work_cb)UV_AfterGetTypeInfo);
  this->Ref();
  return env.Undefined();
}

void ODBCConnection::UV_GetTypeInfo(uv_work_t *req)
{
  gettypeinfo_work_data *data = (gettypeinfo_work_data *)(req->data);
  SQLAllocHandle(SQL_HANDLE_STMT, data->conn->m_hDBC, &data->hSTMT);
  data->result = SQLGetTypeInfo(data->hSTMT, data->dataType);
}

void ODBCConnection::UV_AfterGetTypeInfo(uv_work_t *req, int status)
{
  gettypeinfo_work_data *data = (gettypeinfo_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (!SQL_SUCCEEDED(data->result))
  {
    data->cb->Call({ODBC::GetSQLError(env, SQL_HANDLE_STMT, data->hSTMT), env.Null()});
    SQLFreeHandle(SQL_HANDLE_STMT, data->hSTMT);
  }
  else
  {
    bool *canFreeHandle = new bool(true);
    Napi::Value args[4] = {
      Napi::External<void>::New(env, (void *)(intptr_t)data->conn->m_hENV),
      Napi::External<void>::New(env, (void *)(intptr_t)data->conn->m_hDBC),
      Napi::External<void>::New(env, (void *)(intptr_t)data->hSTMT),
      Napi::External<void>::New(env, (void *)canFreeHandle)
    };
    Napi::Object js_result = ODBCResult::constructor->New({args[0], args[1], args[2], args[3]});
    data->cb->Call({env.Null(), js_result});
  }
  PropagateCallbackException(env);

  data->conn->Unref();
  delete data->cb;
  free(data);
  free(req);
}

/*
 * GetFunctionsSync
 */
Napi::Value ODBCConnection::GetFunctionsSync(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_ARGS(1);
  REQ_UINT_ARG(0, functionId);

  SQLUSMALLINT funcId = (SQLUSMALLINT)functionId;
  SQLUSMALLINT supportedPtr = 0;
  SQLUSMALLINT supportedArr[100] = {0};
  SQLRETURN ret;

  if (funcId)
    ret = SQLGetFunctions(m_hDBC, funcId, &supportedPtr);
  else
    ret = SQLGetFunctions(m_hDBC, funcId, supportedArr);

  if (!SQL_SUCCEEDED(ret))
  {
    Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_DBC, m_hDBC).ToString()).ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);
  }

  if (funcId)
    return Napi::Number::New(env, supportedPtr);

  Napi::Array value = Napi::Array::New(env, 100);
  for (uint32_t i = 0; i < 100; i++)
    value.Set(i, Napi::Number::New(env, supportedArr[i]));
  return value;
}

Napi::Value ODBCConnection::GetFunctions(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  REQ_ARGS(2);
  REQ_UINT_ARG(0, functionId);
  REQ_FUN_ARG(1, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  getfunctions_work_data *data = (getfunctions_work_data *)calloc(1, sizeof(getfunctions_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->functionId = (SQLUSMALLINT)functionId;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_GetFunctions, (uv_after_work_cb)UV_AfterGetFunctions);
  this->Ref();
  return env.Undefined();
}

void ODBCConnection::UV_GetFunctions(uv_work_t *req)
{
  getfunctions_work_data *data = (getfunctions_work_data *)(req->data);

  if (data->functionId)
    data->result = SQLGetFunctions(data->conn->m_hDBC, data->functionId, &data->supportedPtr);
  else
    data->result = SQLGetFunctions(data->conn->m_hDBC, data->functionId, data->supportedArr);
}

void ODBCConnection::UV_AfterGetFunctions(uv_work_t *req, int status)
{
  getfunctions_work_data *data = (getfunctions_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (!SQL_SUCCEEDED(data->result))
    data->cb->Call({ODBC::GetSQLError(env, SQL_HANDLE_DBC, data->conn->m_hDBC), env.Null()});
  else
  {
    if (data->functionId)
      data->cb->Call({env.Null(), Napi::Number::New(env, data->supportedPtr)});
    else
    {
      Napi::Array value = Napi::Array::New(env, 100);
      for (uint32_t i = 0; i < 100; i++)
        value.Set(i, Napi::Number::New(env, data->supportedArr[i]));
      data->cb->Call({env.Null(), value});
    }
  }
  PropagateCallbackException(env);

  data->conn->Unref();
  delete data->cb;
  free(data);
  free(req);
}

/*
 * SetAttrSync
 */
Napi::Value ODBCConnection::SetAttrSync(const Napi::CallbackInfo &info)
{
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
    Napi::TypeError::New(env, "Unsupported Connection Attribute Value.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  SQLRETURN ret = SQLSetConnectAttr(m_hDBC, attr, valuePtr, stringLength);
  if (stringLength != SQL_IS_INTEGER)
    FREE(valuePtr);

  if (!SQL_SUCCEEDED(ret))
  {
    napi_throw(env, ODBC::GetSQLError(env, SQL_HANDLE_DBC, m_hDBC, (char *)"[node-ibm_db] Error in ODBCConnection::SetAttrSync"));
    return Napi::Boolean::New(env, false);
  }
  return Napi::Boolean::New(env, true);
}

Napi::Value ODBCConnection::SetAttr(const Napi::CallbackInfo &info)
{
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
    Napi::TypeError::New(env, "Unsupported Connection Attribute Value.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  setattr_work_data *data = (setattr_work_data *)calloc(1, sizeof(setattr_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->conn = this;
  data->attr = attr;
  data->valuePtr = valuePtr;
  data->stringLength = stringLength;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_SetAttr, (uv_after_work_cb)UV_AfterSetAttr);
  this->Ref();
  return env.Undefined();
}

void ODBCConnection::UV_SetAttr(uv_work_t *req)
{
  setattr_work_data *data = (setattr_work_data *)(req->data);
  data->result = SQLSetConnectAttr(data->conn->m_hDBC, data->attr, data->valuePtr, data->stringLength);
}

void ODBCConnection::UV_AfterSetAttr(uv_work_t *req, int status)
{
  setattr_work_data *data = (setattr_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (!SQL_SUCCEEDED(data->result))
    data->cb->Call({ODBC::GetSQLError(env, SQL_HANDLE_DBC, data->conn->m_hDBC)});
  else
    data->cb->Call({env.Null(), Napi::Boolean::New(env, true)});
  PropagateCallbackException(env);

  if (data->stringLength != SQL_IS_INTEGER)
    FREE(data->valuePtr);

  data->conn->Unref();
  delete data->cb;
  free(data);
  free(req);
}

/*
 * getInfoValue helper
 */
Napi::Value getInfoValue(Napi::Env env, SQLUSMALLINT fInfoType, SQLPOINTER info)
{
  if (info == NULL) return env.Null();

  switch ((unsigned int)fInfoType)
  {
  case SQL_DATA_SOURCE_NAME: case SQL_DRIVER_NAME: case SQL_DRIVER_VER:
#if !defined(__MVS__)
  case SQL_DRIVER_BLDLEVEL: case SQL_DB2_DRIVER_VER:
#endif
  case SQL_ODBC_VER: case SQL_ROW_UPDATES: case SQL_SERVER_NAME:
  case SQL_SEARCH_PATTERN_ESCAPE: case SQL_DATABASE_NAME: case SQL_DBMS_NAME:
  case SQL_XOPEN_CLI_YEAR: case SQL_DBMS_VER: case SQL_DBMS_FUNCTIONLVL:
  case SQL_ACCESSIBLE_TABLES: case SQL_ACCESSIBLE_PROCEDURES:
  case SQL_DATA_SOURCE_READ_ONLY: case SQL_EXPRESSIONS_IN_ORDERBY:
  case SQL_IDENTIFIER_QUOTE_CHAR: case SQL_MULT_RESULT_SETS:
  case SQL_MULTIPLE_ACTIVE_TXN: case SQL_OUTER_JOINS: case SQL_SCHEMA_TERM:
  case SQL_PROCEDURE_TERM: case SQL_TABLE_TERM: case SQL_USER_NAME:
  case SQL_PROCEDURES: case SQL_INTEGRITY: case SQL_DRIVER_ODBC_VER:
  case SQL_COLUMN_ALIAS: case SQL_KEYWORDS: case SQL_ORDER_BY_COLUMNS_IN_SELECT:
  case SQL_SPECIAL_CHARACTERS: case SQL_MAX_ROW_SIZE_INCLUDES_LONG:
  case SQL_NEED_LONG_DATA_LEN: case SQL_LIKE_ESCAPE_CLAUSE:
  case SQL_CATALOG_NAME: case SQL_CATALOG_NAME_SEPARATOR: case SQL_CATALOG_TERM:
  case SQL_COLLATION_SEQ: case SQL_DESCRIBE_PARAMETER:
    return Napi::String::New(env, (char *)info);

  case SQL_MAX_DRIVER_CONNECTIONS: case SQL_ACTIVE_ENVIRONMENTS:
  case SQL_MAX_CONCURRENT_ACTIVITIES: case SQL_ODBC_API_CONFORMANCE:
  case SQL_ODBC_SAG_CLI_CONFORMANCE: case SQL_ODBC_SQL_CONFORMANCE:
  case SQL_CONCAT_NULL_BEHAVIOR: case SQL_CURSOR_COMMIT_BEHAVIOR:
  case SQL_CURSOR_ROLLBACK_BEHAVIOR: case SQL_IDENTIFIER_CASE:
  case SQL_MAX_COLUMN_NAME_LEN: case SQL_MAX_CURSOR_NAME_LEN:
  case SQL_MAX_SCHEMA_NAME_LEN:
#if !defined(__MVS__)
  case SQL_MAX_MODULE_NAME_LEN:
#endif
  case SQL_MAX_PROCEDURE_NAME_LEN: case SQL_MAX_TABLE_NAME_LEN:
  case SQL_TXN_CAPABLE: case SQL_CORRELATION_NAME: case SQL_NON_NULLABLE_COLUMNS:
  case SQL_FILE_USAGE: case SQL_NULL_COLLATION: case SQL_GROUP_BY:
  case SQL_QUOTED_IDENTIFIER_CASE: case SQL_MAX_COLUMNS_IN_GROUP_BY:
  case SQL_MAX_COLUMNS_IN_INDEX: case SQL_MAX_COLUMNS_IN_ORDER_BY:
  case SQL_MAX_COLUMNS_IN_SELECT: case SQL_MAX_COLUMNS_IN_TABLE:
  case SQL_MAX_TABLES_IN_SELECT: case SQL_MAX_USER_NAME_LEN:
  case SQL_CATALOG_LOCATION: case SQL_MAX_CATALOG_NAME_LEN:
  case SQL_MAX_IDENTIFIER_LEN:
    return Napi::Number::New(env, *((SQLSMALLINT *)info));

  case SQL_DRIVER_HDBC: case SQL_DRIVER_HENV: case SQL_DRIVER_HSTMT:
  case SQL_DRIVER_HDESC: case SQL_DRIVER_HLIB: case SQL_MAX_ROW_SIZE:
  case SQL_ASYNC_MODE: case SQL_MAX_STATEMENT_LEN: case SQL_MAX_CHAR_LITERAL_LEN:
  case SQL_MAX_INDEX_SIZE: case SQL_MAX_BINARY_LITERAL_LEN:
  case SQL_CURSOR_SENSITIVITY: case SQL_DEFAULT_TXN_ISOLATION:
  case SQL_MAX_ASYNC_CONCURRENT_STATEMENTS: case SQL_ODBC_INTERFACE_CONFORMANCE:
  case SQL_PARAM_ARRAY_ROW_COUNTS: case SQL_PARAM_ARRAY_SELECTS:
  case SQL_DTC_TRANSITION_COST:
#if !defined(__MVS__)
  case SQL_DATABASE_CODEPAGE: case SQL_APPLICATION_CODEPAGE:
  case SQL_CONNECT_CODEPAGE: case SQL_DB2_DRIVER_TYPE:
#endif
    return Napi::Number::New(env, *((SQLINTEGER *)info));

#if !defined(__MVS__)
  case SQL_INPUT_CHAR_CONVFACTOR: case SQL_OUTPUT_CHAR_CONVFACTOR:
    return Napi::Number::New(env, *((double *)info));
#endif

  default:
    return Napi::Number::New(env, *((SQLINTEGER *)info));
  }
}

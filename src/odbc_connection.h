/*
  Copyright (c) 2013, Dan VerWeire<dverweire@gmail.com>
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

#ifndef _SRC_ODBC_CONNECTION_H
#define _SRC_ODBC_CONNECTION_H

#include <napi.h>

#define DEFAULT_CONNECTION_TIMEOUT 30

#ifdef UNICODE
#define SQLCreateDb SQLCreateDbW
#define SQLDropDb SQLDropDbW
#endif

#define CLI_INTERNAL_ATTRIBUTES 2569

#ifndef SQL_DBMS_FUNCTIONLVL
#define SQL_DBMS_FUNCTIONLVL 203
#endif

class ODBCConnection : public Napi::ObjectWrap<ODBCConnection>
{
public:
  static Napi::FunctionReference constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  ODBCConnection(const Napi::CallbackInfo &info);
  ~ODBCConnection();
  void Free();

  // Instance methods
  Napi::Value Open(const Napi::CallbackInfo &info);
  Napi::Value OpenSync(const Napi::CallbackInfo &info);
  Napi::Value Close(const Napi::CallbackInfo &info);
  Napi::Value CloseSync(const Napi::CallbackInfo &info);
  Napi::Value CreateDbSync(const Napi::CallbackInfo &info);
  Napi::Value DropDbSync(const Napi::CallbackInfo &info);
  Napi::Value CreateStatement(const Napi::CallbackInfo &info);
  Napi::Value CreateStatementSync(const Napi::CallbackInfo &info);
  Napi::Value Query(const Napi::CallbackInfo &info);
  Napi::Value QuerySync(const Napi::CallbackInfo &info);
  Napi::Value BeginTransaction(const Napi::CallbackInfo &info);
  Napi::Value BeginTransactionSync(const Napi::CallbackInfo &info);
  Napi::Value EndTransaction(const Napi::CallbackInfo &info);
  Napi::Value EndTransactionSync(const Napi::CallbackInfo &info);
  Napi::Value SetIsolationLevel(const Napi::CallbackInfo &info);
  Napi::Value GetInfo(const Napi::CallbackInfo &info);
  Napi::Value GetInfoSync(const Napi::CallbackInfo &info);
  Napi::Value GetTypeInfo(const Napi::CallbackInfo &info);
  Napi::Value GetTypeInfoSync(const Napi::CallbackInfo &info);
  Napi::Value GetFunctions(const Napi::CallbackInfo &info);
  Napi::Value GetFunctionsSync(const Napi::CallbackInfo &info);
  Napi::Value SetAttr(const Napi::CallbackInfo &info);
  Napi::Value SetAttrSync(const Napi::CallbackInfo &info);
  Napi::Value Columns(const Napi::CallbackInfo &info);
  Napi::Value Tables(const Napi::CallbackInfo &info);

  // Property getter/setters
  Napi::Value ConnectedGetter(const Napi::CallbackInfo &info);
  Napi::Value ConnectTimeoutGetter(const Napi::CallbackInfo &info);
  void ConnectTimeoutSetter(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value SystemNamingGetter(const Napi::CallbackInfo &info);
  void SystemNamingSetter(const Napi::CallbackInfo &info, const Napi::Value &value);

  // UV async callbacks
  static void UV_Open(uv_work_t *work_req);
  static void UV_AfterOpen(uv_work_t *work_req, int status);
  static void SetConnectionAttributes(ODBCConnection *conn);
  static void UV_Close(uv_work_t *work_req);
  static void UV_AfterClose(uv_work_t *work_req, int status);
  static void UV_CreateStatement(uv_work_t *work_req);
  static void UV_AfterCreateStatement(uv_work_t *work_req, int status);
  static void UV_Query(uv_work_t *req);
  static void UV_AfterQuery(uv_work_t *req, int status);
  static void UV_Tables(uv_work_t *req);
  static void UV_Columns(uv_work_t *req);
  static void UV_BeginTransaction(uv_work_t *work_req);
  static void UV_AfterBeginTransaction(uv_work_t *work_req, int status);
  static void UV_EndTransaction(uv_work_t *work_req);
  static void UV_AfterEndTransaction(uv_work_t *work_req, int status);
  static void UV_GetInfo(uv_work_t *work_req);
  static void UV_AfterGetInfo(uv_work_t *work_req, int status);
  static void UV_GetTypeInfo(uv_work_t *work_req);
  static void UV_AfterGetTypeInfo(uv_work_t *work_req, int status);
  static void UV_GetFunctions(uv_work_t *work_req);
  static void UV_AfterGetFunctions(uv_work_t *work_req, int status);
  static void UV_SetAttr(uv_work_t *work_req);
  static void UV_AfterSetAttr(uv_work_t *work_req, int status);

  ODBCConnection *self(void) { return this; }

  SQLHENV m_hENV;
  SQLHDBC m_hDBC;
  SQLUSMALLINT canHaveMoreResults;
  bool systemNaming;
  bool connected;
  int statements;
  SQLUINTEGER connectTimeout;
};

struct create_statement_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCConnection *conn;
  SQLHSTMT hSTMT;
  int result;
};

struct query_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCConnection *conn;
  SQLHSTMT hSTMT;

  Parameter *params;
  int paramCount;
  int completionType;
  bool noResultObject;
  int32_t arraySize;

  void *sql;
  void *catalog;
  void *schema;
  void *table;
  void *type;
  void *column;

  int sqlLen;
  int sqlSize;

  int result;
};

struct open_connection_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCConnection *conn;
  int result;
  void *connection;
  int connectionLength;
};

struct close_connection_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCConnection *conn;
  int result;
};

struct getinfo_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCConnection *conn;
  SQLUSMALLINT infoType;
  SQLSMALLINT buffLen;
  SQLSMALLINT valueLen;
  void *buffer;
  SQLRETURN rc;
};

struct gettypeinfo_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCConnection *conn;
  SQLSMALLINT dataType;
  SQLHSTMT hSTMT;
  int result;
};

struct getfunctions_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCConnection *conn;
  SQLUSMALLINT functionId;
  SQLUSMALLINT supportedPtr;
  SQLUSMALLINT supportedArr[100];
  SQLRETURN result;
};

struct setattr_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCConnection *conn;
  SQLINTEGER attr;
  SQLPOINTER valuePtr;
  SQLINTEGER stringLength;
  SQLRETURN result;
};

Napi::Value getInfoValue(Napi::Env env, SQLUSMALLINT fInfoType, SQLPOINTER info);

#endif

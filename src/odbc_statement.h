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

#ifndef _SRC_ODBC_STATEMENT_H
#define _SRC_ODBC_STATEMENT_H

#include <napi.h>

class ODBCStatement : public Napi::ObjectWrap<ODBCStatement>
{
public:
  static Napi::FunctionReference* constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  ODBCStatement(const Napi::CallbackInfo &info);
  ~ODBCStatement();
  void Free();

  // Instance methods
  Napi::Value Execute(const Napi::CallbackInfo &info);
  Napi::Value ExecuteSync(const Napi::CallbackInfo &info);
  Napi::Value ExecuteDirect(const Napi::CallbackInfo &info);
  Napi::Value ExecuteDirectSync(const Napi::CallbackInfo &info);
  Napi::Value ExecuteNonQuery(const Napi::CallbackInfo &info);
  Napi::Value ExecuteNonQuerySync(const Napi::CallbackInfo &info);
  Napi::Value Prepare(const Napi::CallbackInfo &info);
  Napi::Value PrepareSync(const Napi::CallbackInfo &info);
  Napi::Value Bind(const Napi::CallbackInfo &info);
  Napi::Value BindSync(const Napi::CallbackInfo &info);
  Napi::Value SetAttr(const Napi::CallbackInfo &info);
  Napi::Value SetAttrSync(const Napi::CallbackInfo &info);
  Napi::Value CloseSync(const Napi::CallbackInfo &info);
  Napi::Value Close(const Napi::CallbackInfo &info);

  // UV callbacks
  static void UV_Execute(uv_work_t *work_req);
  static void UV_AfterExecute(uv_work_t *work_req, int status);
  static void UV_ExecuteDirect(uv_work_t *work_req);
  static void UV_AfterExecuteDirect(uv_work_t *work_req, int status);
  static void UV_ExecuteNonQuery(uv_work_t *work_req);
  static void UV_AfterExecuteNonQuery(uv_work_t *work_req, int status);
  static void UV_Prepare(uv_work_t *work_req);
  static void UV_AfterPrepare(uv_work_t *work_req, int status);
  static void UV_Bind(uv_work_t *work_req);
  static void UV_AfterBind(uv_work_t *work_req, int status);
  static void UV_SetAttr(uv_work_t *work_req);
  static void UV_AfterSetAttr(uv_work_t *work_req, int status);
  static void UV_Close(uv_work_t *work_req);
  static void UV_AfterClose(uv_work_t *work_req, int status);

  ODBCStatement *self(void) { return this; }

  SQLHENV m_hENV;
  SQLHDBC m_hDBC;
  SQLHSTMT m_hSTMT;

  Parameter *params;
  int paramCount;

  uint16_t *buffer;
  size_t bufferLength;
  Column *columns;
  short colCount;
};

struct execute_direct_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCStatement *stmt;
  int result;
  void *sql;
  size_t sqlLen;
};

struct execute_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCStatement *stmt;
  int result;
};

struct prepare_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCStatement *stmt;
  int result;
  void *sql;
  size_t sqlLen;
};

struct bind_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCStatement *stmt;
  int result;
};

struct stmt_setattr_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCStatement *stmt;
  SQLINTEGER attr;
  SQLPOINTER valuePtr;
  SQLINTEGER stringLength;
  int result;
};

struct stmt_close_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBCStatement *stmt;
  SQLUSMALLINT closeOption;
  int result;
};

#endif

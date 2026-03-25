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

#ifndef _SRC_ODBC_RESULT_H
#define _SRC_ODBC_RESULT_H

#include <napi.h>
#include <sqlext.h>

typedef struct {
  SQLPOINTER   data;
  SQLLEN      *indicators;
  SQLSMALLINT  cType;
  SQLLEN       elementSize;
} BoundColumn;

#define FILE_COL_MAX_PATH 1024
typedef struct {
  bool          bound;
  char          fileName[FILE_COL_MAX_PATH];
  SQLSMALLINT   fileNameLength;
  SQLUINTEGER   fileOption;
  SQLINTEGER    stringLength;
  SQLINTEGER    indicator;
} FileColumnBinding;

class ODBCResult : public Napi::ObjectWrap<ODBCResult>
{
public:
  static Napi::FunctionReference constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  ODBCResult(const Napi::CallbackInfo &info);
  ~ODBCResult();
  void Free();

  // Instance methods
  Napi::Value Fetch(const Napi::CallbackInfo &info);
  Napi::Value FetchSync(const Napi::CallbackInfo &info);
  Napi::Value FetchAll(const Napi::CallbackInfo &info);
  Napi::Value FetchAllSync(const Napi::CallbackInfo &info);
  Napi::Value FetchN(const Napi::CallbackInfo &info);
  Napi::Value FetchNSync(const Napi::CallbackInfo &info);
  Napi::Value GetData(const Napi::CallbackInfo &info);
  Napi::Value GetDataSync(const Napi::CallbackInfo &info);
  Napi::Value Close(const Napi::CallbackInfo &info);
  Napi::Value CloseSync(const Napi::CallbackInfo &info);
  Napi::Value MoreResultsSync(const Napi::CallbackInfo &info);
  Napi::Value GetColumnNamesSync(const Napi::CallbackInfo &info);
  Napi::Value GetColumnMetadataSync(const Napi::CallbackInfo &info);
  Napi::Value GetSQLErrorSync(const Napi::CallbackInfo &info);
  Napi::Value GetAffectedRowsSync(const Napi::CallbackInfo &info);
  Napi::Value BindFileToColSync(const Napi::CallbackInfo &info);

  // Property getter/setter
  Napi::Value FetchModeGetter(const Napi::CallbackInfo &info);
  void FetchModeSetter(const Napi::CallbackInfo &info, const Napi::Value &value);

  // UV callbacks
  static void UV_Fetch(uv_work_t *work_req);
  static void UV_AfterFetch(uv_work_t *work_req, int status);
  static void UV_FetchAll(uv_work_t *work_req);
  static void UV_AfterFetchAll(uv_work_t *work_req, int status);
  static void UV_FetchN(uv_work_t *work_req);
  static void UV_AfterFetchN(uv_work_t *work_req, int status);
  static void UV_GetData(uv_work_t *work_req);
  static void UV_AfterGetData(uv_work_t *work_req, int status);
  static void UV_Close(uv_work_t *work_req);
  static void UV_AfterClose(uv_work_t *work_req, int status);

  void OverrideFileColumns(Napi::Env env, Napi::Value row, int fetchMode);

  struct fetch_work_data
  {
    napi_env env;
    Napi::FunctionReference *cb;
    ODBCResult *objResult;
    SQLRETURN result;
    uint16_t fetchMode;
    int count;
    int maxCount;
    int errorCount;
    Napi::Reference<Napi::Array> *rows;
    Napi::Reference<Napi::Object> *objError;
    bool useBlockFetch;
    SQLULEN blockRowIndex;
  };

  struct getdata_work_data
  {
    napi_env env;
    Napi::FunctionReference *cb;
    ODBCResult *objResult;
    SQLUINTEGER colNum;
    SQLUINTEGER dataSize;
  };

  struct close_result_work_data
  {
    napi_env env;
    Napi::FunctionReference *cb;
    ODBCResult *objResult;
    SQLUSMALLINT closeOption;
    int result;
  };

  ODBCResult *self(void) { return this; }

  // Block fetch support
  bool InitBlockFetch();
  void FreeBlockFetchBuffers();
  bool BindColumnsForBlockFetch();
  Napi::Value GetBoundColumnValue(Napi::Env env, int colIndex, SQLULEN rowIndex);
  SQLRETURN BlockFetchNextRow(SQLULEN *outRowIndex);

  SQLHENV m_hENV;
  SQLHDBC m_hDBC;
  SQLHSTMT m_hSTMT;
  bool m_canFreeHandle;
  uint16_t m_fetchMode;

  uint16_t *buffer;
  size_t bufferLength;
  Column *columns;
  short colCount;

  SQLULEN      m_rowArraySize;
  SQLULEN      m_rowsFetched;
  SQLULEN      m_currentRowInBlock;
  SQLUSMALLINT *m_rowStatusArray;
  BoundColumn  *m_boundCols;
  bool         m_blockFetchInitialized;
  bool         m_blockExhausted;

  FileColumnBinding *m_fileColBindings;
  int          m_fileColCount;
};

#endif

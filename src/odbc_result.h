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

#include <nan.h>

class ODBCResult : public Nan::ObjectWrap
{
public:
  static Nan::Persistent<String> OPTION_FETCH_MODE;
  static Nan::Persistent<Function> constructor;
  static NAN_MODULE_INIT(Init);

  void Free();

protected:
  ODBCResult() {};

  explicit ODBCResult(SQLHENV hENV, SQLHDBC hDBC, SQLHSTMT hSTMT, bool canFreeHandle) : Nan::ObjectWrap(),
                                                                                        m_hENV(hENV),
                                                                                        m_hDBC(hDBC),
                                                                                        m_hSTMT(hSTMT),
                                                                                        m_canFreeHandle(canFreeHandle) {};

  ~ODBCResult();

  // constructor
  static NAN_METHOD(New);

  // async methods
  static NAN_METHOD(Fetch);
  static void UV_Fetch(uv_work_t *work_req);
  static void UV_AfterFetch(uv_work_t *work_req, int status);

  static NAN_METHOD(FetchAll);
  static void UV_FetchAll(uv_work_t *work_req);
  static void UV_AfterFetchAll(uv_work_t *work_req, int status);

  static NAN_METHOD(GetData);
  static void UV_GetData(uv_work_t *work_req);
  static void UV_AfterGetData(uv_work_t *work_req, int status);

  static NAN_METHOD(Close);
  static void UV_Close(uv_work_t *work_req);
  static void UV_AfterClose(uv_work_t *work_req, int status);

  // sync methods
  static NAN_METHOD(CloseSync);
  static NAN_METHOD(MoreResultsSync);
  static NAN_METHOD(FetchSync);
  static NAN_METHOD(FetchAllSync);
  static NAN_METHOD(GetColumnNamesSync);
  static NAN_METHOD(GetColumnMetadataSync);
  static NAN_METHOD(GetSQLErrorSync);
  static NAN_METHOD(GetAffectedRowsSync);
  static NAN_METHOD(GetDataSync);

  // property getter/setters
  static NAN_GETTER(FetchModeGetter);
  static NAN_SETTER(FetchModeSetter);

  struct fetch_work_data
  {
    Nan::Callback *cb;
    ODBCResult *objResult;
    SQLRETURN result;

    uint16_t fetchMode;
    int count;
    int errorCount;
    Nan::Persistent<Array> rows;
    Nan::Persistent<Value> objError;
  };

  struct getdata_work_data
  {
    Nan::Callback *cb;
    ODBCResult *objResult;

    SQLUINTEGER colNum;
    SQLUINTEGER dataSize;
  };

  struct close_result_work_data
  {
    Nan::Callback *cb;
    ODBCResult *objResult;
    SQLUSMALLINT closeOption;
    int result;
  };

  ODBCResult *self(void) { return this; }

protected:
  SQLHENV m_hENV;
  SQLHDBC m_hDBC;
  SQLHSTMT m_hSTMT;
  bool m_canFreeHandle;
  uint16_t m_fetchMode;

  uint16_t *buffer;
  size_t bufferLength;
  Column *columns;
  short colCount;
};

#endif

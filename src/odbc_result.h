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
#include <sqlext.h>

// Block fetch (SQL_ATTR_ROW_ARRAY_SIZE > 1) column buffer
typedef struct {
  SQLPOINTER   data;         // Array buffer: N * element_size
  SQLLEN      *indicators;   // Array of N SQLLEN indicator values
  SQLSMALLINT  cType;        // C type used for SQLBindCol
  SQLLEN       elementSize;  // Size of each element in the data buffer
} BoundColumn;

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
                                                                                        m_canFreeHandle(canFreeHandle),
                                                                                        m_rowArraySize(0),
                                                                                        m_rowsFetched(0),
                                                                                        m_currentRowInBlock(0),
                                                                                        m_rowStatusArray(NULL),
                                                                                        m_boundCols(NULL),
                                                                                        m_blockFetchInitialized(false),
                                                                                        m_blockExhausted(false) {};

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

  static NAN_METHOD(FetchN);
  static void UV_FetchN(uv_work_t *work_req);
  static void UV_AfterFetchN(uv_work_t *work_req, int status);

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
  static NAN_METHOD(FetchNSync);
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
    int maxCount;
    int errorCount;
    Nan::Persistent<Array> rows;
    Nan::Persistent<Value> objError;

    // Block fetch support
    bool useBlockFetch;
    SQLULEN blockRowIndex; // Row index within the block for single-row fetch
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

  // Block fetch (SQL_ATTR_ROW_ARRAY_SIZE > 1) support
  bool InitBlockFetch();             // Detect and set up block fetch if needed
  void FreeBlockFetchBuffers();      // Free bound column buffers
  bool BindColumnsForBlockFetch();   // SQLBindCol for all columns
  Local<Value> GetBoundColumnValue(int colIndex, SQLULEN rowIndex); // Read value from bound buffer
  SQLRETURN BlockFetchNextRow(SQLULEN *outRowIndex); // Advance to next valid row in block, fetch new block if needed

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

  // Block fetch state
  SQLULEN      m_rowArraySize;        // Value of SQL_ATTR_ROW_ARRAY_SIZE (0 or 1 = normal)
  SQLULEN      m_rowsFetched;         // Actual rows fetched by last SQLFetch
  SQLULEN      m_currentRowInBlock;   // Current position within the fetched block
  SQLUSMALLINT *m_rowStatusArray;     // Per-row status from SQLFetch
  BoundColumn  *m_boundCols;          // Array of bound column buffers
  bool         m_blockFetchInitialized; // Whether block fetch setup has been done
  bool         m_blockExhausted;      // True when block fetch has reached SQL_NO_DATA
};

#endif

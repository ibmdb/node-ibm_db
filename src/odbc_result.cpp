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

Napi::FunctionReference ODBCResult::constructor;

Napi::Object ODBCResult::Init(Napi::Env env, Napi::Object exports)
{
  DEBUG_PRINTF("ODBCResult::Init\n");

  Napi::Function func = DefineClass(env, "ODBCResult", {
    InstanceMethod("fetchAll", &ODBCResult::FetchAll, NAPI_METHOD_ATTR),
    InstanceMethod("fetch", &ODBCResult::Fetch, NAPI_METHOD_ATTR),
    InstanceMethod("getData", &ODBCResult::GetData, NAPI_METHOD_ATTR),
    InstanceMethod("moreResultsSync", &ODBCResult::MoreResultsSync, NAPI_METHOD_ATTR),
    InstanceMethod("close", &ODBCResult::Close, NAPI_METHOD_ATTR),
    InstanceMethod("closeSync", &ODBCResult::CloseSync, NAPI_METHOD_ATTR),
    InstanceMethod("fetchSync", &ODBCResult::FetchSync, NAPI_METHOD_ATTR),
    InstanceMethod("fetchAllSync", &ODBCResult::FetchAllSync, NAPI_METHOD_ATTR),
    InstanceMethod("fetchN", &ODBCResult::FetchN, NAPI_METHOD_ATTR),
    InstanceMethod("fetchNSync", &ODBCResult::FetchNSync, NAPI_METHOD_ATTR),
    InstanceMethod("getDataSync", &ODBCResult::GetDataSync, NAPI_METHOD_ATTR),
    InstanceMethod("getColumnNamesSync", &ODBCResult::GetColumnNamesSync, NAPI_METHOD_ATTR),
    InstanceMethod("getColumnMetadataSync", &ODBCResult::GetColumnMetadataSync, NAPI_METHOD_ATTR),
    InstanceMethod("getSQLErrorSync", &ODBCResult::GetSQLErrorSync, NAPI_METHOD_ATTR),
    InstanceMethod("getAffectedRowsSync", &ODBCResult::GetAffectedRowsSync, NAPI_METHOD_ATTR),
    InstanceMethod("bindFileToColSync", &ODBCResult::BindFileToColSync, NAPI_METHOD_ATTR),
    InstanceAccessor("fetchMode", &ODBCResult::FetchModeGetter, &ODBCResult::FetchModeSetter, napi_enumerable),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("ODBCResult", func);
  return exports;
}

ODBCResult::~ODBCResult()
{
  DEBUG_PRINTF("ODBCResult::~ODBCResult m_hSTMT=%X m_canFreeHandle=%d\n", m_hSTMT, m_canFreeHandle);
  this->Free();
}

void ODBCResult::Free()
{
  FreeBlockFetchBuffers();

  if (m_fileColBindings) { free(m_fileColBindings); m_fileColBindings = NULL; m_fileColCount = 0; }
  if (m_hSTMT && m_canFreeHandle) { SQLFreeHandle(SQL_HANDLE_STMT, m_hSTMT); m_hSTMT = (SQLHSTMT)NULL; m_canFreeHandle = 0; }
  if (buffer != NULL) { free((uint16_t *)buffer); buffer = NULL; }
  bufferLength = 0;
}

ODBCResult::ODBCResult(const Napi::CallbackInfo &info) : Napi::ObjectWrap<ODBCResult>(info)
{
  DEBUG_PRINTF("ODBCResult::New\n");
  Napi::Env env = info.Env();

  if (info.Length() < 4 || !info[0].IsExternal() || !info[1].IsExternal() || !info[2].IsExternal() || !info[3].IsExternal())
  {
    Napi::TypeError::New(env, "ODBCResult::New requires 4 external arguments").ThrowAsJavaScriptException();
    return;
  }

  m_hENV = (SQLHENV)((intptr_t)info[0].As<Napi::External<void>>().Data());
  m_hDBC = (SQLHDBC)((intptr_t)info[1].As<Napi::External<void>>().Data());
  m_hSTMT = (SQLHSTMT)((intptr_t)info[2].As<Napi::External<void>>().Data());
  bool *canFreeHandle = static_cast<bool *>(info[3].As<Napi::External<void>>().Data());
  m_canFreeHandle = *canFreeHandle;
  delete canFreeHandle;

  bufferLength = MAX_VALUE_SIZE;
  buffer = NULL;
  colCount = 0;
  m_fetchMode = FETCH_OBJECT;

  m_rowArraySize = 0;
  m_rowsFetched = 0;
  m_currentRowInBlock = 0;
  m_rowStatusArray = NULL;
  m_boundCols = NULL;
  m_blockFetchInitialized = false;
  m_blockExhausted = false;
  m_fileColBindings = NULL;
  m_fileColCount = 0;
}

Napi::Value ODBCResult::FetchModeGetter(const Napi::CallbackInfo &info)
{
  return Napi::Number::New(info.Env(), m_fetchMode);
}

void ODBCResult::FetchModeSetter(const Napi::CallbackInfo &info, const Napi::Value &value)
{
  if (value.IsNumber()) m_fetchMode = value.As<Napi::Number>().Int32Value();
}

/*
 * Fetch
 */
Napi::Value ODBCResult::Fetch(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::Fetch\n");
  Napi::Env env = info.Env();

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  fetch_work_data *data = (fetch_work_data *)calloc(1, sizeof(fetch_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  Napi::Function cb;
  data->fetchMode = m_fetchMode;

  if (info.Length() == 1 && info[0].IsFunction())
  {
    cb = info[0].As<Napi::Function>();
  }
  else if (info.Length() == 2 && info[0].IsObject() && info[1].IsFunction())
  {
    cb = info[1].As<Napi::Function>();
    Napi::Object obj = info[0].As<Napi::Object>();
    if (obj.Has("fetchMode") && obj.Get("fetchMode").IsNumber())
      data->fetchMode = obj.Get("fetchMode").As<Napi::Number>().Int32Value();
  }
  else
  {
    free(data); free(work_req);
    Napi::TypeError::New(env, "ODBCResult::Fetch(): 1 or 2 arguments are required. The last argument must be a callback function.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->objResult = this;
  data->useBlockFetch = false;
  data->blockRowIndex = 0;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Fetch, (uv_after_work_cb)UV_AfterFetch);
  this->Ref();
  return env.Undefined();
}

void ODBCResult::UV_Fetch(uv_work_t *work_req)
{
  DEBUG_PRINTF("ODBCResult::UV_Fetch\n");
  fetch_work_data *data = (fetch_work_data *)(work_req->data);

  bool useBlockFetch = data->objResult->InitBlockFetch();
  data->useBlockFetch = useBlockFetch;

  if (useBlockFetch)
  {
    ODBCResult *self = data->objResult;
    if (self->m_blockExhausted)
    {
      data->result = SQL_NO_DATA;
    }
    else
    {
      self->m_rowsFetched = 0;
      self->m_currentRowInBlock = 0;
      data->result = SQLFetch(self->m_hSTMT);
      if (data->result == SQL_NO_DATA) self->m_blockExhausted = true;
    }
  }
  else
  {
    data->result = SQLFetch(data->objResult->m_hSTMT);
  }
}

void ODBCResult::UV_AfterFetch(uv_work_t *work_req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterFetch\n");
  fetch_work_data *data = (fetch_work_data *)(work_req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  SQLRETURN ret = data->result;
  Napi::Value objError;
  bool moreWork = true;
  bool error = false;

  if (data->objResult->colCount == 0)
    data->objResult->columns = ODBC::GetColumns(data->objResult->m_hSTMT, &data->objResult->colCount);

  if (data->objResult->colCount == 0 || data->fetchMode == FETCH_NODATA)
    moreWork = false;
  else if (ret == SQL_ERROR)
  {
    moreWork = false; error = true;
    objError = ODBC::GetSQLError(env, SQL_HANDLE_STMT, data->objResult->m_hSTMT, (char *)"Error in ODBCResult::UV_AfterFetch");
  }
  else if (ret == SQL_NO_DATA)
    moreWork = false;

  if (moreWork)
  {
    Napi::Value rowData;
    if (data->useBlockFetch)
    {
      ODBCResult *self = data->objResult->self();
      Napi::Array blockArray = Napi::Array::New(env);
      int validCount = 0;
      for (SQLULEN r = 0; r < self->m_rowsFetched; r++)
      {
        SQLUSMALLINT st = self->m_rowStatusArray[r];
        if (st == SQL_ROW_SUCCESS || st == SQL_ROW_SUCCESS_WITH_INFO)
        {
          if (data->fetchMode == FETCH_ARRAY)
          {
            Napi::Array array = Napi::Array::New(env);
            for (int c = 0; c < self->colCount; c++)
              array.Set((uint32_t)c, self->GetBoundColumnValue(env, c, r));
            blockArray.Set((uint32_t)validCount++, array);
          }
          else
          {
            Napi::Object tuple = Napi::Object::New(env);
            for (int c = 0; c < self->colCount; c++)
            {
#ifdef UNICODE
              tuple.Set(Napi::String::New(env, (const char16_t *)self->columns[c].name), self->GetBoundColumnValue(env, c, r));
#else
              tuple.Set(Napi::String::New(env, (const char *)self->columns[c].name), self->GetBoundColumnValue(env, c, r));
#endif
            }
            blockArray.Set((uint32_t)validCount++, tuple);
          }
        }
      }
      rowData = blockArray;
    }
    else if (data->fetchMode == FETCH_ARRAY)
    {
      rowData = ODBC::GetRecordArray(env, data->objResult->m_hSTMT, data->objResult->columns,
        &data->objResult->colCount, data->objResult->buffer, data->objResult->bufferLength);
    }
    else
    {
      rowData = ODBC::GetRecordTuple(env, data->objResult->m_hSTMT, data->objResult->columns,
        &data->objResult->colCount, data->objResult->buffer, data->objResult->bufferLength);
    }
    data->objResult->OverrideFileColumns(env, rowData, data->fetchMode);
    data->cb->Call({env.Null(), rowData});
  }
  else
  {
    if (data->fetchMode != FETCH_NODATA)
    {
      ODBC::FreeColumns(data->objResult->columns, &data->objResult->colCount);
    }
    FREE(data->objResult->buffer);

    if (error)
      data->cb->Call({objError, env.Null()});
    else
      data->cb->Call({env.Null(), env.Null()});
  }
  PropagateCallbackException(env);

  data->objResult->Unref();
  delete data->cb;
  free(data);
  free(work_req);
}

/*
 * FetchSync
 */
Napi::Value ODBCResult::FetchSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::FetchSync - Entry\n");
  Napi::Env env = info.Env();

  Napi::Value objError;
  bool moreWork = true;
  bool error = false;
  int fetchMode = m_fetchMode;

  if (info.Length() == 1 && info[0].IsObject())
  {
    Napi::Object obj = info[0].As<Napi::Object>();
    if (obj.Has("fetchMode") && obj.Get("fetchMode").IsNumber())
      fetchMode = obj.Get("fetchMode").As<Napi::Number>().Int32Value();
  }

  if (colCount == 0) columns = ODBC::GetColumns(m_hSTMT, &colCount);

  bool useBlockFetch = InitBlockFetch();
  SQLRETURN ret;

  if (useBlockFetch)
  {
    if (m_blockExhausted) ret = SQL_NO_DATA;
    else { m_rowsFetched = 0; m_currentRowInBlock = 0; ret = SQLFetch(m_hSTMT); }
  }
  else
    ret = SQLFetch(m_hSTMT);

  if (ret == SQL_ERROR)
  {
    moreWork = false; error = true;
    objError = ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT, (char *)"Error in ODBCResult::FetchSync");
  }
  else if (ret == SQL_NO_DATA)
  {
    if (useBlockFetch) m_blockExhausted = true;
    moreWork = false;
  }
  else if (colCount == 0)
    moreWork = false;

  if (!error && fetchMode == FETCH_NODATA) return env.Null();

  if (moreWork)
  {
    Napi::Value data;
    if (useBlockFetch)
    {
      Napi::Array blockArray = Napi::Array::New(env);
      int validCount = 0;
      for (SQLULEN r = 0; r < m_rowsFetched; r++)
      {
        SQLUSMALLINT st = m_rowStatusArray[r];
        if (st == SQL_ROW_SUCCESS || st == SQL_ROW_SUCCESS_WITH_INFO)
        {
          if (fetchMode == FETCH_ARRAY)
          {
            Napi::Array array = Napi::Array::New(env);
            for (int c = 0; c < colCount; c++) array.Set((uint32_t)c, GetBoundColumnValue(env, c, r));
            blockArray.Set((uint32_t)validCount++, array);
          }
          else
          {
            Napi::Object tuple = Napi::Object::New(env);
            for (int c = 0; c < colCount; c++)
            {
#ifdef UNICODE
              tuple.Set(Napi::String::New(env, (const char16_t *)columns[c].name), GetBoundColumnValue(env, c, r));
#else
              tuple.Set(Napi::String::New(env, (const char *)columns[c].name), GetBoundColumnValue(env, c, r));
#endif
            }
            blockArray.Set((uint32_t)validCount++, tuple);
          }
        }
      }
      data = blockArray;
    }
    else if (fetchMode == FETCH_ARRAY)
      data = ODBC::GetRecordArray(env, m_hSTMT, columns, &colCount, buffer, bufferLength);
    else
      data = ODBC::GetRecordTuple(env, m_hSTMT, columns, &colCount, buffer, bufferLength);

    OverrideFileColumns(env, data, fetchMode);
    return data;
  }
  else
  {
    ODBC::FreeColumns(columns, &colCount);
    FREE(buffer);
    if (error)
    {
      Napi::Error::New(env, objError.ToString()).ThrowAsJavaScriptException();
      return env.Null();
    }
    return env.Null();
  }
}

/*
 * FetchAll
 */
Napi::Value ODBCResult::FetchAll(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::FetchAll - Entry\n");
  Napi::Env env = info.Env();

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  fetch_work_data *data = (fetch_work_data *)calloc(1, sizeof(fetch_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  Napi::Function cb;
  data->fetchMode = m_fetchMode;

  if (info.Length() == 1 && info[0].IsFunction())
  {
    cb = info[0].As<Napi::Function>();
  }
  else if (info.Length() == 2 && info[0].IsObject() && info[1].IsFunction())
  {
    cb = info[1].As<Napi::Function>();
    Napi::Object obj = info[0].As<Napi::Object>();
    if (obj.Has("fetchMode") && obj.Get("fetchMode").IsNumber())
      data->fetchMode = obj.Get("fetchMode").As<Napi::Number>().Int32Value();
  }
  else
  {
    free(data); free(work_req);
    Napi::TypeError::New(env, "ODBCResult::FetchAll(): 1 or 2 arguments are required. The last argument must be a callback function.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  data->rows = new Napi::Reference<Napi::Array>(Napi::Persistent(Napi::Array::New(env)));
  data->errorCount = 0;
  data->count = 0;
  data->objError = new Napi::Reference<Napi::Object>(Napi::Persistent(Napi::Object::New(env)));

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->objResult = this;
  data->useBlockFetch = false;
  data->blockRowIndex = 0;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_FetchAll, (uv_after_work_cb)UV_AfterFetchAll);
  this->Ref();
  return env.Undefined();
}

void ODBCResult::UV_FetchAll(uv_work_t *work_req)
{
  DEBUG_PRINTF("ODBCResult::UV_FetchAll - Entry\n");
  fetch_work_data *data = (fetch_work_data *)(work_req->data);

  bool useBlockFetch = data->objResult->InitBlockFetch();
  data->useBlockFetch = useBlockFetch;

  if (useBlockFetch)
  {
    SQLULEN rowIdx = 0;
    data->result = data->objResult->BlockFetchNextRow(&rowIdx);
    data->blockRowIndex = rowIdx;
  }
  else
    data->result = SQLFetch(data->objResult->m_hSTMT);
}

void ODBCResult::UV_AfterFetchAll(uv_work_t *work_req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterFetchAll - Entry\n");
  fetch_work_data *data = (fetch_work_data *)(work_req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  ODBCResult *self = data->objResult->self();
  bool doMoreWork = true;

  if (data->result == SQL_ERROR)
  {
    data->errorCount++;
    data->objError->Reset(ODBC::GetSQLError(env, SQL_HANDLE_STMT, self->m_hSTMT,
      (char *)"[node-odbc] Error in ODBCResult::UV_AfterFetchAll").As<Napi::Object>());
  }

  if (self->colCount == 0) self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);

  bool noDataFetchQuery = false;
  if (self->colCount == 0) { noDataFetchQuery = true; doMoreWork = false; }
  else if (data->result == SQL_NO_DATA) doMoreWork = false;
  else if (data->result == SQL_ERROR) doMoreWork = false;
  else
  {
    Napi::Array rows = data->rows->Value();

    if (data->useBlockFetch)
    {
      SQLULEN rowIdx = data->blockRowIndex;
      if (data->fetchMode == FETCH_ARRAY)
      {
        Napi::Array array = Napi::Array::New(env);
        for (int c = 0; c < self->colCount; c++)
          array.Set((uint32_t)c, self->GetBoundColumnValue(env, c, rowIdx));
        rows.Set((uint32_t)data->count, array);
      }
      else
      {
        Napi::Object tuple = Napi::Object::New(env);
        for (int c = 0; c < self->colCount; c++)
        {
#ifdef UNICODE
          tuple.Set(Napi::String::New(env, (const char16_t *)self->columns[c].name), self->GetBoundColumnValue(env, c, rowIdx));
#else
          tuple.Set(Napi::String::New(env, (const char *)self->columns[c].name), self->GetBoundColumnValue(env, c, rowIdx));
#endif
        }
        rows.Set((uint32_t)data->count, tuple);
      }
    }
    else if (data->fetchMode == FETCH_ARRAY)
    {
      Napi::Value rowVal = ODBC::GetRecordArray(env, self->m_hSTMT, self->columns, &self->colCount, self->buffer, self->bufferLength);
      self->OverrideFileColumns(env, rowVal, data->fetchMode);
      rows.Set((uint32_t)data->count, rowVal);
    }
    else
    {
      Napi::Value rowVal = ODBC::GetRecordTuple(env, self->m_hSTMT, self->columns, &self->colCount, self->buffer, self->bufferLength);
      self->OverrideFileColumns(env, rowVal, data->fetchMode);
      rows.Set((uint32_t)data->count, rowVal);
    }
    data->count++;
  }

  if (doMoreWork)
  {
    uv_queue_work(uv_default_loop(), work_req, UV_FetchAll, (uv_after_work_cb)UV_AfterFetchAll);
  }
  else
  {
    Napi::Value err;
    if (data->errorCount > 0 && !noDataFetchQuery)
      err = data->objError->Value();
    else
      err = env.Null();

    data->cb->Call({err, data->rows->Value(), Napi::Number::New(env, self->colCount)});
    PropagateCallbackException(env);

    ODBC::FreeColumns(self->columns, &self->colCount);
    FREE(self->buffer);
    delete data->cb;
    delete data->rows;
    delete data->objError;
    free(data);
    free(work_req);
    self->Unref();
  }
}

/*
 * FetchAllSync
 */
Napi::Value ODBCResult::FetchAllSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::FetchAllSync - Entry\n");
  Napi::Env env = info.Env();

  Napi::Value objError;
  SQLRETURN ret;
  int count = 0, errorCount = 0;
  int fetchMode = m_fetchMode;

  if (info.Length() == 1 && info[0].IsObject())
  {
    Napi::Object obj = info[0].As<Napi::Object>();
    if (obj.Has("fetchMode") && obj.Get("fetchMode").IsNumber())
      fetchMode = obj.Get("fetchMode").As<Napi::Number>().Int32Value();
  }

  if (colCount == 0) columns = ODBC::GetColumns(m_hSTMT, &colCount);

  Napi::Array rows = Napi::Array::New(env);

  if (colCount > 0)
  {
    bool useBlockFetch = InitBlockFetch();

    if (useBlockFetch)
    {
      while (true)
      {
        m_rowsFetched = 0;
        ret = SQLFetch(m_hSTMT);
        if (ret == SQL_ERROR)
        {
          errorCount++;
          objError = ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT, (char *)"[node-odbc] Error in ODBCResult::FetchAllSync (block fetch)");
          break;
        }
        if (ret == SQL_NO_DATA) break;

        for (SQLULEN r = 0; r < m_rowsFetched; r++)
        {
          if (m_rowStatusArray[r] == SQL_ROW_ERROR || m_rowStatusArray[r] == SQL_ROW_NOROW) continue;
          if (fetchMode == FETCH_ARRAY)
          {
            Napi::Array array = Napi::Array::New(env);
            for (int c = 0; c < colCount; c++) array.Set((uint32_t)c, GetBoundColumnValue(env, c, r));
            rows.Set((uint32_t)count, array);
          }
          else
          {
            Napi::Object tuple = Napi::Object::New(env);
            for (int c = 0; c < colCount; c++)
            {
#ifdef UNICODE
              tuple.Set(Napi::String::New(env, (const char16_t *)columns[c].name), GetBoundColumnValue(env, c, r));
#else
              tuple.Set(Napi::String::New(env, (const char *)columns[c].name), GetBoundColumnValue(env, c, r));
#endif
            }
            rows.Set((uint32_t)count, tuple);
          }
          count++;
        }
      }
    }
    else
    {
      while (true)
      {
        ret = SQLFetch(m_hSTMT);
        if (ret == SQL_ERROR)
        {
          errorCount++;
          objError = ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT,
            (char *)"[node-odbc] Error in ODBCResult::FetchAllSync; probably your query did not have a result set.");
          break;
        }
        if (ret == SQL_NO_DATA) break;

        if (fetchMode == FETCH_ARRAY)
        {
          Napi::Value rowVal = ODBC::GetRecordArray(env, m_hSTMT, columns, &colCount, buffer, bufferLength);
          OverrideFileColumns(env, rowVal, fetchMode);
          rows.Set((uint32_t)count, rowVal);
        }
        else
        {
          Napi::Value rowVal = ODBC::GetRecordTuple(env, m_hSTMT, columns, &colCount, buffer, bufferLength);
          OverrideFileColumns(env, rowVal, fetchMode);
          rows.Set((uint32_t)count, rowVal);
        }
        count++;
      }
    }
  }

  ODBC::FreeColumns(columns, &colCount);
  FREE(buffer);

  if (errorCount > 0) Napi::Error::New(env, objError.ToString()).ThrowAsJavaScriptException();
  return rows;
}

/*
 * FetchN
 */
Napi::Value ODBCResult::FetchN(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::FetchN - Entry\n");
  Napi::Env env = info.Env();

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  fetch_work_data *data = (fetch_work_data *)calloc(1, sizeof(fetch_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  Napi::Function cb;
  data->fetchMode = m_fetchMode;

  if (info.Length() == 2 && info[0].IsNumber() && info[1].IsFunction())
  {
    data->maxCount = info[0].As<Napi::Number>().Int32Value();
    cb = info[1].As<Napi::Function>();
  }
  else if (info.Length() == 3 && info[0].IsNumber() && info[1].IsObject() && info[2].IsFunction())
  {
    data->maxCount = info[0].As<Napi::Number>().Int32Value();
    cb = info[2].As<Napi::Function>();
    Napi::Object obj = info[1].As<Napi::Object>();
    if (obj.Has("fetchMode") && obj.Get("fetchMode").IsNumber())
      data->fetchMode = obj.Get("fetchMode").As<Napi::Number>().Int32Value();
  }
  else
  {
    free(data); free(work_req);
    Napi::TypeError::New(env, "ODBCResult::FetchN(): Arguments must be (count, [option], callback).").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (data->maxCount <= 0)
  {
    free(data); free(work_req);
    Napi::RangeError::New(env, "ODBCResult::FetchN(): count must be a positive integer.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  data->rows = new Napi::Reference<Napi::Array>(Napi::Persistent(Napi::Array::New(env)));
  data->errorCount = 0;
  data->count = 0;
  data->objError = new Napi::Reference<Napi::Object>(Napi::Persistent(Napi::Object::New(env)));

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->objResult = this;
  data->useBlockFetch = false;
  data->blockRowIndex = 0;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_FetchN, (uv_after_work_cb)UV_AfterFetchN);
  this->Ref();
  return env.Undefined();
}

void ODBCResult::UV_FetchN(uv_work_t *work_req)
{
  DEBUG_PRINTF("ODBCResult::UV_FetchN\n");
  fetch_work_data *data = (fetch_work_data *)(work_req->data);

  bool useBlockFetch = data->objResult->InitBlockFetch();
  data->useBlockFetch = useBlockFetch;

  if (useBlockFetch)
  {
    SQLULEN rowIdx = 0;
    data->result = data->objResult->BlockFetchNextRow(&rowIdx);
    data->blockRowIndex = rowIdx;
  }
  else
    data->result = SQLFetch(data->objResult->m_hSTMT);
}

void ODBCResult::UV_AfterFetchN(uv_work_t *work_req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterFetchN - Entry\n");
  fetch_work_data *data = (fetch_work_data *)(work_req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  ODBCResult *self = data->objResult->self();
  bool doMoreWork = true;

  if (data->result == SQL_ERROR)
  {
    data->errorCount++;
    data->objError->Reset(ODBC::GetSQLError(env, SQL_HANDLE_STMT, self->m_hSTMT,
      (char *)"[node-odbc] Error in ODBCResult::UV_AfterFetchN").As<Napi::Object>());
  }

  if (self->colCount == 0) self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);

  if (self->colCount == 0) doMoreWork = false;
  else if (data->result == SQL_NO_DATA) doMoreWork = false;
  else if (data->result == SQL_ERROR) doMoreWork = false;
  else
  {
    Napi::Array rows = data->rows->Value();

    if (data->useBlockFetch)
    {
      SQLULEN rowIdx = data->blockRowIndex;
      if (data->fetchMode == FETCH_ARRAY)
      {
        Napi::Array array = Napi::Array::New(env);
        for (int c = 0; c < self->colCount; c++) array.Set((uint32_t)c, self->GetBoundColumnValue(env, c, rowIdx));
        rows.Set((uint32_t)data->count, array);
      }
      else
      {
        Napi::Object tuple = Napi::Object::New(env);
        for (int c = 0; c < self->colCount; c++)
        {
#ifdef UNICODE
          tuple.Set(Napi::String::New(env, (const char16_t *)self->columns[c].name), self->GetBoundColumnValue(env, c, rowIdx));
#else
          tuple.Set(Napi::String::New(env, (const char *)self->columns[c].name), self->GetBoundColumnValue(env, c, rowIdx));
#endif
        }
        rows.Set((uint32_t)data->count, tuple);
      }
    }
    else if (data->fetchMode == FETCH_ARRAY)
    {
      Napi::Value rowVal = ODBC::GetRecordArray(env, self->m_hSTMT, self->columns, &self->colCount, self->buffer, self->bufferLength);
      self->OverrideFileColumns(env, rowVal, data->fetchMode);
      rows.Set((uint32_t)data->count, rowVal);
    }
    else
    {
      Napi::Value rowVal = ODBC::GetRecordTuple(env, self->m_hSTMT, self->columns, &self->colCount, self->buffer, self->bufferLength);
      self->OverrideFileColumns(env, rowVal, data->fetchMode);
      rows.Set((uint32_t)data->count, rowVal);
    }
    data->count++;

    if (data->count >= data->maxCount) doMoreWork = false;
  }

  if (doMoreWork)
  {
    uv_queue_work(uv_default_loop(), work_req, UV_FetchN, (uv_after_work_cb)UV_AfterFetchN);
  }
  else
  {
    Napi::Value err;
    if (data->errorCount > 0 && self->colCount > 0)
      err = data->objError->Value();
    else
      err = env.Null();

    data->cb->Call({err, data->rows->Value()});
    delete data->cb;
    delete data->rows;
    delete data->objError;
    free(data);
    free(work_req);
    self->Unref();
  }
}

/*
 * FetchNSync
 */
Napi::Value ODBCResult::FetchNSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::FetchNSync - Entry\n");
  Napi::Env env = info.Env();

  if (!info[0].IsNumber())
  {
    Napi::TypeError::New(env, "ODBCResult::FetchNSync(): First argument must be an integer count.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int maxCount = info[0].As<Napi::Number>().Int32Value();
  if (maxCount <= 0)
  {
    Napi::RangeError::New(env, "ODBCResult::FetchNSync(): count must be a positive integer.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Value objError;
  SQLRETURN ret;
  int count = 0, errorCount = 0;
  int fetchMode = m_fetchMode;

  if (info.Length() >= 2 && info[1].IsObject())
  {
    Napi::Object obj = info[1].As<Napi::Object>();
    if (obj.Has("fetchMode") && obj.Get("fetchMode").IsNumber())
      fetchMode = obj.Get("fetchMode").As<Napi::Number>().Int32Value();
  }

  if (colCount == 0) columns = ODBC::GetColumns(m_hSTMT, &colCount);

  Napi::Array rows = Napi::Array::New(env);

  if (colCount > 0)
  {
    bool useBlockFetch = InitBlockFetch();

    if (useBlockFetch)
    {
      SQLULEN rowIdx;
      while (count < maxCount)
      {
        ret = BlockFetchNextRow(&rowIdx);
        if (ret == SQL_ERROR) { errorCount++; objError = ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT, (char *)"[node-odbc] Error in ODBCResult::FetchNSync (block fetch)"); break; }
        if (ret == SQL_NO_DATA) break;

        if (fetchMode == FETCH_ARRAY)
        {
          Napi::Array array = Napi::Array::New(env);
          for (int c = 0; c < colCount; c++) array.Set((uint32_t)c, GetBoundColumnValue(env, c, rowIdx));
          rows.Set((uint32_t)count, array);
        }
        else
        {
          Napi::Object tuple = Napi::Object::New(env);
          for (int c = 0; c < colCount; c++)
          {
#ifdef UNICODE
            tuple.Set(Napi::String::New(env, (const char16_t *)columns[c].name), GetBoundColumnValue(env, c, rowIdx));
#else
            tuple.Set(Napi::String::New(env, (const char *)columns[c].name), GetBoundColumnValue(env, c, rowIdx));
#endif
          }
          rows.Set((uint32_t)count, tuple);
        }
        count++;
      }
    }
    else
    {
      while (count < maxCount)
      {
        ret = SQLFetch(m_hSTMT);
        if (ret == SQL_ERROR) { errorCount++; objError = ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT, (char *)"[node-odbc] Error in ODBCResult::FetchNSync"); break; }
        if (ret == SQL_NO_DATA) break;

        if (fetchMode == FETCH_ARRAY)
        {
          Napi::Value rowVal = ODBC::GetRecordArray(env, m_hSTMT, columns, &colCount, buffer, bufferLength);
          OverrideFileColumns(env, rowVal, fetchMode);
          rows.Set((uint32_t)count, rowVal);
        }
        else
        {
          Napi::Value rowVal = ODBC::GetRecordTuple(env, m_hSTMT, columns, &colCount, buffer, bufferLength);
          OverrideFileColumns(env, rowVal, fetchMode);
          rows.Set((uint32_t)count, rowVal);
        }
        count++;
      }
    }
  }

  if (errorCount > 0) Napi::Error::New(env, objError.ToString()).ThrowAsJavaScriptException();
  return rows;
}

/*
 * GetData
 */
Napi::Value ODBCResult::GetData(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::GetData\n");
  Napi::Env env = info.Env();

  REQ_INT_ARG(0, colNum);
  REQ_INT_ARG(1, dataSize);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  getdata_work_data *data = (getdata_work_data *)calloc(1, sizeof(getdata_work_data));
  if (!data) free(work_req);
  MEMCHECK(data);

  Napi::Function cb;

  if (info.Length() == 3 && info[2].IsFunction())
  {
    cb = info[2].As<Napi::Function>();
    data->colNum = (SQLUINTEGER)colNum;
    data->dataSize = (SQLUINTEGER)dataSize;
  }
  else if (info.Length() == 2 && info[1].IsFunction())
  {
    cb = info[1].As<Napi::Function>();
    data->colNum = (SQLUINTEGER)colNum;
    data->dataSize = 0;
  }
  else
  {
    free(data); free(work_req);
    Napi::TypeError::New(env, "ODBCResult::GetData(): 2 or 3 arguments are required.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->objResult = this;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_GetData, (uv_after_work_cb)UV_AfterGetData);
  this->Ref();
  return env.Undefined();
}

void ODBCResult::UV_GetData(uv_work_t *work_req)
{
  // No-op; actual work is done in UV_AfterGetData since it accesses JS values
}

void ODBCResult::UV_AfterGetData(uv_work_t *work_req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterGetData - Entry\n");
  getdata_work_data *data = (getdata_work_data *)(work_req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  ODBCResult *objResult = data->objResult;
  Napi::Value err = env.Null();
  Napi::Value val = env.Null();

  if (objResult->colCount > 0 && data->colNum <= (unsigned)objResult->colCount)
  {
    objResult->bufferLength = (size_t)data->dataSize;
    objResult->columns[data->colNum - 1].getData = true;
    val = ODBC::GetColumnValue(env, objResult->m_hSTMT, objResult->columns[data->colNum - 1], objResult->buffer, objResult->bufferLength);
  }
  else
  {
    ODBC::FreeColumns(objResult->columns, &objResult->colCount);
    FREE(objResult->buffer);
  }

  data->cb->Call({err, val});
  PropagateCallbackException(env);

  delete data->cb;
  data->objResult->Unref();
  free(data);
  free(work_req);
}

/*
 * GetDataSync
 */
Napi::Value ODBCResult::GetDataSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::GetDataSync - Entry\n");
  Napi::Env env = info.Env();

  REQ_INT_ARG(0, colNum);
  REQ_INT_ARG(1, dataSize);

  if (colCount > 0 && colNum <= colCount)
  {
    bufferLength = (size_t)dataSize;
    columns[colNum - 1].getData = true;
    return ODBC::GetColumnValue(env, m_hSTMT, columns[colNum - 1], buffer, bufferLength);
  }
  else
  {
    ODBC::FreeColumns(columns, &colCount);
    FREE(buffer);
    return env.Null();
  }
}

/*
 * Close
 */
Napi::Value ODBCResult::Close(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::Close - Entry\n");
  Napi::Env env = info.Env();
  Napi::Function cb;
  SQLUSMALLINT closeOption = SQL_DROP;

  if (info.Length() == 2)
  {
    if (!info[0].IsNumber()) { Napi::TypeError::New(env, "Argument 0 must be an Integer.").ThrowAsJavaScriptException(); return env.Undefined(); }
    if (!info[1].IsFunction()) { Napi::TypeError::New(env, "Argument 1 must be a Function.").ThrowAsJavaScriptException(); return env.Undefined(); }
    closeOption = (SQLUSMALLINT)info[0].As<Napi::Number>().Int32Value();
    cb = info[1].As<Napi::Function>();
  }
  else if (info.Length() == 1)
  {
    if (!info[0].IsFunction()) { Napi::TypeError::New(env, "ODBCResult::Close(): Argument 0 must be a Function.").ThrowAsJavaScriptException(); return env.Undefined(); }
    cb = info[0].As<Napi::Function>();
  }

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);
  close_result_work_data *data = (close_result_work_data *)(calloc(1, sizeof(close_result_work_data)));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->objResult = this;
  data->closeOption = closeOption;
  data->env = env;
  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_Close, (uv_after_work_cb)UV_AfterClose);
  this->Ref();
  return env.Undefined();
}

void ODBCResult::UV_Close(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCResult::UV_Close - Entry\n");
  close_result_work_data *data = (close_result_work_data *)(req->data);
  ODBCResult *objResult = data->objResult;
  SQLHSTMT hSTMT = (SQLHSTMT)objResult->m_hSTMT;

  if (data->closeOption == SQL_DROP && objResult->m_canFreeHandle)
  {
    objResult->Free();
    data->result = 0;
  }
  else if (data->closeOption == SQL_DROP && !objResult->m_canFreeHandle)
  {
    SQLFreeStmt(hSTMT, SQL_CLOSE);
    data->result = 0;
  }
  else
  {
    data->result = SQLFreeStmt(hSTMT, data->closeOption);
  }
  DEBUG_PRINTF("ODBCResult::UV_Close - Exit\n");
}

void ODBCResult::UV_AfterClose(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterClose - Entry\n");
  close_result_work_data *data = (close_result_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);
  ODBCResult *self = data->objResult->self();

  if (data->result != SQL_SUCCESS)
    ODBC::CallbackSQLError(env, SQL_HANDLE_STMT, data->objResult->m_hSTMT, data->cb);
  else
    data->cb->Call({env.Null()});
  PropagateCallbackException(env);

  self->Unref();
  delete data->cb;
  free(data);
  free(req);
}

/*
 * CloseSync
 */
Napi::Value ODBCResult::CloseSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::CloseSync - Entry\n");
  Napi::Env env = info.Env();
  SQLUSMALLINT closeOption = SQL_DROP;

  if (info.Length() > 0 && info[0].IsNumber())
    closeOption = (SQLUSMALLINT)info[0].As<Napi::Number>().Int32Value();

  if (closeOption == SQL_DROP && m_canFreeHandle)
    this->Free();
  else if (closeOption == SQL_DROP && !m_canFreeHandle)
    SQLFreeStmt(m_hSTMT, SQL_CLOSE);
  else
    SQLFreeStmt(m_hSTMT, closeOption);

  return Napi::Boolean::New(env, true);
}

/*
 * MoreResultsSync
 */
Napi::Value ODBCResult::MoreResultsSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::MoreResultsSync\n");
  Napi::Env env = info.Env();

  SQLRETURN ret = SQLMoreResults(m_hSTMT);

  if (ret == SQL_ERROR)
    Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT, (char *)"[node-odbc] Error in ODBCResult::MoreResultsSync").ToString()).ThrowAsJavaScriptException();

  return Napi::Boolean::New(env, SQL_SUCCEEDED(ret) || ret == SQL_ERROR);
}

/*
 * GetColumnNamesSync
 */
Napi::Value ODBCResult::GetColumnNamesSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::GetColumnNamesSync - Entry\n");
  Napi::Env env = info.Env();

  if (colCount == 0) columns = ODBC::GetColumns(m_hSTMT, &colCount);

  Napi::Array cols = Napi::Array::New(env);
  for (int i = 0; i < colCount; i++)
    cols.Set((uint32_t)i, Napi::String::New(env, (const char *)columns[i].name));

  ODBC::FreeColumns(columns, &colCount);
  return cols;
}

/*
 * GetColumnMetadataSync
 */
Napi::Value ODBCResult::GetColumnMetadataSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::GetColumnMetadataSync - Entry\n");
  Napi::Env env = info.Env();

  if (colCount == 0) columns = ODBC::GetColumns(m_hSTMT, &colCount);

  Napi::Array result = Napi::Array::New(env);

  for (int i = 0; i < colCount; i++)
  {
    Napi::Object col = Napi::Object::New(env);
    col.Set("index", Napi::Number::New(env, columns[i].index));
    col.Set("SQL_DESC_NAME", Napi::String::New(env, (const char *)columns[i].name));
    col.Set("SQL_DESC_TYPE_NAME", Napi::String::New(env, (const char *)columns[i].type_name));
    col.Set("SQL_DESC_CONSIZE_TYPE", Napi::Number::New(env, (int32_t)columns[i].type));
    col.Set("SQL_DESC_DISPLAY_SIZE", Napi::Number::New(env, columns[i].max_display_len));
    col.Set("SQL_DESC_PRECISION", Napi::Number::New(env, columns[i].precision));
    col.Set("SQL_DESC_SCALE", Napi::Number::New(env, columns[i].scale));
    col.Set("SQL_DESC_LENGTH", Napi::Number::New(env, columns[i].field_len));
    result.Set((uint32_t)i, col);
  }

  ODBC::FreeColumns(columns, &colCount);
  return result;
}

/*
 * GetSQLErrorSync
 */
Napi::Value ODBCResult::GetSQLErrorSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::GetSQLErrorSync\n");
  Napi::Env env = info.Env();
  return ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT, (char *)"");
}

/*
 * GetAffectedRowsSync
 */
Napi::Value ODBCResult::GetAffectedRowsSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::GetAffectedRowsSync\n");
  Napi::Env env = info.Env();
  SQLLEN rowCount = 0;
  SQLRETURN ret = SQLRowCount(m_hSTMT, &rowCount);
  if (!SQL_SUCCEEDED(ret)) rowCount = 0;
  return Napi::Number::New(env, (double)rowCount);
}

/*
 * Block Fetch Support
 */
#define BLOCK_FETCH_MAX_COL_SIZE 65536

bool ODBCResult::InitBlockFetch()
{
  if (m_blockFetchInitialized) return (m_rowArraySize > 1);
  m_blockFetchInitialized = true;

  SQLULEN rowArraySize = 0;
  SQLRETURN ret = SQLGetStmtAttr(m_hSTMT, SQL_ATTR_ROW_ARRAY_SIZE, &rowArraySize, SQL_IS_UINTEGER, NULL);
  if (!SQL_SUCCEEDED(ret) || rowArraySize <= 1) { m_rowArraySize = 1; return false; }

  m_rowArraySize = rowArraySize;

  if (colCount == 0) columns = ODBC::GetColumns(m_hSTMT, &colCount);
  if (colCount == 0) { m_rowArraySize = 1; return false; }

  m_rowStatusArray = (SQLUSMALLINT *)calloc(m_rowArraySize, sizeof(SQLUSMALLINT));
  if (!m_rowStatusArray) { m_rowArraySize = 1; return false; }

  ret = SQLSetStmtAttr(m_hSTMT, SQL_ATTR_ROW_STATUS_PTR, m_rowStatusArray, 0);
  if (!SQL_SUCCEEDED(ret)) { free(m_rowStatusArray); m_rowStatusArray = NULL; m_rowArraySize = 1; return false; }

  ret = SQLSetStmtAttr(m_hSTMT, SQL_ATTR_ROWS_FETCHED_PTR, &m_rowsFetched, 0);
  if (!SQL_SUCCEEDED(ret)) { free(m_rowStatusArray); m_rowStatusArray = NULL; m_rowArraySize = 1; return false; }

  if (!BindColumnsForBlockFetch()) { FreeBlockFetchBuffers(); m_rowArraySize = 1; return false; }
  return true;
}

bool ODBCResult::BindColumnsForBlockFetch()
{
  m_boundCols = (BoundColumn *)calloc(colCount, sizeof(BoundColumn));
  if (!m_boundCols) return false;

  for (int i = 0; i < colCount; i++)
  {
    SQLSMALLINT cType;
    SQLLEN elemSize;

    switch (columns[i].type)
    {
    case SQL_INTEGER: case SQL_SMALLINT: case SQL_TINYINT:
      cType = SQL_C_SLONG; elemSize = sizeof(SQLINTEGER); break;
    case SQL_BIGINT:
      cType = SQL_C_CHAR; elemSize = 22; break;
    case SQL_FLOAT: case SQL_REAL: case SQL_DOUBLE: case SQL_NUMERIC: case SQL_DECIMAL: case SQL_DECFLOAT:
      cType = SQL_C_DOUBLE; elemSize = sizeof(double); break;
    case SQL_BIT:
      cType = SQL_C_CHAR; elemSize = 4; break;
    case SQL_DATETIME: case SQL_TIMESTAMP: case SQL_TYPE_TIMESTAMP:
#ifdef _WIN32
      cType = SQL_C_CHAR; elemSize = sizeof(SQL_TIMESTAMP_STRUCT); break;
#else
      cType = SQL_C_TYPE_TIMESTAMP; elemSize = sizeof(SQL_TIMESTAMP_STRUCT); break;
#endif
    case SQL_BLOB: case SQL_BINARY: case SQL_VARBINARY: case SQL_LONGVARBINARY:
    {
      cType = SQL_C_BINARY;
      SQLLEN colSize = columns[i].field_len;
      if (colSize <= 0 || colSize > BLOCK_FETCH_MAX_COL_SIZE) colSize = BLOCK_FETCH_MAX_COL_SIZE;
      elemSize = colSize;
      break;
    }
    default:
    {
      cType = SQL_C_CHAR;
#ifdef UNICODE
      cType = SQL_C_WCHAR;
#endif
      SQLLEN colSize = columns[i].field_len;
      if (colSize <= 0 || colSize > BLOCK_FETCH_MAX_COL_SIZE) colSize = BLOCK_FETCH_MAX_COL_SIZE;
#ifdef UNICODE
      elemSize = (colSize + 1) * sizeof(uint16_t);
#else
      elemSize = colSize + 1;
#endif
      break;
    }
    }

    m_boundCols[i].cType = cType;
    m_boundCols[i].elementSize = elemSize;
    m_boundCols[i].data = calloc(m_rowArraySize, elemSize);
    if (!m_boundCols[i].data) return false;
    m_boundCols[i].indicators = (SQLLEN *)calloc(m_rowArraySize, sizeof(SQLLEN));
    if (!m_boundCols[i].indicators) return false;

    SQLRETURN ret = SQLBindCol(m_hSTMT, (SQLUSMALLINT)(i + 1), cType, m_boundCols[i].data, elemSize, m_boundCols[i].indicators);
    if (!SQL_SUCCEEDED(ret)) return false;
  }
  return true;
}

void ODBCResult::FreeBlockFetchBuffers()
{
  if (m_boundCols)
  {
    for (int i = 0; i < colCount; i++)
    {
      if (m_boundCols[i].data) { free(m_boundCols[i].data); m_boundCols[i].data = NULL; }
      if (m_boundCols[i].indicators) { free(m_boundCols[i].indicators); m_boundCols[i].indicators = NULL; }
    }
    free(m_boundCols); m_boundCols = NULL;
  }
  if (m_rowStatusArray) { free(m_rowStatusArray); m_rowStatusArray = NULL; }
  m_blockFetchInitialized = false;
  m_rowArraySize = 0;
}

Napi::Value ODBCResult::GetBoundColumnValue(Napi::Env env, int colIndex, SQLULEN rowIndex)
{
  if (m_fileColBindings && colIndex < m_fileColCount && m_fileColBindings[colIndex].bound)
  {
    FileColumnBinding *fcb = &m_fileColBindings[colIndex];
    if (fcb->indicator == SQL_NULL_DATA) return env.Null();
    return Napi::String::New(env, fcb->fileName);
  }

  BoundColumn *bc = &m_boundCols[colIndex];
  SQLLEN indicator = bc->indicators[rowIndex];
  if (indicator == SQL_NULL_DATA) return env.Null();

  char *rowData = (char *)bc->data + (rowIndex * bc->elementSize);

  switch (columns[colIndex].type)
  {
  case SQL_INTEGER: case SQL_SMALLINT: case SQL_TINYINT:
    return Napi::Number::New(env, (int)(*(SQLINTEGER *)rowData));

  case SQL_BIGINT:
    return Napi::String::New(env, (const char *)rowData);

  case SQL_FLOAT: case SQL_REAL: case SQL_DOUBLE: case SQL_NUMERIC: case SQL_DECIMAL: case SQL_DECFLOAT:
    return Napi::Number::New(env, *(double *)rowData);

  case SQL_BIT:
    return Napi::Boolean::New(env, (*rowData != '0'));

  case SQL_DATETIME: case SQL_TIMESTAMP: case SQL_TYPE_TIMESTAMP:
  {
    SQL_TIMESTAMP_STRUCT *ts = (SQL_TIMESTAMP_STRUCT *)rowData;
#ifdef _WIN32
    struct tm timeInfo = {};
#elif defined(_AIX) || defined(__MVS__)
    struct tm timeInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
    struct tm timeInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
    timeInfo.tm_year = ts->year - 1900;
    timeInfo.tm_mon = ts->month - 1;
    timeInfo.tm_mday = ts->day;
    timeInfo.tm_hour = ts->hour;
    timeInfo.tm_min = ts->minute;
    timeInfo.tm_sec = ts->second;
    timeInfo.tm_isdst = -1;

#ifdef TIMEGM
    double msec = (double(timegm(&timeInfo)) * 1000) + (ts->fraction / 1000000);
#else
    double msec = (double(mktime(&timeInfo)) * 1000) + (ts->fraction / 1000000);
#endif
    return Napi::Date::New(env, msec);
  }

  case SQL_BLOB: case SQL_BINARY: case SQL_VARBINARY: case SQL_LONGVARBINARY:
  {
    SQLLEN dataLen = indicator;
    if (dataLen > bc->elementSize) dataLen = bc->elementSize;
    return Napi::Buffer<char>::Copy(env, rowData, (size_t)dataLen);
  }

  default:
  {
#ifdef UNICODE
    return Napi::String::New(env, (const char16_t *)rowData);
#else
    return Napi::String::New(env, (const char *)rowData);
#endif
  }
  }
}

SQLRETURN ODBCResult::BlockFetchNextRow(SQLULEN *outRowIndex)
{
  if (m_blockExhausted) return SQL_NO_DATA;

  while (m_currentRowInBlock < m_rowsFetched)
  {
    SQLUSMALLINT st = m_rowStatusArray[m_currentRowInBlock];
    if (st == SQL_ROW_SUCCESS || st == SQL_ROW_SUCCESS_WITH_INFO)
    {
      *outRowIndex = m_currentRowInBlock;
      m_currentRowInBlock++;
      return SQL_SUCCESS;
    }
    m_currentRowInBlock++;
  }

  m_rowsFetched = 0;
  m_currentRowInBlock = 0;
  SQLRETURN ret = SQLFetch(m_hSTMT);

  if (ret == SQL_NO_DATA) { m_blockExhausted = true; return SQL_NO_DATA; }
  if (ret == SQL_ERROR) return SQL_ERROR;

  while (m_currentRowInBlock < m_rowsFetched)
  {
    SQLUSMALLINT st = m_rowStatusArray[m_currentRowInBlock];
    if (st == SQL_ROW_SUCCESS || st == SQL_ROW_SUCCESS_WITH_INFO)
    {
      *outRowIndex = m_currentRowInBlock;
      m_currentRowInBlock++;
      return SQL_SUCCESS;
    }
    m_currentRowInBlock++;
  }

  return BlockFetchNextRow(outRowIndex);
}

/*
 * BindFileToColSync
 */
Napi::Value ODBCResult::BindFileToColSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBCResult::BindFileToColSync - Entry\n");
  Napi::Env env = info.Env();

  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsString())
  {
    Napi::TypeError::New(env, "bindFileToColSync(colNum, filePath[, fileOption]): colNum (integer) and filePath (string) are required.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int colNum = info[0].As<Napi::Number>().Int32Value();
  std::string filePath = info[1].As<Napi::String>().Utf8Value();
  int filePathLen = (int)filePath.length();

  if (colNum < 1) { Napi::RangeError::New(env, "colNum must be >= 1").ThrowAsJavaScriptException(); return env.Undefined(); }
  if (filePathLen <= 0 || filePathLen >= FILE_COL_MAX_PATH) { Napi::RangeError::New(env, "filePath is empty or exceeds max path length").ThrowAsJavaScriptException(); return env.Undefined(); }

  SQLUINTEGER fileOption = SQL_FILE_OVERWRITE;
  if (info.Length() >= 3 && info[2].IsNumber())
    fileOption = info[2].As<Napi::Number>().Uint32Value();

  if (colCount == 0) columns = ODBC::GetColumns(m_hSTMT, &colCount);
  if (colNum > colCount) { Napi::RangeError::New(env, "colNum exceeds the number of columns in the result set").ThrowAsJavaScriptException(); return env.Undefined(); }

  if (!m_fileColBindings)
  {
    m_fileColBindings = (FileColumnBinding *)calloc(colCount, sizeof(FileColumnBinding));
    if (!m_fileColBindings) { Napi::Error::New(env, "Failed to allocate memory for file column bindings").ThrowAsJavaScriptException(); return env.Undefined(); }
    m_fileColCount = colCount;
  }

  FileColumnBinding *fcb = &m_fileColBindings[colNum - 1];
  memset(fcb, 0, sizeof(FileColumnBinding));
  strncpy(fcb->fileName, filePath.c_str(), FILE_COL_MAX_PATH - 1);
  fcb->fileName[FILE_COL_MAX_PATH - 1] = '\0';
  fcb->fileNameLength = (SQLSMALLINT)filePathLen;
  fcb->fileOption = fileOption;
  fcb->stringLength = 0;
  fcb->indicator = 0;

  SQLRETURN ret = SQLBindFileToCol(m_hSTMT, (SQLUSMALLINT)colNum,
    (SQLCHAR *)fcb->fileName, &fcb->fileNameLength, &fcb->fileOption,
    (SQLSMALLINT)FILE_COL_MAX_PATH, &fcb->stringLength, &fcb->indicator);

  if (!SQL_SUCCEEDED(ret))
  {
    Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, m_hSTMT, (char *)"Error in ODBCResult::BindFileToColSync").ToString()).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  fcb->bound = true;
  columns[colNum - 1].isFileBound = true;
  return Napi::Boolean::New(env, true);
}

/*
 * OverrideFileColumns
 */
void ODBCResult::OverrideFileColumns(Napi::Env env, Napi::Value row, int fetchMode)
{
  if (!m_fileColBindings || !row.IsObject() || row.IsNull()) return;

  Napi::Object obj = row.As<Napi::Object>();

  for (int i = 0; i < m_fileColCount && i < colCount; i++)
  {
    FileColumnBinding *fcb = &m_fileColBindings[i];
    if (!fcb->bound) continue;

    Napi::Value val;
    if (fcb->indicator == SQL_NULL_DATA) val = env.Null();
    else val = Napi::String::New(env, fcb->fileName);

    if (fetchMode == FETCH_ARRAY)
      obj.Set((uint32_t)i, val);
    else
    {
#ifdef UNICODE
      obj.Set(Napi::String::New(env, (const char16_t *)columns[i].name), val);
#else
      obj.Set(Napi::String::New(env, (const char *)columns[i].name), val);
#endif
    }
  }
}

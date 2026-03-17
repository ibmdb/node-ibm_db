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

Nan::Persistent<Function> ODBCResult::constructor;
Nan::Persistent<String> ODBCResult::OPTION_FETCH_MODE;

NAN_MODULE_INIT(ODBCResult::Init)
{
  DEBUG_PRINTF("ODBCResult::Init\n");
  Nan::HandleScope scope;

  Local<FunctionTemplate> constructor_template = Nan::New<FunctionTemplate>(New);

  // Constructor Template
  constructor_template->SetClassName(Nan::New("ODBCResult").ToLocalChecked());

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = constructor_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);

  // Prototype Methods
  Nan::SetPrototypeMethod(constructor_template, "fetchAll", FetchAll);
  Nan::SetPrototypeMethod(constructor_template, "fetch", Fetch);
  Nan::SetPrototypeMethod(constructor_template, "getData", GetData);

  Nan::SetPrototypeMethod(constructor_template, "moreResultsSync", MoreResultsSync);
  Nan::SetPrototypeMethod(constructor_template, "close", Close);
  Nan::SetPrototypeMethod(constructor_template, "closeSync", CloseSync);
  Nan::SetPrototypeMethod(constructor_template, "fetchSync", FetchSync);
  Nan::SetPrototypeMethod(constructor_template, "fetchAllSync", FetchAllSync);
  Nan::SetPrototypeMethod(constructor_template, "fetchN", FetchN);
  Nan::SetPrototypeMethod(constructor_template, "fetchNSync", FetchNSync);
  Nan::SetPrototypeMethod(constructor_template, "getDataSync", GetDataSync);
  Nan::SetPrototypeMethod(constructor_template, "getColumnNamesSync", GetColumnNamesSync);
  Nan::SetPrototypeMethod(constructor_template, "getColumnMetadataSync", GetColumnMetadataSync);
  Nan::SetPrototypeMethod(constructor_template, "getSQLErrorSync", GetSQLErrorSync);
  Nan::SetPrototypeMethod(constructor_template, "getAffectedRowsSync", GetAffectedRowsSync);

  // Properties
  OPTION_FETCH_MODE.Reset(Nan::New("fetchMode").ToLocalChecked());
  Nan::SetAccessor(instance_template, Nan::New("fetchMode").ToLocalChecked(), FetchModeGetter, FetchModeSetter);

  // Attach the Database Constructor to the target object
  constructor.Reset(Nan::GetFunction(constructor_template).ToLocalChecked());
  Nan::Set(target, Nan::New("ODBCResult").ToLocalChecked(),
           Nan::GetFunction(constructor_template).ToLocalChecked());
}

ODBCResult::~ODBCResult()
{
  DEBUG_PRINTF("ODBCResult::~ODBCResult m_hSTMT=%X m_canFreeHandle=%d\n", m_hSTMT, m_canFreeHandle);
  this->Free();
}

void ODBCResult::Free()
{
  FreeBlockFetchBuffers();

  if (m_hSTMT && m_canFreeHandle)
  {
    SQLFreeHandle(SQL_HANDLE_STMT, m_hSTMT);
    DEBUG_PRINTF("ODBCResult::Free SQLFreeHandle called for m_hSTMT=%X\n", m_hSTMT);
    m_hSTMT = (SQLHSTMT)NULL;
    m_canFreeHandle = 0;
  }

  if (bufferLength != 0)
  {
    bufferLength = 0;
  }
  if (buffer != NULL)
  {
    free((uint16_t *)buffer);
    buffer = NULL;
  }
}

NAN_METHOD(ODBCResult::New)
{
  DEBUG_PRINTF("ODBCResult::New\n");
  Nan::HandleScope scope;

  REQ_EXT_ARG(0, js_henv);
  REQ_EXT_ARG(1, js_hdbc);
  REQ_EXT_ARG(2, js_hstmt);
  REQ_EXT_ARG(3, js_canFreeHandle);

  SQLHENV hENV = (SQLHENV)((intptr_t)js_henv->Value());
  SQLHDBC hDBC = (SQLHDBC)((intptr_t)js_hdbc->Value());
  SQLHSTMT hSTMT = (SQLHSTMT)((intptr_t)js_hstmt->Value());
  bool *canFreeHandle = static_cast<bool *>(js_canFreeHandle->Value());

  // create a new OBCResult object
  ODBCResult *objODBCResult = new ODBCResult(hENV, hDBC, hSTMT, *canFreeHandle);

  DEBUG_PRINTF("ODBCResult::New m_hENV=%X m_hDBC=%X m_hSTMT=%X canFreeHandle=%X\n",
               objODBCResult->m_hENV,
               objODBCResult->m_hDBC,
               objODBCResult->m_hSTMT,
               objODBCResult->m_canFreeHandle);

  // free the pointer to canFreeHandle
  delete canFreeHandle;

  // specify the buffer length
  objODBCResult->bufferLength = MAX_VALUE_SIZE;
  objODBCResult->buffer = NULL; // Will get allocated in ODBC::GetColumnValue

  // set the initial colCount to 0
  objODBCResult->colCount = 0;

  // default fetchMode to FETCH_OBJECT
  objODBCResult->m_fetchMode = FETCH_OBJECT;

  // initialize block fetch fields
  objODBCResult->m_rowArraySize = 0;
  objODBCResult->m_rowsFetched = 0;
  objODBCResult->m_currentRowInBlock = 0;
  objODBCResult->m_rowStatusArray = NULL;
  objODBCResult->m_boundCols = NULL;
  objODBCResult->m_blockFetchInitialized = false;
  objODBCResult->m_blockExhausted = false;

  objODBCResult->Wrap(info.Holder());

  info.GetReturnValue().Set(info.Holder());
}

NAN_GETTER(ODBCResult::FetchModeGetter)
{
  Nan::HandleScope scope;

  ODBCResult *obj = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  info.GetReturnValue().Set(Nan::New(obj->m_fetchMode));
}

NAN_SETTER(ODBCResult::FetchModeSetter)
{
  Nan::HandleScope scope;

  ODBCResult *obj = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  if (value->IsNumber())
  {
    obj->m_fetchMode = Nan::To<int32_t>(value).FromJust();
  }
}

/*
 * Fetch
 */

NAN_METHOD(ODBCResult::Fetch)
{
  DEBUG_PRINTF("ODBCResult::Fetch\n");
  Nan::HandleScope scope;

  ODBCResult *objODBCResult = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  fetch_work_data *data = (fetch_work_data *)calloc(1, sizeof(fetch_work_data));
  if (!data)
    free(work_req); // Below MEMCHECK macro will log error and return;
  MEMCHECK(data);

  Local<Function> cb;

  // set the fetch mode to the default of this instance
  data->fetchMode = objODBCResult->m_fetchMode;

  if (info.Length() == 1 && info[0]->IsFunction())
  {
    cb = Local<Function>::Cast(info[0]);
  }
  else if (info.Length() == 2 && info[0]->IsObject() && info[1]->IsFunction())
  {
    cb = Local<Function>::Cast(info[1]);

    Local<Object> obj = Nan::To<v8::Object>(info[0]).ToLocalChecked();

    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (Nan::HasOwnProperty(obj, fetchModeKey).IsJust() && Nan::Get(obj, fetchModeKey).ToLocalChecked()->IsInt32())
    {
      data->fetchMode = Nan::To<Uint32>(Nan::Get(obj, fetchModeKey).ToLocalChecked()).ToLocalChecked()->Value();
    }
  }
  else
  {
    free(data);
    free(work_req);
    return Nan::ThrowTypeError("ODBCResult::Fetch(): 1 or 2 arguments are required. The last argument must be a callback function.");
  }

  DEBUG_PRINTF("ODBCResult::Fetch fetchMode = %i, hSTMT = %X\n", data->fetchMode, objODBCResult->m_hSTMT);
  data->cb = new Nan::Callback(cb);

  data->objResult = objODBCResult;
  data->useBlockFetch = false;
  data->blockRowIndex = 0;
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_Fetch,
      (uv_after_work_cb)UV_AfterFetch);

  objODBCResult->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCResult::UV_Fetch(uv_work_t *work_req)
{
  DEBUG_PRINTF("ODBCResult::UV_Fetch\n");

  fetch_work_data *data = (fetch_work_data *)(work_req->data);

  // InitBlockFetch is safe on worker thread (no V8 calls)
  bool useBlockFetch = data->objResult->InitBlockFetch();
  data->useBlockFetch = useBlockFetch;

  if (useBlockFetch)
  {
    // Block fetch: fetch entire block at once
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
      if (data->result == SQL_NO_DATA)
        self->m_blockExhausted = true;
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
  Nan::HandleScope scope;

  fetch_work_data *data = (fetch_work_data *)(work_req->data);

  SQLRETURN ret = data->result;
  // TODO: we should probably define this on the work data so we
  // don't have to keep creating it?
  Local<Value> objError;
  bool moreWork = true;
  bool error = false;

  if (data->objResult->colCount == 0)
  {
    data->objResult->columns = ODBC::GetColumns(
        data->objResult->m_hSTMT,
        &data->objResult->colCount);
  }

  // check to see if the result has no columns
  if (data->objResult->colCount == 0 || data->fetchMode == FETCH_NODATA)
  {
    // this means
    moreWork = false;
  }
  // check to see if there was an error
  else if (ret == SQL_ERROR)
  {
    moreWork = false;
    error = true;

    objError = ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        data->objResult->m_hSTMT,
        (char *)"Error in ODBCResult::UV_AfterFetch");
  }
  // check to see if we are at the end of the recordset
  else if (ret == SQL_NO_DATA)
  {
    moreWork = false;
  }

  if (moreWork)
  {
    Local<Value> info[2];

    info[0] = Nan::Null();

    if (data->useBlockFetch)
    {
      // Return an array of all valid rows in this block
      ODBCResult *self = data->objResult->self();
      Local<Array> blockArray = Nan::New<Array>();
      int validCount = 0;

      for (SQLULEN r = 0; r < self->m_rowsFetched; r++)
      {
        SQLUSMALLINT status = self->m_rowStatusArray[r];
        if (status == SQL_ROW_SUCCESS || status == SQL_ROW_SUCCESS_WITH_INFO)
        {
          if (data->fetchMode == FETCH_ARRAY)
          {
            Local<Array> array = Nan::New<Array>();
            for (int c = 0; c < self->colCount; c++)
            {
              Nan::Set(array, Nan::New(c), self->GetBoundColumnValue(c, r));
            }
            Nan::Set(blockArray, Nan::New(validCount++), array);
          }
          else
          {
            Local<Object> tuple = Nan::New<Object>();
            for (int c = 0; c < self->colCount; c++)
            {
#ifdef UNICODE
              Nan::Set(tuple,
                       Nan::New((uint16_t *)self->columns[c].name).ToLocalChecked(),
                       self->GetBoundColumnValue(c, r));
#else
              Nan::Set(tuple,
                       Nan::New((const char *)self->columns[c].name).ToLocalChecked(),
                       self->GetBoundColumnValue(c, r));
#endif
            }
            Nan::Set(blockArray, Nan::New(validCount++), tuple);
          }
        }
      }
      info[1] = blockArray;
    }
    else if (data->fetchMode == FETCH_ARRAY)
    {
      info[1] = ODBC::GetRecordArray(
          data->objResult->m_hSTMT,
          data->objResult->columns,
          &data->objResult->colCount,
          data->objResult->buffer,
          data->objResult->bufferLength);
    }
    else
    {
      info[1] = ODBC::GetRecordTuple(
          data->objResult->m_hSTMT,
          data->objResult->columns,
          &data->objResult->colCount,
          data->objResult->buffer,
          data->objResult->bufferLength);
    }

    Nan::TryCatch try_catch;

    CallNanCallback(data->cb, 2, info);
    delete data->cb;

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }
  else
  {
    if (data->fetchMode != FETCH_NODATA)
    {
      ODBC::FreeColumns(data->objResult->columns, &data->objResult->colCount);
    }
    FREE(data->objResult->buffer);

    Local<Value> info[2];

    // if there was an error, pass that as arg[0] otherwise Null
    if (error)
    {
      info[0] = objError;
    }
    else
    {
      info[0] = Nan::Null();
    }

    info[1] = Nan::Null();

    Nan::TryCatch try_catch;

    CallNanCallback(data->cb, 2, info);
    delete data->cb;

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  data->objResult->Unref();

  free(data);
  free(work_req);

  return;
}

/*
 * FetchSync
 */

NAN_METHOD(ODBCResult::FetchSync)
{
  DEBUG_PRINTF("ODBCResult::FetchSync - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *objResult = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  Local<Value> objError;
  bool moreWork = true;
  bool error = false;
  int fetchMode = objResult->m_fetchMode;

  if (info.Length() == 1 && info[0]->IsObject())
  {
    // Local<Object> obj = info[0]->ToObject();
    Local<Object> obj = Nan::To<v8::Object>(info[0]).ToLocalChecked();

    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (Nan::HasOwnProperty(obj, fetchModeKey).IsJust() && Nan::Get(obj, fetchModeKey).ToLocalChecked()->IsInt32())
    {
      fetchMode = Nan::To<Uint32>(Nan::Get(obj, fetchModeKey).ToLocalChecked()).ToLocalChecked()->Value();
    }
  }

  if (objResult->colCount == 0)
  {
    objResult->columns = ODBC::GetColumns(
        objResult->m_hSTMT,
        &objResult->colCount);
  }

  // Check if block fetch mode is active
  bool useBlockFetch = objResult->InitBlockFetch();

  SQLRETURN ret;
  SQLULEN rowIdx = 0;

  if (useBlockFetch)
  {
    // Block fetch: fetch entire block and return array of rows
    if (objResult->m_blockExhausted)
    {
      ret = SQL_NO_DATA;
    }
    else
    {
      objResult->m_rowsFetched = 0;
      objResult->m_currentRowInBlock = 0;
      ret = SQLFetch(objResult->m_hSTMT);
    }
  }
  else
  {
    ret = SQLFetch(objResult->m_hSTMT);
  }

  // check to see if there was an error
  if (ret == SQL_ERROR)
  {
    moreWork = false;
    error = true;

    objError = ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        objResult->m_hSTMT,
        (char *)"Error in ODBCResult::FetchSync");
  }
  // check to see if we are at the end of the recordset
  else if (ret == SQL_NO_DATA)
  {
    if (useBlockFetch)
      objResult->m_blockExhausted = true;
    moreWork = false;
  }
  // check to see if the result has no columns
  else if (objResult->colCount == 0)
  {
    moreWork = false;
  }

  if (!error && fetchMode == FETCH_NODATA)
  {
    info.GetReturnValue().Set(Nan::Null());
  }
  else if (moreWork)
  {
    Local<Value> data;

    if (useBlockFetch)
    {
      // Return an array of all valid rows in this block
      Local<Array> blockArray = Nan::New<Array>();
      int validCount = 0;

      for (SQLULEN r = 0; r < objResult->m_rowsFetched; r++)
      {
        SQLUSMALLINT status = objResult->m_rowStatusArray[r];
        if (status == SQL_ROW_SUCCESS || status == SQL_ROW_SUCCESS_WITH_INFO)
        {
          if (fetchMode == FETCH_ARRAY)
          {
            Local<Array> array = Nan::New<Array>();
            for (int c = 0; c < objResult->colCount; c++)
            {
              Nan::Set(array, Nan::New(c), objResult->GetBoundColumnValue(c, r));
            }
            Nan::Set(blockArray, Nan::New(validCount++), array);
          }
          else
          {
            Local<Object> tuple = Nan::New<Object>();
            for (int c = 0; c < objResult->colCount; c++)
            {
#ifdef UNICODE
              Nan::Set(tuple,
                       Nan::New((uint16_t *)objResult->columns[c].name).ToLocalChecked(),
                       objResult->GetBoundColumnValue(c, r));
#else
              Nan::Set(tuple,
                       Nan::New((const char *)objResult->columns[c].name).ToLocalChecked(),
                       objResult->GetBoundColumnValue(c, r));
#endif
            }
            Nan::Set(blockArray, Nan::New(validCount++), tuple);
          }
        }
      }
      data = blockArray;
    }
    else if (fetchMode == FETCH_ARRAY)
    {
      data = ODBC::GetRecordArray(
          objResult->m_hSTMT,
          objResult->columns,
          &objResult->colCount,
          objResult->buffer,
          objResult->bufferLength);
    }
    else
    {
      data = ODBC::GetRecordTuple(
          objResult->m_hSTMT,
          objResult->columns,
          &objResult->colCount,
          objResult->buffer,
          objResult->bufferLength);
    }

    info.GetReturnValue().Set(data);
  }
  else
  {
    ODBC::FreeColumns(objResult->columns, &objResult->colCount);
    FREE(objResult->buffer);

    // if there was an error, pass that as arg[0] otherwise Null
    if (error)
    {
      Nan::ThrowError(objError);

      info.GetReturnValue().Set(Nan::Null());
    }
    else
    {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
  DEBUG_PRINTF("ODBCResult::FetchSync - Exit\n");
}

/*
 * FetchAll
 */

NAN_METHOD(ODBCResult::FetchAll)
{
  DEBUG_PRINTF("ODBCResult::FetchAll - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *objODBCResult = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  fetch_work_data *data = (fetch_work_data *)calloc(1, sizeof(fetch_work_data));
  if (!data)
    free(work_req); // Below MEMCHECK macro will log error and return;
  MEMCHECK(data);

  Local<Function> cb;

  data->fetchMode = objODBCResult->m_fetchMode;

  if (info.Length() == 1 && info[0]->IsFunction())
  {
    cb = Local<Function>::Cast(info[0]);
  }
  else if (info.Length() == 2 && info[0]->IsObject() && info[1]->IsFunction())
  {
    cb = Local<Function>::Cast(info[1]);

    Local<Object> obj = Nan::To<v8::Object>(info[0]).ToLocalChecked();

    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (Nan::HasOwnProperty(obj, fetchModeKey).IsJust() && Nan::Get(obj, fetchModeKey).ToLocalChecked()->IsInt32())
    {
      data->fetchMode = Nan::To<Uint32>(Nan::Get(obj, fetchModeKey).ToLocalChecked()).ToLocalChecked()->Value();
    }
  }
  else
  {
    free(data);
    free(work_req);
    return Nan::ThrowTypeError("ODBCResult::FetchAll(): 1 or 2 arguments are required. The last argument must be a callback function.");
  }

  data->rows.Reset(Nan::New<Array>());
  data->errorCount = 0;
  data->count = 0;
  data->objError.Reset(Nan::New<Object>());

  data->cb = new Nan::Callback(cb);
  data->objResult = objODBCResult;
  data->useBlockFetch = false;
  data->blockRowIndex = 0;

  work_req->data = data;

  uv_queue_work(uv_default_loop(),
                work_req,
                UV_FetchAll,
                (uv_after_work_cb)UV_AfterFetchAll);

  data->objResult->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBCResult::FetchAll - Exit\n");
}

void ODBCResult::UV_FetchAll(uv_work_t *work_req)
{
  DEBUG_PRINTF("ODBCResult::UV_FetchAll - Entry\n");

  fetch_work_data *data = (fetch_work_data *)(work_req->data);

  // InitBlockFetch is safe on worker thread (no V8 calls)
  bool useBlockFetch = data->objResult->InitBlockFetch();
  data->useBlockFetch = useBlockFetch;

  if (useBlockFetch)
  {
    SQLULEN rowIdx = 0;
    data->result = data->objResult->BlockFetchNextRow(&rowIdx);
    data->blockRowIndex = rowIdx;
  }
  else
  {
    data->result = SQLFetch(data->objResult->m_hSTMT);
  }
  DEBUG_PRINTF("ODBCResult::UV_FetchAll - Exit, return code = %d for stmt %X\n", data->result, data->objResult->m_hSTMT);
}

void ODBCResult::UV_AfterFetchAll(uv_work_t *work_req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterFetchAll - Entry\n");
  Nan::HandleScope scope;

  fetch_work_data *data = (fetch_work_data *)(work_req->data);

  ODBCResult *self = data->objResult->self();

  bool doMoreWork = true;

  /* Check : to see if there was an ERROR on SQLFetch() call.
   * So before GetColums call we should store the error.
   * Reason : GetColumns internally calls SQLGetDiagField method,
   * and SQLGetDiagField() method retrieves only the diagnostic information of
   * most recent CLI function call, any diagnostic information from a previous call
   * with the same handle will be lost. - issue253
   */
  if (data->result == SQL_ERROR)
  {
    data->errorCount++;
    data->objError.Reset(ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        self->m_hSTMT,
        (char *)"[node-odbc] Error in ODBCResult::UV_AfterFetchAll"));
  }

  if (self->colCount == 0)
  {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
    DEBUG_PRINTF("ODBCResult::UV_AfterFetchAll, colcount = %d, columns = %d, stmt = %X\n",
                 self->colCount, self->columns, data->objResult->m_hSTMT);
  }

  /* Check : to see if the result set has columns.
   * Queries like insert into... (which has no actual fetch data),
   * will also return error after SQLFetch call, which is expected here
   * (as we are calling SQLFetch for every SQL query) but not true,
   * hence we should ignore these error.
   */
  bool noDataFetchQuery = false;

  if (self->colCount == 0)
  {
    noDataFetchQuery = true;
    doMoreWork = false;
  }
  // check to see if we are at the end of the recordset
  else if (data->result == SQL_NO_DATA)
  {
    doMoreWork = false;
  }
  // check to see if there was an error
  else if (data->result == SQL_ERROR)
  {
    doMoreWork = false;
  }

  else
  {
    Local<Array> rows = Nan::New(data->rows);

    if (data->useBlockFetch)
    {
      SQLULEN rowIdx = data->blockRowIndex;
      if (data->fetchMode == FETCH_ARRAY)
      {
        Local<Array> array = Nan::New<Array>();
        for (int c = 0; c < self->colCount; c++)
        {
          Nan::Set(array, Nan::New(c), self->GetBoundColumnValue(c, rowIdx));
        }
        Nan::Set(rows, Nan::New(data->count), array);
      }
      else
      {
        Local<Object> tuple = Nan::New<Object>();
        for (int c = 0; c < self->colCount; c++)
        {
#ifdef UNICODE
          Nan::Set(tuple,
                   Nan::New((uint16_t *)self->columns[c].name).ToLocalChecked(),
                   self->GetBoundColumnValue(c, rowIdx));
#else
          Nan::Set(tuple,
                   Nan::New((const char *)self->columns[c].name).ToLocalChecked(),
                   self->GetBoundColumnValue(c, rowIdx));
#endif
        }
        Nan::Set(rows, Nan::New(data->count), tuple);
      }
    }
    else if (data->fetchMode == FETCH_ARRAY)
    {
      Nan::Set(rows,
               Nan::New(data->count),
               ODBC::GetRecordArray(
                   self->m_hSTMT,
                   self->columns,
                   &self->colCount,
                   self->buffer,
                   self->bufferLength));
    }
    else
    {
      Nan::Set(rows,
               Nan::New(data->count),
               ODBC::GetRecordTuple(
                   self->m_hSTMT,
                   self->columns,
                   &self->colCount,
                   self->buffer,
                   self->bufferLength));
    }
    data->count++;
  }
  if (doMoreWork)
  {
    // Go back to the thread pool and fetch more data!
    uv_queue_work(uv_default_loop(),
                  work_req,
                  UV_FetchAll,
                  (uv_after_work_cb)UV_AfterFetchAll);
  }
  else
  {
    DEBUG_PRINTF("ODBCResult::UV_AfterFetchAll Done for stmt %X\n", data->objResult->m_hSTMT);

    Local<Value> info[3];

    if (data->errorCount > 0)
    {
      if (noDataFetchQuery)
      {
        info[0] = Nan::Null();
        noDataFetchQuery = false;
      }
      else
      {
        info[0] = Nan::New(data->objError);
      }
    }
    else
    {
      info[0] = Nan::Null();
    }

    info[1] = Nan::New(data->rows);
    info[2] = Nan::New(self->colCount);
    Nan::TryCatch try_catch;

    CallNanCallback(data->cb, 3, info);
    ODBC::FreeColumns(self->columns, &self->colCount);
    FREE(self->buffer);
    delete data->cb;
    data->rows.Reset();
    data->objError.Reset();

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }

    // TODO: Do we need to free self->rows somehow?
    free(data);
    free(work_req);
    self->Unref();
  }
  DEBUG_PRINTF("ODBCResult::UV_AfterFetchAll - Exit\n");
}

/*
 * FetchAllSync
 */

NAN_METHOD(ODBCResult::FetchAllSync)
{
  DEBUG_PRINTF("ODBCResult::FetchAllSync - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  Local<Value> objError = Nan::New<Object>();

  SQLRETURN ret;
  int count = 0;
  int errorCount = 0;
  int fetchMode = self->m_fetchMode;

  if (info.Length() == 1 && info[0]->IsObject())
  {
    Local<Object> obj = Nan::To<v8::Object>(info[0]).ToLocalChecked();

    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (Nan::HasOwnProperty(obj, fetchModeKey).IsJust() && Nan::Get(obj, fetchModeKey).ToLocalChecked()->IsInt32())
    {
      fetchMode = Nan::To<Uint32>(Nan::Get(obj, fetchModeKey).ToLocalChecked()).ToLocalChecked()->Value();
    }
  }

  if (self->colCount == 0)
  {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }

  DEBUG_PRINTF("ODBCResult::FetchAllSync colCount = %i, fetchMode = %i\n", self->colCount, fetchMode);

  Local<Array> rows = Nan::New<Array>();

  // Only loop through the recordset if there are columns
  if (self->colCount > 0)
  {
    // Check if block fetch (SQL_ATTR_ROW_ARRAY_SIZE > 1) should be used
    bool useBlockFetch = self->InitBlockFetch();

    if (useBlockFetch)
    {
      // Block fetch path: SQLBindCol-based, fetches N rows per SQLFetch call
      while (true)
      {
        self->m_rowsFetched = 0;
        ret = SQLFetch(self->m_hSTMT);

        if (ret == SQL_ERROR)
        {
          errorCount++;
          objError = ODBC::GetSQLError(
              SQL_HANDLE_STMT,
              self->m_hSTMT,
              (char *)"[node-odbc] Error in ODBCResult::FetchAllSync (block fetch)");
          break;
        }

        if (ret == SQL_NO_DATA)
        {
          break;
        }

        DEBUG_PRINTF("ODBCResult::FetchAllSync block fetch: rowsFetched=%lu\n",
                     (unsigned long)self->m_rowsFetched);

        // Process all rows returned by this SQLFetch call
        for (SQLULEN r = 0; r < self->m_rowsFetched; r++)
        {
          // Skip rows with error or no-row status
          if (self->m_rowStatusArray[r] == SQL_ROW_ERROR ||
              self->m_rowStatusArray[r] == SQL_ROW_NOROW)
          {
            continue;
          }

          if (fetchMode == FETCH_ARRAY)
          {
            Local<Array> array = Nan::New<Array>();
            for (int c = 0; c < self->colCount; c++)
            {
              Nan::Set(array, Nan::New(c), self->GetBoundColumnValue(c, r));
            }
            Nan::Set(rows, Nan::New(count), array);
          }
          else
          {
            Local<Object> tuple = Nan::New<Object>();
            for (int c = 0; c < self->colCount; c++)
            {
#ifdef UNICODE
              Nan::Set(tuple,
                       Nan::New((uint16_t *)self->columns[c].name).ToLocalChecked(),
                       self->GetBoundColumnValue(c, r));
#else
              Nan::Set(tuple,
                       Nan::New((const char *)self->columns[c].name).ToLocalChecked(),
                       self->GetBoundColumnValue(c, r));
#endif
            }
            Nan::Set(rows, Nan::New(count), tuple);
          }
          count++;
        }
      }
    }
    else
    {
      // Normal single-row fetch path (original code)
      while (true)
      {
        ret = SQLFetch(self->m_hSTMT);

        if (ret == SQL_ERROR)
        {
          errorCount++;
          objError = ODBC::GetSQLError(
              SQL_HANDLE_STMT,
              self->m_hSTMT,
              (char *)"[node-odbc] Error in ODBCResult::UV_AfterFetchAll; probably"
                      " your query did not have a result set.");
          break;
        }

        if (ret == SQL_NO_DATA)
        {
          break;
        }

        if (fetchMode == FETCH_ARRAY)
        {
          Nan::Set(rows,
                   Nan::New(count),
                   ODBC::GetRecordArray(
                       self->m_hSTMT,
                       self->columns,
                       &self->colCount,
                       self->buffer,
                       self->bufferLength));
        }
        else
        {
          Nan::Set(rows,
                   Nan::New(count),
                   ODBC::GetRecordTuple(
                       self->m_hSTMT,
                       self->columns,
                       &self->colCount,
                       self->buffer,
                       self->bufferLength));
        }
        count++;
      }
    }
  }
  ODBC::FreeColumns(self->columns, &self->colCount);
  FREE(self->buffer);

  // throw the error object if there were errors
  if (errorCount > 0)
  {
    Nan::ThrowError(objError);
  }

  info.GetReturnValue().Set(rows);
  DEBUG_PRINTF("ODBCResult::FetchAllSync() -Exit\n");
}

/*
 * FetchN - Fetch up to N rows asynchronously from the result set.
 * Arguments: count (int), [option (object)], callback (function)
 * Returns array of up to count rows. Does not free columns/close cursor.
 */

NAN_METHOD(ODBCResult::FetchN)
{
  DEBUG_PRINTF("ODBCResult::FetchN - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *objODBCResult = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  fetch_work_data *data = (fetch_work_data *)calloc(1, sizeof(fetch_work_data));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  Local<Function> cb;
  data->fetchMode = objODBCResult->m_fetchMode;

  if (info.Length() == 2 && info[0]->IsInt32() && info[1]->IsFunction())
  {
    data->maxCount = Nan::To<int32_t>(info[0]).FromJust();
    cb = Local<Function>::Cast(info[1]);
  }
  else if (info.Length() == 3 && info[0]->IsInt32() && info[1]->IsObject() && info[2]->IsFunction())
  {
    data->maxCount = Nan::To<int32_t>(info[0]).FromJust();
    cb = Local<Function>::Cast(info[2]);

    Local<Object> obj = Nan::To<v8::Object>(info[1]).ToLocalChecked();
    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (Nan::HasOwnProperty(obj, fetchModeKey).IsJust() && Nan::Get(obj, fetchModeKey).ToLocalChecked()->IsInt32())
    {
      data->fetchMode = Nan::To<Uint32>(Nan::Get(obj, fetchModeKey).ToLocalChecked()).ToLocalChecked()->Value();
    }
  }
  else
  {
    free(data);
    free(work_req);
    return Nan::ThrowTypeError("ODBCResult::FetchN(): Arguments must be (count, [option], callback).");
  }

  if (data->maxCount <= 0)
  {
    free(data);
    free(work_req);
    return Nan::ThrowRangeError("ODBCResult::FetchN(): count must be a positive integer.");
  }

  data->rows.Reset(Nan::New<Array>());
  data->errorCount = 0;
  data->count = 0;
  data->objError.Reset(Nan::New<Object>());

  data->cb = new Nan::Callback(cb);
  data->objResult = objODBCResult;
  data->useBlockFetch = false;
  data->blockRowIndex = 0;
  work_req->data = data;

  uv_queue_work(uv_default_loop(),
                work_req,
                UV_FetchN,
                (uv_after_work_cb)UV_AfterFetchN);

  data->objResult->Ref();
  info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBCResult::FetchN - Exit\n");
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
  {
    data->result = SQLFetch(data->objResult->m_hSTMT);
  }
}

void ODBCResult::UV_AfterFetchN(uv_work_t *work_req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterFetchN - Entry\n");
  Nan::HandleScope scope;

  fetch_work_data *data = (fetch_work_data *)(work_req->data);
  ODBCResult *self = data->objResult->self();

  bool doMoreWork = true;

  if (data->result == SQL_ERROR)
  {
    data->errorCount++;
    data->objError.Reset(ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        self->m_hSTMT,
        (char *)"[node-odbc] Error in ODBCResult::UV_AfterFetchN"));
  }

  if (self->colCount == 0)
  {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }

  if (self->colCount == 0)
  {
    doMoreWork = false;
  }
  else if (data->result == SQL_NO_DATA)
  {
    doMoreWork = false;
  }
  else if (data->result == SQL_ERROR)
  {
    doMoreWork = false;
  }
  else
  {
    Local<Array> rows = Nan::New(data->rows);

    if (data->useBlockFetch)
    {
      SQLULEN rowIdx = data->blockRowIndex;
      if (data->fetchMode == FETCH_ARRAY)
      {
        Local<Array> array = Nan::New<Array>();
        for (int c = 0; c < self->colCount; c++)
        {
          Nan::Set(array, Nan::New(c), self->GetBoundColumnValue(c, rowIdx));
        }
        Nan::Set(rows, Nan::New(data->count), array);
      }
      else
      {
        Local<Object> tuple = Nan::New<Object>();
        for (int c = 0; c < self->colCount; c++)
        {
#ifdef UNICODE
          Nan::Set(tuple,
                   Nan::New((uint16_t *)self->columns[c].name).ToLocalChecked(),
                   self->GetBoundColumnValue(c, rowIdx));
#else
          Nan::Set(tuple,
                   Nan::New((const char *)self->columns[c].name).ToLocalChecked(),
                   self->GetBoundColumnValue(c, rowIdx));
#endif
        }
        Nan::Set(rows, Nan::New(data->count), tuple);
      }
    }
    else if (data->fetchMode == FETCH_ARRAY)
    {
      Nan::Set(rows,
               Nan::New(data->count),
               ODBC::GetRecordArray(
                   self->m_hSTMT,
                   self->columns,
                   &self->colCount,
                   self->buffer,
                   self->bufferLength));
    }
    else
    {
      Nan::Set(rows,
               Nan::New(data->count),
               ODBC::GetRecordTuple(
                   self->m_hSTMT,
                   self->columns,
                   &self->colCount,
                   self->buffer,
                   self->bufferLength));
    }
    data->count++;

    // Stop if we've reached the requested batch size
    if (data->count >= data->maxCount)
    {
      doMoreWork = false;
    }
  }

  if (doMoreWork)
  {
    uv_queue_work(uv_default_loop(),
                  work_req,
                  UV_FetchN,
                  (uv_after_work_cb)UV_AfterFetchN);
  }
  else
  {
    DEBUG_PRINTF("ODBCResult::UV_AfterFetchN Done for stmt %X, count=%d\n",
                 data->objResult->m_hSTMT, data->count);

    Local<Value> info[2];

    if (data->errorCount > 0 && self->colCount > 0)
    {
      info[0] = Nan::New(data->objError);
    }
    else
    {
      info[0] = Nan::Null();
    }

    info[1] = Nan::New(data->rows);

    Nan::TryCatch try_catch;
    CallNanCallback(data->cb, 2, info);
    delete data->cb;
    data->rows.Reset();
    data->objError.Reset();

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }

    free(data);
    free(work_req);
    self->Unref();
  }
  DEBUG_PRINTF("ODBCResult::UV_AfterFetchN - Exit\n");
}

/*
 * FetchNSync - Synchronously fetch up to N rows from the result set.
 * Arguments: count (int), [option (object)]
 * Returns array of up to count rows. Does not free columns/close cursor.
 */

NAN_METHOD(ODBCResult::FetchNSync)
{
  DEBUG_PRINTF("ODBCResult::FetchNSync - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  if (!info[0]->IsInt32())
  {
    return Nan::ThrowTypeError("ODBCResult::FetchNSync(): First argument must be an integer count.");
  }

  int maxCount = Nan::To<int32_t>(info[0]).FromJust();
  if (maxCount <= 0)
  {
    return Nan::ThrowRangeError("ODBCResult::FetchNSync(): count must be a positive integer.");
  }

  Local<Value> objError = Nan::New<Object>();
  SQLRETURN ret;
  int count = 0;
  int errorCount = 0;
  int fetchMode = self->m_fetchMode;

  if (info.Length() >= 2 && info[1]->IsObject())
  {
    Local<Object> obj = Nan::To<v8::Object>(info[1]).ToLocalChecked();
    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (Nan::HasOwnProperty(obj, fetchModeKey).IsJust() && Nan::Get(obj, fetchModeKey).ToLocalChecked()->IsInt32())
    {
      fetchMode = Nan::To<Uint32>(Nan::Get(obj, fetchModeKey).ToLocalChecked()).ToLocalChecked()->Value();
    }
  }

  if (self->colCount == 0)
  {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }

  Local<Array> rows = Nan::New<Array>();

  if (self->colCount > 0)
  {
    bool useBlockFetch = self->InitBlockFetch();

    if (useBlockFetch)
    {
      SQLULEN rowIdx;
      while (count < maxCount)
      {
        ret = self->BlockFetchNextRow(&rowIdx);

        if (ret == SQL_ERROR)
        {
          errorCount++;
          objError = ODBC::GetSQLError(
              SQL_HANDLE_STMT,
              self->m_hSTMT,
              (char *)"[node-odbc] Error in ODBCResult::FetchNSync (block fetch)");
          break;
        }

        if (ret == SQL_NO_DATA)
        {
          break;
        }

        if (fetchMode == FETCH_ARRAY)
        {
          Local<Array> array = Nan::New<Array>();
          for (int c = 0; c < self->colCount; c++)
          {
            Nan::Set(array, Nan::New(c), self->GetBoundColumnValue(c, rowIdx));
          }
          Nan::Set(rows, Nan::New(count), array);
        }
        else
        {
          Local<Object> tuple = Nan::New<Object>();
          for (int c = 0; c < self->colCount; c++)
          {
#ifdef UNICODE
            Nan::Set(tuple,
                     Nan::New((uint16_t *)self->columns[c].name).ToLocalChecked(),
                     self->GetBoundColumnValue(c, rowIdx));
#else
            Nan::Set(tuple,
                     Nan::New((const char *)self->columns[c].name).ToLocalChecked(),
                     self->GetBoundColumnValue(c, rowIdx));
#endif
          }
          Nan::Set(rows, Nan::New(count), tuple);
        }
        count++;
      }
    }
    else
    {
      while (count < maxCount)
      {
        ret = SQLFetch(self->m_hSTMT);

        if (ret == SQL_ERROR)
        {
          errorCount++;
          objError = ODBC::GetSQLError(
              SQL_HANDLE_STMT,
              self->m_hSTMT,
              (char *)"[node-odbc] Error in ODBCResult::FetchNSync");
          break;
        }

        if (ret == SQL_NO_DATA)
        {
          break;
        }

        if (fetchMode == FETCH_ARRAY)
        {
          Nan::Set(rows,
                   Nan::New(count),
                   ODBC::GetRecordArray(
                       self->m_hSTMT,
                       self->columns,
                       &self->colCount,
                       self->buffer,
                       self->bufferLength));
        }
        else
        {
          Nan::Set(rows,
                   Nan::New(count),
                   ODBC::GetRecordTuple(
                       self->m_hSTMT,
                       self->columns,
                       &self->colCount,
                       self->buffer,
                       self->bufferLength));
        }
        count++;
      }
    }
  }

  if (errorCount > 0)
  {
    Nan::ThrowError(objError);
  }

  info.GetReturnValue().Set(rows);
  DEBUG_PRINTF("ODBCResult::FetchNSync - Exit, count=%d\n", count);
}

/*
 * GetData
 */

NAN_METHOD(ODBCResult::GetData)
{
  DEBUG_PRINTF("ODBCResult::GetData\n");
  Nan::HandleScope scope;

  ODBCResult *objResult = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  REQ_INT_ARG(0, colNum);
  REQ_INT_ARG(1, dataSize);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  getdata_work_data *data = (getdata_work_data *)calloc(1, sizeof(getdata_work_data));
  if (!data)
    free(work_req); // Below MEMCHECK macro will log error and return;
  MEMCHECK(data);

  Local<Function> cb;

  if (info.Length() == 3 && info[2]->IsFunction())
  {
    cb = Local<Function>::Cast(info[2]);
    data->colNum = (SQLUINTEGER)colNum;
    data->dataSize = (SQLUINTEGER)dataSize;
  }
  else if (info.Length() == 2 && info[1]->IsFunction())
  {
    cb = Local<Function>::Cast(info[1]);
    data->colNum = (SQLUINTEGER)colNum;
    data->dataSize = 0;
  }
  else
  {
    free(data);
    free(work_req);
    return Nan::ThrowTypeError("ODBCResult::GetData(): 2 or 3 arguments are required.");
  }

  DEBUG_PRINTF("ODBCResult::GetData: colNum = %d, dataSize = %d\n", colNum, dataSize);
  data->cb = new Nan::Callback(cb);

  data->objResult = objResult;
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_GetData,
      (uv_after_work_cb)UV_AfterGetData);

  objResult->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCResult::UV_GetData(uv_work_t *work_req)
{
  DEBUG_PRINTF("ODBCResult::UV_GetData - Entry\n");
  DEBUG_PRINTF("ODBCResult::UV_GetData - Exit\n");
}

void ODBCResult::UV_AfterGetData(uv_work_t *work_req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterGetData - Entry\n");
  Nan::HandleScope scope;

  getdata_work_data *data = (getdata_work_data *)(work_req->data);

  ODBCResult *objResult = data->objResult;
  Local<Value> info[2];
  info[0] = Nan::Null();
  info[1] = Nan::Null();

  if (objResult->colCount > 0 && data->colNum <= (unsigned)objResult->colCount)
  {
    objResult->bufferLength = (size_t)data->dataSize;
    objResult->columns[data->colNum - 1].getData = true;

    Nan::TryCatch tc;
    info[1] = ODBC::GetColumnValue(objResult->m_hSTMT, objResult->columns[data->colNum - 1], objResult->buffer, objResult->bufferLength);
    if (tc.HasCaught())
    {
      info[0] = tc.Exception();
    }
  }
  else
  {
    ODBC::FreeColumns(objResult->columns, &objResult->colCount);
    FREE(objResult->buffer);
  }

  Nan::TryCatch try_catch;

  CallNanCallback(data->cb, 2, info);
  delete data->cb;

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  data->objResult->Unref();

  free(data);
  free(work_req);
  DEBUG_PRINTF("ODBCResult::UV_AfterGetData - Exit\n");

  return;
}
/*
 * GetDataSync
 */

NAN_METHOD(ODBCResult::GetDataSync)
{
  DEBUG_PRINTF("ODBCResult::GetDataSync - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *objResult = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  REQ_INT_ARG(0, colNum);
  REQ_INT_ARG(1, dataSize);

  DEBUG_PRINTF("ODBCResult::GetDataSync: colNum = %d, dataSize = %d\n", colNum, dataSize);
  if (objResult->colCount > 0 && colNum <= objResult->colCount)
  {
    Local<Value> data;
    objResult->bufferLength = (size_t)dataSize;
    objResult->columns[colNum - 1].getData = true;
    data = ODBC::GetColumnValue(objResult->m_hSTMT, objResult->columns[colNum - 1], objResult->buffer, objResult->bufferLength);

    info.GetReturnValue().Set(data);
  }
  else
  {
    ODBC::FreeColumns(objResult->columns, &objResult->colCount);
    FREE(objResult->buffer);
    info.GetReturnValue().Set(Nan::Null());
  }
  DEBUG_PRINTF("ODBCResult::GetDataSync - Exit\n");
}

/*
 * Close
 * result.close([sql_close,] function(err) {})
 */

NAN_METHOD(ODBCResult::Close)
{
  DEBUG_PRINTF("ODBCResult::Close- Entry\n");

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
      return Nan::ThrowTypeError("ODBCResult::Close(): Argument 0 must be a Function.");
    }
    cb = Local<Function>::Cast(info[0]);
  }

  ODBCResult *result = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  close_result_work_data *data = (close_result_work_data *)(calloc(1, sizeof(close_result_work_data)));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->objResult = result;
  data->closeOption = closeOption;
  work_req->data = data;

  DEBUG_PRINTF("ODBCResult::Close closeOption=%d ,stmt=%X\n", closeOption, result->m_hSTMT);

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_Close,
      (uv_after_work_cb)UV_AfterClose);

  result->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBCResult::Close - Exit\n");
}
void ODBCResult::UV_Close(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCResult::UV_Close- Entry\n");

  close_result_work_data *data = (close_result_work_data *)(req->data);

  SQLRETURN ret;
  ODBCResult *objResult = data->objResult;
  SQLHSTMT m_hSTMT = (SQLHSTMT)objResult->m_hSTMT;

  DEBUG_PRINTF("ODBCResult::UV_Close m_hSTMT=%X\n", m_hSTMT);

  if (data->closeOption == SQL_DROP && objResult->m_canFreeHandle)
  {
    objResult->Free();
    data->result = 0;
  }
  else if (data->closeOption == SQL_DROP && !objResult->m_canFreeHandle)
  {
    // We technically can't free the handle so, we'll SQL_CLOSE
    //  Don't set result->m_canFreeHandle to true in this function.
    //  Handle would be freed by the call of ODBCStatement.Close().
    SQLFreeStmt(m_hSTMT, SQL_CLOSE);
  }
  else
  {
    ret = SQLFreeStmt(m_hSTMT, data->closeOption);
    data->result = ret;
  }

  DEBUG_PRINTF("ODBCResult::UV_Close - Exit m_hSTMT=%X, result=%i\n", m_hSTMT, data->result);
}

void ODBCResult::UV_AfterClose(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterClose- Entry\n");

  Nan::HandleScope scope;

  close_result_work_data *data = (close_result_work_data *)(req->data);
  ODBCResult *self = data->objResult->self();
  SQLHSTMT m_hSTMT = (SQLHSTMT)data->objResult->m_hSTMT;

  if (data->result != SQL_SUCCESS)
  {
    ODBC::CallbackSQLError(
        SQL_HANDLE_STMT,
        m_hSTMT,
        data->cb);
  }
  else
  {
    Local<Value> info[1];

    info[0] = Nan::Null();
    Nan::TryCatch try_catch;
    CallNanCallback(data->cb, 1, info);

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  self->Unref();
  delete data->cb;

  free(data);
  free(req);
  DEBUG_PRINTF("ODBCResult::UV_AfterClose - Exit, m_hSTMT = %X\n", m_hSTMT);
}

/*
 * CloseSync
 *
 */

NAN_METHOD(ODBCResult::CloseSync)
{
  DEBUG_PRINTF("ODBCResult::CloseSync - Entry\n");
  Nan::HandleScope scope;

  OPT_INT_ARG(0, closeOption, SQL_DROP);

  ODBCResult *result = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  DEBUG_PRINTF("ODBCResult::CloseSync closeOption=%i m_canFreeHandle=%i, hSTMT=%X\n",
               closeOption, result->m_canFreeHandle, result->m_hSTMT);

  if (closeOption == SQL_DROP && result->m_canFreeHandle)
  {
    result->Free();
  }
  else if (closeOption == SQL_DROP && !result->m_canFreeHandle)
  {
    // We technically can't free the handle so, we'll SQL_CLOSE
    //  Don't set result->m_canFreeHandle to true in this function.
    //  Handle would be freed by the call of ODBCStatement.Close().
    SQLFreeStmt(result->m_hSTMT, SQL_CLOSE);
  }
  else
  {
    SQLFreeStmt(result->m_hSTMT, closeOption);
  }

  info.GetReturnValue().Set(Nan::True());
  DEBUG_PRINTF("ODBCResult::CloseSync - Exit\n");
}

NAN_METHOD(ODBCResult::MoreResultsSync)
{
  DEBUG_PRINTF("ODBCResult::MoreResultsSync\n");
  Nan::HandleScope scope;

  ODBCResult *result = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  SQLRETURN ret = SQLMoreResults(result->m_hSTMT);

  if (ret == SQL_ERROR)
  {
    Nan::ThrowError(ODBC::GetSQLError(SQL_HANDLE_STMT, result->m_hSTMT, (char *)"[node-odbc] Error in ODBCResult::MoreResultsSync"));
  }

  info.GetReturnValue().Set(SQL_SUCCEEDED(ret) || ret == SQL_ERROR ? Nan::True() : Nan::False());
}

/*
 * GetColumnNamesSync
 */

NAN_METHOD(ODBCResult::GetColumnNamesSync)
{
  DEBUG_PRINTF("ODBCResult::GetColumnNamesSync - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  Local<Array> cols = Nan::New<Array>();

  if (self->colCount == 0)
  {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }

  for (int i = 0; i < self->colCount; i++)
  {
    Nan::Set(cols, Nan::New(i),
             Nan::New((const char *)self->columns[i].name).ToLocalChecked());
  }

  ODBC::FreeColumns(self->columns, &self->colCount);
  info.GetReturnValue().Set(cols);
  DEBUG_PRINTF("ODBCResult::GetColumnNamesSync - Exit\n");
}

/*
 * GetColumnMetadataSync
 */
NAN_METHOD(ODBCResult::GetColumnMetadataSync)
{
  DEBUG_PRINTF("ODBCResult::GetColumnMetadataSync - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  if (self->colCount == 0)
  {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }

  Local<Array> columns = Nan::New<Array>();

  for (int i = 0; i < self->colCount; i++)
  {
    Local<Object> col = Nan::New<Object>();
    Nan::Set(col, Nan::New("index").ToLocalChecked(), Nan::New(self->columns[i].index));
    Nan::Set(col, Nan::New("SQL_DESC_NAME").ToLocalChecked(), Nan::New((const char *)self->columns[i].name).ToLocalChecked());
    Nan::Set(col, Nan::New("SQL_DESC_TYPE_NAME").ToLocalChecked(), Nan::New((const char *)self->columns[i].type_name).ToLocalChecked());
    Nan::Set(col, Nan::New("SQL_DESC_CONSIZE_TYPE").ToLocalChecked(), Nan::New((int32_t)self->columns[i].type));
    Nan::Set(col, Nan::New("SQL_DESC_DISPLAY_SIZE").ToLocalChecked(), Nan::New(self->columns[i].max_display_len));
    Nan::Set(col, Nan::New("SQL_DESC_PRECISION").ToLocalChecked(), Nan::New(self->columns[i].precision));
    Nan::Set(col, Nan::New("SQL_DESC_SCALE").ToLocalChecked(), Nan::New(self->columns[i].scale));
    Nan::Set(col, Nan::New("SQL_DESC_LENGTH").ToLocalChecked(), Nan::New(self->columns[i].field_len));

    Nan::Set(columns, Nan::New(i), col);
  }

  ODBC::FreeColumns(self->columns, &self->colCount);
  info.GetReturnValue().Set(columns);
  DEBUG_PRINTF("ODBCResult::GetColumnMetadataSync - Exit\n");
}

/*
 * GetSQLErrorSync
 */
NAN_METHOD(ODBCResult::GetSQLErrorSync)
{
  DEBUG_PRINTF("ODBCResult::GetSQLErrorSync - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());
  Local<Value> objError = Nan::New<Object>();

  objError = ODBC::GetSQLError(SQL_HANDLE_STMT, self->m_hSTMT, (char *)"");

  info.GetReturnValue().Set(objError);
  DEBUG_PRINTF("ODBCResult::GetSQLErrorSync - Exit\n");
}

/*
 * GetAffectedRowsSync
 */
NAN_METHOD(ODBCResult::GetAffectedRowsSync)
{
  DEBUG_PRINTF("ODBCResult::GetAffectedRowsSync - Entry\n");
  Nan::HandleScope scope;

  ODBCResult *self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  SQLLEN rowCount = 0;
  SQLRETURN ret = SQLRowCount(self->m_hSTMT, &rowCount);

  if (!SQL_SUCCEEDED(ret))
  {
    rowCount = 0;
  }

  info.GetReturnValue().Set(Nan::New<Number>(rowCount));
  DEBUG_PRINTF("ODBCResult::GetAffectedRowsSync - Exit\n");
}

/*
 * Block Fetch Support
 * When SQL_ATTR_ROW_ARRAY_SIZE > 1, use SQLBindCol to fetch N rows at once.
 */

// Maximum buffer size per element for string/binary columns (cap for LOBs)
#define BLOCK_FETCH_MAX_COL_SIZE 65536

/*
 * InitBlockFetch
 * Called once before the first fetch. Queries SQL_ATTR_ROW_ARRAY_SIZE.
 * If > 1, sets up column binding for block fetch.
 * Returns true if block fetch mode is active, false for normal single-row fetch.
 */
bool ODBCResult::InitBlockFetch()
{
  if (m_blockFetchInitialized) {
    return (m_rowArraySize > 1);
  }
  m_blockFetchInitialized = true;

  // Query the current SQL_ATTR_ROW_ARRAY_SIZE from the statement
  SQLULEN rowArraySize = 0;
  SQLRETURN ret = SQLGetStmtAttr(m_hSTMT, SQL_ATTR_ROW_ARRAY_SIZE,
                                 &rowArraySize, SQL_IS_UINTEGER, NULL);
  if (!SQL_SUCCEEDED(ret) || rowArraySize <= 1) {
    m_rowArraySize = 1;
    return false;
  }

  m_rowArraySize = rowArraySize;
  DEBUG_PRINTF("ODBCResult::InitBlockFetch: SQL_ATTR_ROW_ARRAY_SIZE = %lu\n",
               (unsigned long)m_rowArraySize);

  // Get columns if not already retrieved
  if (colCount == 0) {
    columns = ODBC::GetColumns(m_hSTMT, &colCount);
  }
  if (colCount == 0) {
    m_rowArraySize = 1;
    return false;
  }

  // Set up SQL_ATTR_ROW_STATUS_PTR and SQL_ATTR_ROWS_FETCHED_PTR
  m_rowStatusArray = (SQLUSMALLINT *)calloc(m_rowArraySize, sizeof(SQLUSMALLINT));
  if (!m_rowStatusArray) {
    m_rowArraySize = 1;
    return false;
  }

  ret = SQLSetStmtAttr(m_hSTMT, SQL_ATTR_ROW_STATUS_PTR,
                        m_rowStatusArray, 0);
  if (!SQL_SUCCEEDED(ret)) {
    DEBUG_PRINTF("ODBCResult::InitBlockFetch: Failed to set SQL_ATTR_ROW_STATUS_PTR\n");
    free(m_rowStatusArray);
    m_rowStatusArray = NULL;
    m_rowArraySize = 1;
    return false;
  }

  ret = SQLSetStmtAttr(m_hSTMT, SQL_ATTR_ROWS_FETCHED_PTR,
                        &m_rowsFetched, 0);
  if (!SQL_SUCCEEDED(ret)) {
    DEBUG_PRINTF("ODBCResult::InitBlockFetch: Failed to set SQL_ATTR_ROWS_FETCHED_PTR\n");
    free(m_rowStatusArray);
    m_rowStatusArray = NULL;
    m_rowArraySize = 1;
    return false;
  }

  // Bind columns
  if (!BindColumnsForBlockFetch()) {
    FreeBlockFetchBuffers();
    m_rowArraySize = 1;
    return false;
  }

  return true;
}

/*
 * BindColumnsForBlockFetch
 * Allocates array buffers for each column and calls SQLBindCol.
 */
bool ODBCResult::BindColumnsForBlockFetch()
{
  DEBUG_PRINTF("ODBCResult::BindColumnsForBlockFetch: colCount=%d, rowArraySize=%lu\n",
               colCount, (unsigned long)m_rowArraySize);

  m_boundCols = (BoundColumn *)calloc(colCount, sizeof(BoundColumn));
  if (!m_boundCols) return false;

  for (int i = 0; i < colCount; i++) {
    SQLSMALLINT cType;
    SQLLEN elemSize;

    switch (columns[i].type) {
      case SQL_INTEGER:
      case SQL_SMALLINT:
      case SQL_TINYINT:
        cType = SQL_C_SLONG;
        elemSize = sizeof(SQLINTEGER);
        break;

      case SQL_BIGINT:
        cType = SQL_C_CHAR;
        // BIGINT as string: max 20 digits + sign + null
        elemSize = 22;
        break;

      case SQL_FLOAT:
      case SQL_REAL:
      case SQL_DOUBLE:
      case SQL_NUMERIC:
      case SQL_DECIMAL:
      case SQL_DECFLOAT:
        cType = SQL_C_DOUBLE;
        elemSize = sizeof(double);
        break;

      case SQL_BIT:
        cType = SQL_C_CHAR;
        elemSize = 4;
        break;

      case SQL_DATETIME:
      case SQL_TIMESTAMP:
      case SQL_TYPE_TIMESTAMP:
#ifdef _WIN32
        cType = SQL_C_CHAR;
        elemSize = sizeof(SQL_TIMESTAMP_STRUCT);
#else
        cType = SQL_C_TYPE_TIMESTAMP;
        elemSize = sizeof(SQL_TIMESTAMP_STRUCT);
#endif
        break;

      case SQL_BLOB:
      case SQL_BINARY:
      case SQL_VARBINARY:
      case SQL_LONGVARBINARY:
        cType = SQL_C_BINARY;
        {
          SQLLEN colSize = columns[i].field_len;
          if (colSize <= 0 || colSize > BLOCK_FETCH_MAX_COL_SIZE)
            colSize = BLOCK_FETCH_MAX_COL_SIZE;
          elemSize = colSize;
        }
        break;

      default:
        // Treat as string (VARCHAR, CHAR, CLOB, etc.)
        cType = SQL_C_CHAR;
#ifdef UNICODE
        cType = SQL_C_WCHAR;
#endif
        {
          SQLLEN colSize = columns[i].field_len;
          if (colSize <= 0 || colSize > BLOCK_FETCH_MAX_COL_SIZE)
            colSize = BLOCK_FETCH_MAX_COL_SIZE;
#ifdef UNICODE
          elemSize = (colSize + 1) * sizeof(uint16_t);
#else
          elemSize = colSize + 1;  // +1 for null terminator
#endif
        }
        break;
    }

    m_boundCols[i].cType = cType;
    m_boundCols[i].elementSize = elemSize;

    // Allocate array buffer for N rows
    m_boundCols[i].data = calloc(m_rowArraySize, elemSize);
    if (!m_boundCols[i].data) {
      DEBUG_PRINTF("ODBCResult::BindColumnsForBlockFetch: Failed to allocate data buffer for col %d\n", i);
      return false;
    }

    // Allocate indicator array for N rows
    m_boundCols[i].indicators = (SQLLEN *)calloc(m_rowArraySize, sizeof(SQLLEN));
    if (!m_boundCols[i].indicators) {
      DEBUG_PRINTF("ODBCResult::BindColumnsForBlockFetch: Failed to allocate indicator buffer for col %d\n", i);
      return false;
    }

    SQLRETURN ret = SQLBindCol(m_hSTMT,
                               (SQLUSMALLINT)(i + 1),
                               cType,
                               m_boundCols[i].data,
                               elemSize,
                               m_boundCols[i].indicators);
    if (!SQL_SUCCEEDED(ret)) {
      DEBUG_PRINTF("ODBCResult::BindColumnsForBlockFetch: SQLBindCol failed for col %d, ret=%d\n", i, ret);
      return false;
    }
  }

  return true;
}

/*
 * FreeBlockFetchBuffers
 * Frees all memory allocated for block fetch.
 */
void ODBCResult::FreeBlockFetchBuffers()
{
  if (m_boundCols) {
    for (int i = 0; i < colCount; i++) {
      if (m_boundCols[i].data) {
        free(m_boundCols[i].data);
        m_boundCols[i].data = NULL;
      }
      if (m_boundCols[i].indicators) {
        free(m_boundCols[i].indicators);
        m_boundCols[i].indicators = NULL;
      }
    }
    free(m_boundCols);
    m_boundCols = NULL;
  }
  if (m_rowStatusArray) {
    free(m_rowStatusArray);
    m_rowStatusArray = NULL;
  }
  m_blockFetchInitialized = false;
  m_rowArraySize = 0;
}

/*
 * GetBoundColumnValue
 * Read value from bound buffer for a specific column and row.
 */
Local<Value> ODBCResult::GetBoundColumnValue(int colIndex, SQLULEN rowIndex)
{
  Nan::EscapableHandleScope scope;

  BoundColumn *bc = &m_boundCols[colIndex];
  SQLLEN indicator = bc->indicators[rowIndex];

  // NULL check
  if (indicator == SQL_NULL_DATA) {
    return scope.Escape(Nan::Null());
  }

  char *rowData = (char *)bc->data + (rowIndex * bc->elementSize);

  switch (columns[colIndex].type) {
    case SQL_INTEGER:
    case SQL_SMALLINT:
    case SQL_TINYINT:
    {
      SQLINTEGER value = *(SQLINTEGER *)rowData;
      return scope.Escape(Nan::New<Number>((int)value));
    }

    case SQL_BIGINT:
    {
      // BIGINT was fetched as string
      return scope.Escape(Nan::New((const char *)rowData).ToLocalChecked());
    }

    case SQL_FLOAT:
    case SQL_REAL:
    case SQL_DOUBLE:
    case SQL_NUMERIC:
    case SQL_DECIMAL:
    case SQL_DECFLOAT:
    {
      double value = *(double *)rowData;
      return scope.Escape(Nan::New<Number>(value));
    }

    case SQL_BIT:
    {
      return scope.Escape(Nan::New((*rowData == '0') ? false : true));
    }

    case SQL_DATETIME:
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIMESTAMP:
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
      return scope.Escape(Nan::New<Date>((double(timegm(&timeInfo)) * 1000) +
                                         (ts->fraction / 1000000))
                              .ToLocalChecked());
#else
      return scope.Escape(Nan::New<Date>((double(mktime(&timeInfo)) * 1000) +
                                         (ts->fraction / 1000000))
                              .ToLocalChecked());
#endif
    }

    case SQL_BLOB:
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
    {
      SQLLEN dataLen = indicator;
      if (dataLen > bc->elementSize) dataLen = bc->elementSize;
      return scope.Escape(Nan::CopyBuffer(rowData, (uint32_t)dataLen).ToLocalChecked());
    }

    default:
    {
      // String data
#ifdef UNICODE
      return scope.Escape(Nan::New((uint16_t *)rowData).ToLocalChecked());
#else
      return scope.Escape(Nan::New((const char *)rowData).ToLocalChecked());
#endif
    }
  }
}

/*
 * BlockFetchNextRow
 * Advances to the next valid row in the current block.
 * If the current block is exhausted, fetches a new block.
 * Sets *outRowIndex to the row index within the block.
 * Returns SQL_SUCCESS if a row is available, SQL_NO_DATA if done, SQL_ERROR on error.
 */
SQLRETURN ODBCResult::BlockFetchNextRow(SQLULEN *outRowIndex)
{
  if (m_blockExhausted) {
    return SQL_NO_DATA;
  }

  // Try to find the next valid row in the current block
  while (m_currentRowInBlock < m_rowsFetched) {
    SQLUSMALLINT status = m_rowStatusArray[m_currentRowInBlock];
    if (status == SQL_ROW_SUCCESS || status == SQL_ROW_SUCCESS_WITH_INFO) {
      *outRowIndex = m_currentRowInBlock;
      m_currentRowInBlock++;
      return SQL_SUCCESS;
    }
    m_currentRowInBlock++;
  }

  // Current block exhausted, fetch a new one
  m_rowsFetched = 0;
  m_currentRowInBlock = 0;
  SQLRETURN ret = SQLFetch(m_hSTMT);

  if (ret == SQL_NO_DATA) {
    m_blockExhausted = true;
    return SQL_NO_DATA;
  }
  if (ret == SQL_ERROR) {
    return SQL_ERROR;
  }

  DEBUG_PRINTF("ODBCResult::BlockFetchNextRow: fetched %lu rows\n",
               (unsigned long)m_rowsFetched);

  // Find the first valid row in the new block
  while (m_currentRowInBlock < m_rowsFetched) {
    SQLUSMALLINT status = m_rowStatusArray[m_currentRowInBlock];
    if (status == SQL_ROW_SUCCESS || status == SQL_ROW_SUCCESS_WITH_INFO) {
      *outRowIndex = m_currentRowInBlock;
      m_currentRowInBlock++;
      return SQL_SUCCESS;
    }
    m_currentRowInBlock++;
  }

  // All rows in this block were error/norow - try next block recursively
  return BlockFetchNextRow(outRowIndex);
}

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

  data->result = SQLFetch(data->objResult->m_hSTMT);
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
    if (data->fetchMode == FETCH_ARRAY)
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

    data->cb->Call(2, info);
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

    data->cb->Call(2, info);
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

  SQLRETURN ret = SQLFetch(objResult->m_hSTMT);

  if (objResult->colCount == 0)
  {
    objResult->columns = ODBC::GetColumns(
        objResult->m_hSTMT,
        &objResult->colCount);
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

    if (fetchMode == FETCH_ARRAY)
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

  data->result = SQLFetch(data->objResult->m_hSTMT);
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
    if (data->fetchMode == FETCH_ARRAY)
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

    data->cb->Call(3, info);
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
    // loop through all records
    while (true)
    {
      ret = SQLFetch(self->m_hSTMT);

      // check to see if there was an error
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

      // check to see if we are at the end of the recordset
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

  data->cb->Call(2, info);
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
    data->cb->Call(1, info);

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

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

void ODBCResult::Init(v8::Handle<Object> exports)
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

  Nan::SetPrototypeMethod(constructor_template, "moreResultsSync", MoreResultsSync);
  Nan::SetPrototypeMethod(constructor_template, "closeSync", CloseSync);
  Nan::SetPrototypeMethod(constructor_template, "fetchSync", FetchSync);
  Nan::SetPrototypeMethod(constructor_template, "fetchAllSync", FetchAllSync);
  Nan::SetPrototypeMethod(constructor_template, "getColumnNamesSync", GetColumnNamesSync);
  Nan::SetPrototypeMethod(constructor_template, "getColumnMetadataSync", GetColumnMetadataSync);
  Nan::SetPrototypeMethod(constructor_template, "getSQLErrorSync", GetSQLErrorSync);

  // Properties
  OPTION_FETCH_MODE.Reset(Nan::New("fetchMode").ToLocalChecked());
  Nan::SetAccessor(instance_template, Nan::New("fetchMode").ToLocalChecked(), FetchModeGetter, FetchModeSetter);
  
  // Attach the Database Constructor to the target object
  constructor.Reset(constructor_template->GetFunction());
  exports->Set(Nan::New("ODBCResult").ToLocalChecked(),
               constructor_template->GetFunction());
}

ODBCResult::~ODBCResult()
{
  DEBUG_PRINTF("ODBCResult::~ODBCResult m_hSTMT=%x\n", m_hSTMT);
  this->Free();
}

void ODBCResult::Free()
{
  DEBUG_PRINTF("ODBCResult::Free m_hSTMT=%X m_canFreeHandle=%X\n", m_hSTMT, m_canFreeHandle);
  
  if (m_hSTMT && m_canFreeHandle) {
    SQLFreeHandle( SQL_HANDLE_STMT, m_hSTMT);
    m_hSTMT = (SQLHSTMT)NULL;
  }
  
  if (bufferLength > 0) {
    bufferLength = 0;
    free(buffer);
    buffer = NULL;
  }
  DEBUG_PRINTF("ODBCResult::Free() Done.\n");
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
  bool* canFreeHandle = static_cast<bool *>(js_canFreeHandle->Value());
  
  //create a new OBCResult object
  ODBCResult* objODBCResult = new ODBCResult(hENV, hDBC, hSTMT, *canFreeHandle);
  
  DEBUG_PRINTF("ODBCResult::New m_hENV=%X m_hDBC=%X m_hSTMT=%X canFreeHandle=%X\n",
    objODBCResult->m_hENV,
    objODBCResult->m_hDBC,
    objODBCResult->m_hSTMT,
    objODBCResult->m_canFreeHandle
  );
  
  //free the pointer to canFreeHandle
  delete canFreeHandle;

  //specify the buffer length
  objODBCResult->bufferLength = MAX_VALUE_SIZE;
  
  //initialze a buffer for this object
  objODBCResult->buffer = (uint16_t *) malloc(objODBCResult->bufferLength+2);
  MEMCHECK( objODBCResult->buffer );

  //set the initial colCount to 0
  objODBCResult->colCount = 0;

  //default fetchMode to FETCH_OBJECT
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
  
  if (value->IsNumber()) {
    obj->m_fetchMode = value->Int32Value();
  }
}

/*
 * Fetch
 */

NAN_METHOD(ODBCResult::Fetch)
{
  DEBUG_PRINTF("ODBCResult::Fetch\n");
  Nan::HandleScope scope;
  
  ODBCResult* objODBCResult = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  MEMCHECK( work_req );
  
  fetch_work_data* data = (fetch_work_data *) calloc(1, sizeof(fetch_work_data));
  MEMCHECK( data );
  
  Local<Function> cb;
   
  //set the fetch mode to the default of this instance
  data->fetchMode = objODBCResult->m_fetchMode;
  
  if (info.Length() == 1 && info[0]->IsFunction()) {
    cb = Local<Function>::Cast(info[0]);
  }
  else if (info.Length() == 2 && info[0]->IsObject() && info[1]->IsFunction()) {
    cb = Local<Function>::Cast(info[1]);  
    
    Local<Object> obj = info[0]->ToObject();
    
    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (obj->Has(fetchModeKey) && obj->Get(fetchModeKey)->IsInt32()) {
      data->fetchMode = Nan::To<Uint32>(obj->Get(fetchModeKey)).ToLocalChecked()->Value();
    }
  }
  else {
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

void ODBCResult::UV_Fetch(uv_work_t* work_req)
{
  DEBUG_PRINTF("ODBCResult::UV_Fetch\n");
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  data->result = SQLFetch(data->objResult->m_hSTMT);
}

void ODBCResult::UV_AfterFetch(uv_work_t* work_req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterFetch\n");
  Nan::HandleScope scope;
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  SQLRETURN ret = data->result;
  //TODO: we should probably define this on the work data so we
  //don't have to keep creating it?
  Local<Value> objError;
  bool moreWork = true;
  bool error = false;
  
  if (data->objResult->colCount == 0) {
    data->objResult->columns = ODBC::GetColumns(
      data->objResult->m_hSTMT, 
      &data->objResult->colCount);
  }
  
  //check to see if the result has no columns
  if (data->objResult->colCount == 0) {
    //this means
    moreWork = false;
  }
  //check to see if there was an error
  else if (ret == SQL_ERROR)  {
    moreWork = false;
    error = true;
    
    objError = ODBC::GetSQLError(
      SQL_HANDLE_STMT, 
      data->objResult->m_hSTMT,
      (char *) "Error in ODBCResult::UV_AfterFetch");
  }
  //check to see if we are at the end of the recordset
  else if (ret == SQL_NO_DATA) {
    moreWork = false;
  }

  if (moreWork) {
    Local<Value> info[2];

    info[0] = Nan::Null();
    if (data->fetchMode == FETCH_ARRAY) {
      info[1] = ODBC::GetRecordArray(
        data->objResult->m_hSTMT,
        data->objResult->columns,
        &data->objResult->colCount,
        data->objResult->buffer,
        data->objResult->bufferLength);
    }
    else {
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

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
  }
  else {
    ODBC::FreeColumns(data->objResult->columns, &data->objResult->colCount);
    
    Local<Value> info[2];
    
    //if there was an error, pass that as arg[0] otherwise Null
    if (error) {
      info[0] = objError;
    }
    else {
      info[0] = Nan::Null();
    }
    
    info[1] = Nan::Null();

    Nan::TryCatch try_catch;

    data->cb->Call(2, info);
    delete data->cb;

    if (try_catch.HasCaught()) {
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
  
  ODBCResult* objResult = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  Local<Value> objError;
  bool moreWork = true;
  bool error = false;
  int fetchMode = objResult->m_fetchMode;
  
  if (info.Length() == 1 && info[0]->IsObject()) {
    Local<Object> obj = info[0]->ToObject();
    
    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (obj->Has(fetchModeKey) && obj->Get(fetchModeKey)->IsInt32()) {
      fetchMode = Nan::To<Uint32>(obj->Get(fetchModeKey)).ToLocalChecked()->Value();
    }
  }
  
  SQLRETURN ret = SQLFetch(objResult->m_hSTMT);

  if (objResult->colCount == 0) {
    objResult->columns = ODBC::GetColumns(
      objResult->m_hSTMT, 
      &objResult->colCount);
  }
  
  //check to see if the result has no columns
  if (objResult->colCount == 0) {
    moreWork = false;
  }
  //check to see if there was an error
  else if (ret == SQL_ERROR)  {
    moreWork = false;
    error = true;
    
    objError = ODBC::GetSQLError(
      SQL_HANDLE_STMT, 
      objResult->m_hSTMT,
      (char *) "Error in ODBCResult::UV_AfterFetch");
  }
  //check to see if we are at the end of the recordset
  else if (ret == SQL_NO_DATA) {
    moreWork = false;
  }

  if (moreWork) {
    Local<Value> data;
    
    if (fetchMode == FETCH_ARRAY) {
      data = ODBC::GetRecordArray(
        objResult->m_hSTMT,
        objResult->columns,
        &objResult->colCount,
        objResult->buffer,
        objResult->bufferLength);
    }
    else {
      data = ODBC::GetRecordTuple(
        objResult->m_hSTMT,
        objResult->columns,
        &objResult->colCount,
        objResult->buffer,
        objResult->bufferLength);
    }
    
    info.GetReturnValue().Set(data);
  }
  else {
    ODBC::FreeColumns(objResult->columns, &objResult->colCount);

    //if there was an error, pass that as arg[0] otherwise Null
    if (error) {
      Nan::ThrowError(objError);
      
      info.GetReturnValue().Set(Nan::Null());
    }
    else {
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
  
  ODBCResult* objODBCResult = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  MEMCHECK( work_req );
  
  fetch_work_data* data = (fetch_work_data *) calloc(1, sizeof(fetch_work_data));
  MEMCHECK( data );
  
  Local<Function> cb;
  
  data->fetchMode = objODBCResult->m_fetchMode;
  
  if (info.Length() == 1 && info[0]->IsFunction()) {
    cb = Local<Function>::Cast(info[0]);
  }
  else if (info.Length() == 2 && info[0]->IsObject() && info[1]->IsFunction()) {
    cb = Local<Function>::Cast(info[1]);  
    
    Local<Object> obj = info[0]->ToObject();
    
    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (obj->Has(fetchModeKey) && obj->Get(fetchModeKey)->IsInt32()) {
      data->fetchMode = Nan::To<Uint32>(obj->Get(fetchModeKey)).ToLocalChecked()->Value();
    }
  }
  else {
    Nan::ThrowTypeError("ODBCResult::FetchAll(): 1 or 2 arguments are required. The last argument must be a callback function.");
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

void ODBCResult::UV_FetchAll(uv_work_t* work_req)
{
  DEBUG_PRINTF("ODBCResult::UV_FetchAll - Entry\n");
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  data->result = SQLFetch(data->objResult->m_hSTMT);
  DEBUG_PRINTF("ODBCResult::UV_FetchAll - Exit, return code = %d for stmt %X\n", data->result, data->objResult->m_hSTMT);
 }

void ODBCResult::UV_AfterFetchAll(uv_work_t* work_req, int status)
{
  DEBUG_PRINTF("ODBCResult::UV_AfterFetchAll - Entry\n");
  Nan::HandleScope scope;
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  ODBCResult* self = data->objResult->self();
  
  bool doMoreWork = true;
  
  /* Check : to see if there was an ERROR on SQLFetch() call.
   * So before GetColums call we should store the error.
   * Reason : GetColumns internally calls SQLGetDiagField method,
   * and SQLGetDiagField() method retrieves only the diagnostic information of
   * most recent CLI function call, any diagnostic information from a previous call 
   * with the same handle will be lost. - issue253
  */ 
  if (data->result == SQL_ERROR) {
    data->errorCount++;
    data->objError.Reset(ODBC::GetSQLError(
      SQL_HANDLE_STMT,
      self->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCResult::UV_AfterFetchAll"
    ));
  }
  
  if (self->colCount == 0) {
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

  if (self->colCount == 0) {
    noDataFetchQuery = true;
    doMoreWork = false;
  }
  //check to see if we are at the end of the recordset
  else if (data->result == SQL_NO_DATA) {
    doMoreWork = false;
  }
  //check to see if there was an error
  else if (data->result == SQL_ERROR)  {
    doMoreWork = false;
  }
  
  else {
    Local<Array> rows = Nan::New(data->rows);
    if (data->fetchMode == FETCH_ARRAY) {
      rows->Set(
        Nan::New(data->count), 
        ODBC::GetRecordArray(
          self->m_hSTMT,
          self->columns,
          &self->colCount,
          self->buffer,
          self->bufferLength)
      );
    }
    else {
      rows->Set(
        Nan::New(data->count), 
        ODBC::GetRecordTuple(
          self->m_hSTMT,
          self->columns,
          &self->colCount,
          self->buffer,
          self->bufferLength)
      );
    }
    data->count++;
  }
  if (doMoreWork) {
    //Go back to the thread pool and fetch more data!
    uv_queue_work( uv_default_loop(),
                   work_req,
                   UV_FetchAll,
                   (uv_after_work_cb)UV_AfterFetchAll);
  }
  else {
    DEBUG_PRINTF("ODBCResult::UV_AfterFetchAll Done for stmt %X\n", data->objResult->m_hSTMT);
    
    Local<Value> info[3];
    
    if (data->errorCount > 0) {
      if(noDataFetchQuery) {
        info[0] = Nan::Null();
        noDataFetchQuery = false;
      }
      else {
        info[0] = Nan::New(data->objError);
      }
    }
    else {
      info[0] = Nan::Null();
    }
    
    info[1] = Nan::New(data->rows);
    info[2] = Nan::New(self->colCount);
    Nan::TryCatch try_catch;

    data->cb->Call(3, info);
    ODBC::FreeColumns(self->columns, &self->colCount);
    delete data->cb;
    data->rows.Reset();
    data->objError.Reset();

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

    //TODO: Do we need to free self->rows somehow?
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
  
  ODBCResult* self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());
  
  Local<Value> objError = Nan::New<Object>();
  
  SQLRETURN ret;
  int count = 0;
  int errorCount = 0;
  int fetchMode = self->m_fetchMode;

  if (info.Length() == 1 && info[0]->IsObject()) {
    Local<Object> obj = info[0]->ToObject();
    
    Local<String> fetchModeKey = Nan::New<String>(OPTION_FETCH_MODE);
    if (obj->Has(fetchModeKey) && obj->Get(fetchModeKey)->IsInt32()) {
      fetchMode = Nan::To<Uint32>(obj->Get(fetchModeKey)).ToLocalChecked()->Value();
    }
  }
  
  if (self->colCount == 0) {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }
  
  DEBUG_PRINTF("ODBCResult::FetchAllSync colCount = %i, fetchMode = %i\n", self->colCount, fetchMode);

  Local<Array> rows = Nan::New<Array>();
  
  //Only loop through the recordset if there are columns
  if (self->colCount > 0) {
    //loop through all records
    while (true) 
    {
      ret = SQLFetch(self->m_hSTMT);
      
      //check to see if there was an error
      if (ret == SQL_ERROR)  
      {
        errorCount++;
        objError = ODBC::GetSQLError(
          SQL_HANDLE_STMT, 
          self->m_hSTMT,
          (char *) "[node-odbc] Error in ODBCResult::UV_AfterFetchAll; probably"
            " your query did not have a result set."
        );
        break;
      }
      
      //check to see if we are at the end of the recordset
      if (ret == SQL_NO_DATA) {
        ODBC::FreeColumns(self->columns, &self->colCount);
        
        break;
      }

      if (fetchMode == FETCH_ARRAY) {
        rows->Set(
          Nan::New(count), 
          ODBC::GetRecordArray(
            self->m_hSTMT,
            self->columns,
            &self->colCount,
            self->buffer,
            self->bufferLength)
        );
      }
      else {
        rows->Set(
          Nan::New(count), 
          ODBC::GetRecordTuple(
            self->m_hSTMT,
            self->columns,
            &self->colCount,
            self->buffer,
            self->bufferLength)
        );
      }
      count++;
    }
  }
  else {
    ODBC::FreeColumns(self->columns, &self->colCount);
  }
  
  //throw the error object if there were errors
  if (errorCount > 0) {
    Nan::ThrowError(objError);
  }
  
  info.GetReturnValue().Set(rows);
  DEBUG_PRINTF("ODBCResult::FetchAllSync() -Exit\n");
}

/*
 * CloseSync
 * 
 */

NAN_METHOD(ODBCResult::CloseSync)
{
  DEBUG_PRINTF("ODBCResult::CloseSync - Entry\n");
  Nan::HandleScope scope;
  
  OPT_INT_ARG(0, closeOption, SQL_DESTROY);
  
  ODBCResult* result = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());
 
  DEBUG_PRINTF("ODBCResult::CloseSync closeOption=%i m_canFreeHandle=%i, hSTMT=%X\n", 
               closeOption, result->m_canFreeHandle,result->m_hSTMT);
  
  if (closeOption == SQL_DESTROY && result->m_canFreeHandle) {
    result->Free();
  }
  else if (closeOption == SQL_DESTROY && !result->m_canFreeHandle) {
    //We technically can't free the handle so, we'll SQL_CLOSE
    // Don't set result->m_canFreeHandle to true in this function.
    // Handle would be freed by the call of ODBCStatement.Close().
    SQLFreeStmt(result->m_hSTMT, SQL_CLOSE);
  }
  else {
    SQLFreeStmt(result->m_hSTMT, closeOption);
  }
  
  info.GetReturnValue().Set(Nan::True());
  DEBUG_PRINTF("ODBCResult::CloseSync - Exit\n");
}

NAN_METHOD(ODBCResult::MoreResultsSync)
{
  DEBUG_PRINTF("ODBCResult::MoreResultsSync\n");
  Nan::HandleScope scope;
  
  ODBCResult* result = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());
  
  SQLRETURN ret = SQLMoreResults(result->m_hSTMT);

  if (ret == SQL_ERROR) {
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
  
  ODBCResult* self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());
  
  Local<Array> cols = Nan::New<Array>();
  
  if (self->colCount == 0) {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }
  
  for (int i = 0; i < self->colCount; i++) {
    cols->Set(Nan::New(i),
              Nan::New((const char *) self->columns[i].name).ToLocalChecked());
  }
    
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

  ODBCResult* self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());

  if (self->colCount == 0) {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }

  Local<Array> columns = Nan::New<Array>();

  for (int i = 0; i < self->colCount; i ++) {
    Local<Object> col = Nan::New<Object>();
    col->Set(Nan::New("index").ToLocalChecked(), Nan::New(self->columns[i].index));
    col->Set(Nan::New("SQL_DESC_CONCISE_TYPE").ToLocalChecked(), Nan::New((const char *) self->columns[i].name).ToLocalChecked());
    col->Set(Nan::New("SQL_DESC_TYPE_NAME").ToLocalChecked(), Nan::New((const char *) self->columns[i].type_name).ToLocalChecked());
    col->Set(Nan::New("SQL_DESC_DISPLAY_SIZE").ToLocalChecked(), Nan::New(self->columns[i].max_display_len));
    col->Set(Nan::New("SQL_DESC_PRECISION").ToLocalChecked(), Nan::New(self->columns[i].precision));
    col->Set(Nan::New("SQL_DESC_SCALE").ToLocalChecked(), Nan::New(self->columns[i].scale));
    col->Set(Nan::New("SQL_DESC_LENGTH").ToLocalChecked(), Nan::New(self->columns[i].field_len));

    columns->Set(Nan::New(i), col);
  }

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

  ODBCResult* self = Nan::ObjectWrap::Unwrap<ODBCResult>(info.Holder());
  Local<Value> objError = Nan::New<Object>();

  objError = ODBC::GetSQLError(SQL_HANDLE_STMT, self->m_hSTMT, (char *) "");

  info.GetReturnValue().Set(objError);
  DEBUG_PRINTF("ODBCResult::GetSQLErrorSync - Exit\n");
}

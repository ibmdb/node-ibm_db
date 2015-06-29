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

Persistent<Function> ODBCResult::constructor;
Persistent<String> ODBCResult::OPTION_FETCH_MODE;

void ODBCResult::Init(v8::Handle<Object> exports) {
  DEBUG_PRINTF("ODBCResult::Init\n");
  NanScope();

  Local<FunctionTemplate> constructor_template = NanNew<FunctionTemplate>(New);

  // Constructor Template
  constructor_template->SetClassName(NanNew("ODBCResult"));

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = constructor_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);
  
  // Prototype Methods
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "fetchAll", FetchAll);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "fetch", Fetch);

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "moreResultsSync", MoreResultsSync);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "closeSync", CloseSync);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "fetchSync", FetchSync);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "fetchAllSync", FetchAllSync);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "getColumnNamesSync", GetColumnNamesSync);

  // Properties
  NanAssignPersistent(OPTION_FETCH_MODE, NanNew("fetchMode"));
  instance_template->SetAccessor(NanNew("fetchMode"), FetchModeGetter, FetchModeSetter);
  
  // Attach the Database Constructor to the target object
  NanAssignPersistent(constructor, constructor_template->GetFunction());
  exports->Set(NanNew("ODBCResult"),
               constructor_template->GetFunction());
}

ODBCResult::~ODBCResult() {
  DEBUG_PRINTF("ODBCResult::~ODBCResult m_hSTMT=%x\n", m_hSTMT);
  this->Free();
}

void ODBCResult::Free() {
  DEBUG_PRINTF("ODBCResult::Free m_hSTMT=%X m_canFreeHandle=%X\n", m_hSTMT, m_canFreeHandle);
  
  if (m_hSTMT && m_canFreeHandle) {
    uv_mutex_lock(&ODBC::g_odbcMutex);
    
    SQLFreeHandle( SQL_HANDLE_STMT, m_hSTMT);
    
    m_hSTMT = NULL;
  
    uv_mutex_unlock(&ODBC::g_odbcMutex);
  }
  
  if (bufferLength > 0) {
    bufferLength = 0;
    free(buffer);
  }
}

NAN_METHOD(ODBCResult::New) {
  DEBUG_PRINTF("ODBCResult::New\n");
  NanScope();
  
  REQ_EXT_ARG(0, js_henv);
  REQ_EXT_ARG(1, js_hdbc);
  REQ_EXT_ARG(2, js_hstmt);
  REQ_EXT_ARG(3, js_canFreeHandle);
  
  HENV hENV = static_cast<HENV>(js_henv->Value());
  HDBC hDBC = static_cast<HDBC>(js_hdbc->Value());
  HSTMT hSTMT = static_cast<HSTMT>(js_hstmt->Value());
  bool* canFreeHandle = static_cast<bool *>(js_canFreeHandle->Value());
  
  //create a new OBCResult object
  ODBCResult* objODBCResult = new ODBCResult(hENV, hDBC, hSTMT, *canFreeHandle);
  
  DEBUG_PRINTF("ODBCResult::New m_hDBC=%X m_hDBC=%X m_hSTMT=%X canFreeHandle=%X\n",
    objODBCResult->m_hENV,
    objODBCResult->m_hDBC,
    objODBCResult->m_hSTMT,
    objODBCResult->m_canFreeHandle
  );
  
  //free the pointer to canFreeHandle
  delete canFreeHandle;

  //specify the buffer length
  objODBCResult->bufferLength = MAX_VALUE_SIZE - 1;
  
  //initialze a buffer for this object
  objODBCResult->buffer = (uint16_t *) malloc(objODBCResult->bufferLength + 1);
  //TODO: make sure the malloc succeeded

  //set the initial colCount to 0
  objODBCResult->colCount = 0;

  //default fetchMode to FETCH_OBJECT
  objODBCResult->m_fetchMode = FETCH_OBJECT;
  
  objODBCResult->Wrap(args.Holder());
  
  NanReturnValue(args.Holder());
}

NAN_GETTER(ODBCResult::FetchModeGetter) {
  NanScope();

  ODBCResult *obj = ObjectWrap::Unwrap<ODBCResult>(args.Holder());

  NanReturnValue(NanNew(obj->m_fetchMode));
}

NAN_SETTER(ODBCResult::FetchModeSetter) {
  NanScope();

  ODBCResult *obj = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  if (value->IsNumber()) {
    obj->m_fetchMode = value->Int32Value();
  }
}

/*
 * Fetch
 */

NAN_METHOD(ODBCResult::Fetch) {
  DEBUG_PRINTF("ODBCResult::Fetch\n");
  NanScope();
  
  ODBCResult* objODBCResult = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  fetch_work_data* data = (fetch_work_data *) calloc(1, sizeof(fetch_work_data));
  
  Local<Function> cb;
   
  //set the fetch mode to the default of this instance
  data->fetchMode = objODBCResult->m_fetchMode;
  
  if (args.Length() == 1 && args[0]->IsFunction()) {
    cb = Local<Function>::Cast(args[0]);
  }
  else if (args.Length() == 2 && args[0]->IsObject() && args[1]->IsFunction()) {
    cb = Local<Function>::Cast(args[1]);  
    
    Local<Object> obj = args[0]->ToObject();
    
    Local<String> fetchModeKey = NanNew<String>(OPTION_FETCH_MODE);
    if (obj->Has(fetchModeKey) && obj->Get(fetchModeKey)->IsInt32()) {
      data->fetchMode = obj->Get(fetchModeKey)->ToInt32()->Value();
    }
  }
  else {
    return NanThrowTypeError("ODBCResult::Fetch(): 1 or 2 arguments are required. The last argument must be a callback function.");
  }
  
  data->cb = new NanCallback(cb);
  
  data->objResult = objODBCResult;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(), 
    work_req, 
    UV_Fetch, 
    (uv_after_work_cb)UV_AfterFetch);

  objODBCResult->Ref();

  NanReturnValue(NanUndefined());
}

void ODBCResult::UV_Fetch(uv_work_t* work_req) {
  DEBUG_PRINTF("ODBCResult::UV_Fetch\n");
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  data->result = SQLFetch(data->objResult->m_hSTMT);
}

void ODBCResult::UV_AfterFetch(uv_work_t* work_req, int status) {
  DEBUG_PRINTF("ODBCResult::UV_AfterFetch\n");
  NanScope();
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  SQLRETURN ret = data->result;
  //TODO: we should probably define this on the work data so we
  //don't have to keep creating it?
  Local<Object> objError;
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
    Handle<Value> args[2];

    args[0] = NanNull();
    if (data->fetchMode == FETCH_ARRAY) {
      args[1] = ODBC::GetRecordArray(
        data->objResult->m_hSTMT,
        data->objResult->columns,
        &data->objResult->colCount,
        data->objResult->buffer,
        data->objResult->bufferLength);
    }
    else {
      args[1] = ODBC::GetRecordTuple(
        data->objResult->m_hSTMT,
        data->objResult->columns,
        &data->objResult->colCount,
        data->objResult->buffer,
        data->objResult->bufferLength);
    }

    TryCatch try_catch;

    data->cb->Call(2, args);
    delete data->cb;

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
  }
  else {
    ODBC::FreeColumns(data->objResult->columns, &data->objResult->colCount);
    
    Handle<Value> args[2];
    
    //if there was an error, pass that as arg[0] otherwise Null
    if (error) {
      args[0] = objError;
    }
    else {
      args[0] = NanNull();
    }
    
    args[1] = NanNull();

    TryCatch try_catch;

    data->cb->Call(2, args);
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

NAN_METHOD(ODBCResult::FetchSync) {
  DEBUG_PRINTF("ODBCResult::FetchSync\n");
  NanScope();
  
  ODBCResult* objResult = ObjectWrap::Unwrap<ODBCResult>(args.Holder());

  Local<Object> objError;
  bool moreWork = true;
  bool error = false;
  int fetchMode = objResult->m_fetchMode;
  
  if (args.Length() == 1 && args[0]->IsObject()) {
    Local<Object> obj = args[0]->ToObject();
    
    Local<String> fetchModeKey = NanNew<String>(OPTION_FETCH_MODE);
    if (obj->Has(fetchModeKey) && obj->Get(fetchModeKey)->IsInt32()) {
      fetchMode = obj->Get(fetchModeKey)->ToInt32()->Value();
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
    Handle<Value> data;
    
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
    
    NanReturnValue(data);
  }
  else {
    ODBC::FreeColumns(objResult->columns, &objResult->colCount);

    //if there was an error, pass that as arg[0] otherwise Null
    if (error) {
      NanThrowError(objError);
      
      NanReturnValue(NanNull());
    }
    else {
      NanReturnValue(NanNull());
    }
  }
}

/*
 * FetchAll
 */

NAN_METHOD(ODBCResult::FetchAll) {
  DEBUG_PRINTF("ODBCResult::FetchAll\n");
  NanScope();
  
  ODBCResult* objODBCResult = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  fetch_work_data* data = (fetch_work_data *) calloc(1, sizeof(fetch_work_data));
  
  Local<Function> cb;
  
  data->fetchMode = objODBCResult->m_fetchMode;
  
  if (args.Length() == 1 && args[0]->IsFunction()) {
    cb = Local<Function>::Cast(args[0]);
  }
  else if (args.Length() == 2 && args[0]->IsObject() && args[1]->IsFunction()) {
    cb = Local<Function>::Cast(args[1]);  
    
    Local<Object> obj = args[0]->ToObject();
    
    Local<String> fetchModeKey = NanNew<String>(OPTION_FETCH_MODE);
    if (obj->Has(fetchModeKey) && obj->Get(fetchModeKey)->IsInt32()) {
      data->fetchMode = obj->Get(fetchModeKey)->ToInt32()->Value();
    }
  }
  else {
    NanThrowTypeError("ODBCResult::FetchAll(): 1 or 2 arguments are required. The last argument must be a callback function.");
  }
  
  NanAssignPersistent(data->rows, NanNew<Array>());
  data->errorCount = 0;
  data->count = 0;
  NanAssignPersistent(data->objError, NanNew<Object>());
  
  data->cb = new NanCallback(cb);
  data->objResult = objODBCResult;
  
  work_req->data = data;
  
  uv_queue_work(uv_default_loop(),
    work_req, 
    UV_FetchAll, 
    (uv_after_work_cb)UV_AfterFetchAll);

  data->objResult->Ref();

  NanReturnValue(NanUndefined());
}

void ODBCResult::UV_FetchAll(uv_work_t* work_req) {
  DEBUG_PRINTF("ODBCResult::UV_FetchAll\n");
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  data->result = SQLFetch(data->objResult->m_hSTMT);
 }

void ODBCResult::UV_AfterFetchAll(uv_work_t* work_req, int status) {
  DEBUG_PRINTF("ODBCResult::UV_AfterFetchAll\n");
  NanScope();
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  ODBCResult* self = data->objResult->self();
  
  bool doMoreWork = true;
  
  if (self->colCount == 0) {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }
  
  //check to see if the result set has columns
  if (self->colCount == 0) {
    //this most likely means that the query was something like
    //'insert into ....'
    doMoreWork = false;
  }
  //check to see if there was an error
  else if (data->result == SQL_ERROR)  {
    data->errorCount++;
    
    NanAssignPersistent(data->objError, ODBC::GetSQLError(
      SQL_HANDLE_STMT, 
      self->m_hSTMT,
      (char *) "[node-odbc] Error in ODBCResult::UV_AfterFetchAll"
    ));
    
    doMoreWork = false;
  }
  //check to see if we are at the end of the recordset
  else if (data->result == SQL_NO_DATA) {
    doMoreWork = false;
  }
  else {
    //TODO: !important: persistent forces us to set this to a local handle, but do we need to recopy it back to persistent handle?
    Local<Array> rows = NanNew(data->rows);
    if (data->fetchMode == FETCH_ARRAY) {
      rows->Set(
        NanNew(data->count), 
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
        NanNew(data->count), 
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
    uv_queue_work(
      uv_default_loop(),
      work_req, 
      UV_FetchAll, 
      (uv_after_work_cb)UV_AfterFetchAll);
  }
  else {
    ODBC::FreeColumns(self->columns, &self->colCount);
    
    Handle<Value> args[2];
    
    if (data->errorCount > 0) {
      args[0] = NanNew(data->objError);
    }
    else {
      args[0] = NanNull();
    }
    
    args[1] = NanNew(data->rows);

    TryCatch try_catch;

    data->cb->Call(2, args);
    delete data->cb;
    NanDisposePersistent(data->rows);
    NanDisposePersistent(data->objError);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

    //TODO: Do we need to free self->rows somehow?
    free(data);
    free(work_req);

    self->Unref(); 
  }
}

/*
 * FetchAllSync
 */

NAN_METHOD(ODBCResult::FetchAllSync) {
  DEBUG_PRINTF("ODBCResult::FetchAllSync\n");
  NanScope();
  
  ODBCResult* self = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  Local<Object> objError = NanNew<Object>();
  
  SQLRETURN ret;
  int count = 0;
  int errorCount = 0;
  int fetchMode = self->m_fetchMode;

  if (args.Length() == 1 && args[0]->IsObject()) {
    Local<Object> obj = args[0]->ToObject();
    
    Local<String> fetchModeKey = NanNew<String>(OPTION_FETCH_MODE);
    if (obj->Has(fetchModeKey) && obj->Get(fetchModeKey)->IsInt32()) {
      fetchMode = obj->Get(fetchModeKey)->ToInt32()->Value();
    }
  }
  
  if (self->colCount == 0) {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }
  
  Local<Array> rows = NanNew<Array>();
  
  //Only loop through the recordset if there are columns
  if (self->colCount > 0) {
    //loop through all records
    while (true) {
      ret = SQLFetch(self->m_hSTMT);
      
      //check to see if there was an error
      if (ret == SQL_ERROR)  {
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
          NanNew(count), 
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
          NanNew(count), 
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
    NanThrowError(objError);
  }
  
  NanReturnValue(rows);
}

/*
 * CloseSync
 * 
 */

NAN_METHOD(ODBCResult::CloseSync) {
  DEBUG_PRINTF("ODBCResult::CloseSync\n");
  NanScope();
  
  OPT_INT_ARG(0, closeOption, SQL_DESTROY);
  
  ODBCResult* result = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
 
  DEBUG_PRINTF("ODBCResult::CloseSync closeOption=%i m_canFreeHandle=%i\n", 
               closeOption, result->m_canFreeHandle);
  
  if (closeOption == SQL_DESTROY && result->m_canFreeHandle) {
    result->Free();
  }
  else if (closeOption == SQL_DESTROY && !result->m_canFreeHandle) {
    //We technically can't free the handle so, we'll SQL_CLOSE
    uv_mutex_lock(&ODBC::g_odbcMutex);
    
    SQLFreeStmt(result->m_hSTMT, SQL_CLOSE);
  
    uv_mutex_unlock(&ODBC::g_odbcMutex);
  }
  else {
    uv_mutex_lock(&ODBC::g_odbcMutex);
    
    SQLFreeStmt(result->m_hSTMT, closeOption);
  
    uv_mutex_unlock(&ODBC::g_odbcMutex);
  }
  
  NanReturnValue(NanTrue());
}

NAN_METHOD(ODBCResult::MoreResultsSync) {
  DEBUG_PRINTF("ODBCResult::MoreResultsSync\n");
  NanScope();
  
  ODBCResult* result = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  SQLRETURN ret = SQLMoreResults(result->m_hSTMT);

  if (ret == SQL_ERROR) {
    NanThrowError(ODBC::GetSQLError(SQL_HANDLE_STMT, result->m_hSTMT, (char *)"[node-odbc] Error in ODBCResult::MoreResultsSync"));
  }

  NanReturnValue(SQL_SUCCEEDED(ret) || ret == SQL_ERROR ? NanTrue() : NanFalse());
}

/*
 * GetColumnNamesSync
 */

NAN_METHOD(ODBCResult::GetColumnNamesSync) {
  DEBUG_PRINTF("ODBCResult::GetColumnNamesSync\n");
  NanScope();
  
  ODBCResult* self = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  Local<Array> cols = NanNew<Array>();
  
  if (self->colCount == 0) {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }
  
  for (int i = 0; i < self->colCount; i++) {
    cols->Set(NanNew(i),
              NanNew((const char *) self->columns[i].name));
  }
    
  NanReturnValue(cols);
}

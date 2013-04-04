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

Persistent<FunctionTemplate> ODBCResult::constructor_template;

void ODBCResult::Init(v8::Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  // Constructor Template
  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->SetClassName(String::NewSymbol("ODBCResult"));

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = constructor_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);
  
  // Prototype Methods
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "fetchAll", FetchAll);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "fetch", Fetch);

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "moreResultsSync", MoreResultsSync);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "closeSync", CloseSync);

  // Attach the Database Constructor to the target object
  target->Set( v8::String::NewSymbol("ODBCResult"),
               constructor_template->GetFunction());
  
  scope.Close(Undefined());
}

ODBCResult::~ODBCResult() {
  this->Free();
}

void ODBCResult::Free() {
  DEBUG_PRINTF("ODBCResult::Free\n");
  
  if (m_hSTMT) {
    uv_mutex_lock(&ODBC::g_odbcMutex);
    
    //This doesn't actually deallocate the statement handle
    //that should not be done by the result object; that should
    //be done by the statement object
    SQLFreeStmt(m_hSTMT, SQL_CLOSE);
    m_hSTMT = NULL;
    
    uv_mutex_unlock(&ODBC::g_odbcMutex);
    
    if (bufferLength > 0) {
      free(buffer);
    }
  }
}

Handle<Value> ODBCResult::New(const Arguments& args) {
  DEBUG_PRINTF("ODBCResult::New\n");
  
  HandleScope scope;
  
  REQ_EXT_ARG(0, js_henv);
  REQ_EXT_ARG(1, js_hdbc);
  REQ_EXT_ARG(2, js_hstmt);
  
  HENV hENV = static_cast<HENV>(js_henv->Value());
  HDBC hDBC = static_cast<HDBC>(js_hdbc->Value());
  HSTMT hSTMT = static_cast<HSTMT>(js_hstmt->Value());
  
  //create a new OBCResult object
  ODBCResult* objODBCResult = new ODBCResult(hENV, hDBC, hSTMT);
  
  //specify the buffer length
  objODBCResult->bufferLength = MAX_VALUE_SIZE - 1;
  
  //initialze a buffer for this object
  objODBCResult->buffer = (uint16_t *) malloc(objODBCResult->bufferLength + 1);
  //TODO: make sure the malloc succeeded

  //set the initial colCount to 0
  objODBCResult->colCount = 0;
  
  objODBCResult->Wrap(args.Holder());
  
  return scope.Close(args.Holder());
}

/*
 * Fetch
 */

Handle<Value> ODBCResult::Fetch(const Arguments& args) {
  DEBUG_PRINTF("ODBCResult::Fetch\n");
  
  HandleScope scope;
  
  ODBCResult* objODBCResult = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  fetch_work_data* data = (fetch_work_data *) calloc(1, sizeof(fetch_work_data));
  
  Local<Function> cb;
   
  if (args.Length() == 0 || !args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(
              String::New("Argument 0 must be a callback function."))
    );
  }
  
  cb = Local<Function>::Cast(args[0]);
  
  data->cb = Persistent<Function>::New(cb);
  
  data->objResult = objODBCResult;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(), 
    work_req, 
    UV_Fetch, 
    (uv_after_work_cb)UV_AfterFetch);

  objODBCResult->Ref();

  return scope.Close(Undefined());
}

void ODBCResult::UV_Fetch(uv_work_t* work_req) {
  DEBUG_PRINTF("ODBCResult::UV_Fetch\n");
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  data->result = SQLFetch(data->objResult->m_hSTMT);
}

void ODBCResult::UV_AfterFetch(uv_work_t* work_req, int status) {
  DEBUG_PRINTF("ODBCResult::UV_AfterFetch\n");
  
  HandleScope scope;
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  SQLRETURN ret = data->result;
  
  //check to see if there was an error
  if (ret == SQL_ERROR)  {
    ODBC::CallbackSQLError(
      data->objResult->m_hENV, 
      data->objResult->m_hDBC, 
      data->objResult->m_hSTMT,
      data->cb);
    
    free(data);
    free(work_req);
    
    data->objResult->Unref();
    
    return;
  }
  
  //check to see if we are at the end of the recordset
  if (ret == SQL_NO_DATA) {
    ODBC::FreeColumns(data->objResult->columns, &data->objResult->colCount);
    
    Handle<Value> args[2];
    args[0] = Null();
    args[1] = Null();
    
    data->cb->Call(Context::GetCurrent()->Global(), 2, args);
    data->cb.Dispose();
    
    free(data);
    free(work_req);
    
    data->objResult->Unref();
    
    return;
  }

  if (data->objResult->colCount == 0) {
    data->objResult->columns = ODBC::GetColumns(
      data->objResult->m_hSTMT, 
      &data->objResult->colCount);
  }
  
  Handle<Value> args[2];

  args[0] = Null();
  args[1] = ODBC::GetRecordTuple(
    data->objResult->m_hSTMT,
    data->objResult->columns,
    &data->objResult->colCount,
    data->objResult->buffer,
    data->objResult->bufferLength);

  data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  data->cb.Dispose();

  free(data);
  free(work_req);
  
  return;
}

/*
 * FetchAll
 */

Handle<Value> ODBCResult::FetchAll(const Arguments& args) {
  DEBUG_PRINTF("ODBCResult::FetchAll\n");
  
  HandleScope scope;
  
  ODBCResult* objODBCResult = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  fetch_work_data* data = (fetch_work_data *) calloc(1, sizeof(fetch_work_data));
  
  Local<Function> cb;
   
  if (args.Length() == 0 || !args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(
              String::New("Argument 0 must be a callback function."))
    );
  }
  
  cb = Local<Function>::Cast(args[0]);
  
  data->cb = Persistent<Function>::New(cb);
  data->objResult = objODBCResult;
  
  work_req->data = data;
  
  uv_queue_work(uv_default_loop(),
    work_req, 
    UV_FetchAll, 
    (uv_after_work_cb)UV_AfterFetchAll);

  data->objResult->Ref();

  return scope.Close(Undefined());
}

void ODBCResult::UV_FetchAll(uv_work_t* work_req) {
  DEBUG_PRINTF("ODBCResult::UV_FetchAll\n");
  //fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  /*
   * NOTE: Terriblness resides here:
   * 
   * FetchAll is not actually asynchronous at this time. In order to truly do this
   * we would need to allocate memory here and load the entire recordset into
   * some structure so that we can then loop over that structure in UV_AfterFetchAll
   * 
   * One reason that this is the case is that somewhere I read that you should
   * not do anything with V8 data structure while you are in the thread pool.
   * If that is not true, then we could just do all of UV_AfterFetchAll right here
   * while we are in the thread pool.
   * 
   * For true async behaviour, use Fetch; one at a time.
   * 
   */

  //data->result = SQLFetch(data->objResult->m_hSTMT);
}

void ODBCResult::UV_AfterFetchAll(uv_work_t* work_req, int status) {
  DEBUG_PRINTF("ODBCResult::UV_AfterFetchAll\n");
  
  HandleScope scope;
  
  fetch_work_data* data = (fetch_work_data *)(work_req->data);
  
  ODBCResult* self = data->objResult->self();
  
  Local<Object> objError = Object::New();
  
  int count = 0;
  int errorCount = 0;
  
  if (self->colCount == 0) {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }
  
  Local<Array> rows = Array::New();
  
  //loop through all records
  while (true) {
    SQLRETURN ret = SQLFetch(self->m_hSTMT);
    
    //check to see if there was an error
    if (ret == SQL_ERROR)  {
      errorCount++;
      
      char errorMessage[512];
      char errorSQLState[128];

      SQLError( self->m_hENV, 
                self->m_hDBC, 
                self->m_hSTMT,
                (SQLCHAR *) errorSQLState,
                NULL,
                (SQLCHAR *) errorMessage,
                sizeof(errorMessage),
                NULL);

      objError->Set(String::New("state"), String::New(errorSQLState));
      objError->Set(String::New("error"),
                    String::New("[node-odbc] Error in SQLFetch"));
      objError->Set(String::New("message"), String::New(errorMessage));
      
      break;
    }
    
    //check to see if we are at the end of the recordset
    if (ret == SQL_NO_DATA) {
      ODBC::FreeColumns(self->columns, &self->colCount);
      
      break;
    }

    rows->Set(
      Integer::New(count), 
      ODBC::GetRecordTuple(
        self->m_hSTMT,
        self->columns,
        &self->colCount,
        self->buffer,
        self->bufferLength)
    );

    count++;
  }
  
  Handle<Value> args[2];
  args[0] = Null();
  args[1] = rows;
    
  data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  data->cb.Dispose();
  
  free(data);
  free(work_req);
  
  self->Unref();
}

Handle<Value> ODBCResult::CloseSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCResult::Close\n");
  
  HandleScope scope;
  
  ODBCResult* result = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  result->Free();
  
  return scope.Close(Undefined());
}

Handle<Value> ODBCResult::MoreResultsSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCResult::MoreResults\n");
  
  HandleScope scope;
  
  ODBCResult* result = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  //result->colCount = 0;
  
  SQLRETURN ret = SQLMoreResults(result->m_hSTMT);

  return scope.Close(SQL_SUCCEEDED(ret) ? True() : False());
}

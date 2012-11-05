/*
  Copyright (c) 2012, Dan VerWeire<dverweire@gmail.com>
  Copyright (c) 2010, Lee Smith<notwink@gmail.com>

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
#include "odbc_result.h"

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
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "moreResults", MoreResults);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "close", Close);

  // Attach the Database Constructor to the target object
  target->Set( v8::String::NewSymbol("ODBCResult"),
               constructor_template->GetFunction());
  
  scope.Close(Undefined());
}

ODBCResult::~ODBCResult() {
  this->Free();
}

void ODBCResult::Free() {
  if (m_hDBC) {
    uv_mutex_lock(&ODBC::g_odbcMutex);
    
    SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
    m_hDBC = NULL;
    
    uv_mutex_unlock(&ODBC::g_odbcMutex);
    
    if (bufferLength > 0) {
      free(buffer);
    }
  }
}

Handle<Value> ODBCResult::New(const Arguments& args) {
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

Handle<Value> ODBCResult::Fetch(const Arguments& args) {
  HandleScope scope;
  
  ODBCResult* objODBCResult = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  Fetch_Request* req_fetch = (Fetch_Request *) calloc(1, sizeof(Fetch_Request));
  
  Local<Function> cb;
   
  if (args.Length() == 0 || !args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(
              String::New("Argument 0 must be a callback function."))
    );
  }
  
  cb = Local<Function>::Cast(args[0]);
  
  req_fetch->callback = Persistent<Function>::New(cb);
  
  req_fetch->objResult = objODBCResult;
  work_req->data = req_fetch;
  
  uv_queue_work(uv_default_loop(), work_req, UV_Fetch, UV_AfterFetch);

  objODBCResult->Ref();

  return scope.Close(Undefined());
}

void ODBCResult::UV_Fetch(uv_work_t* work_req) {
  Fetch_Request* req_fetch = (Fetch_Request *)(work_req->data);
  
  ODBCResult* self = req_fetch->objResult->self();
  
  req_fetch->result = SQLFetch(self->m_hSTMT);
}

void ODBCResult::UV_AfterFetch(uv_work_t* work_req) {
  HandleScope scope;
  
  Fetch_Request* req_fetch = (Fetch_Request *)(work_req->data);
  
  SQLRETURN ret = req_fetch->result;
  
  ODBCResult* self = req_fetch->objResult->self();
  
  //check to see if there was an error
  if (ret == SQL_ERROR)  {
    Local<Object> objError = Object::New();

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
    
    //emit an error event immidiately.
    Local<Value> args[1];
    args[0] = objError;
    req_fetch->callback->Call(Context::GetCurrent()->Global(), 1, args);
    req_fetch->callback.Dispose();
    
    free(work_req);
    free(req_fetch);
    
    self->Unref();
    
    return;
  }
  
  //check to see if we are at the end of the recordset
  if (ret == SQL_NO_DATA) {
    ODBC::FreeColumns(self->columns, &self->colCount);
    
    Handle<Value> args[2];
    args[0] = Null();
    args[1] = Null();
    
    req_fetch->callback->Call(Context::GetCurrent()->Global(), 2, args);
    req_fetch->callback.Dispose();
    
    free(work_req);
    free(req_fetch);
    
    self->Unref();
    
    return;
  }

  if (self->colCount == 0) {
    self->columns = ODBC::GetColumns(self->m_hSTMT, &self->colCount);
  }
  
  Handle<Value> args[2];

  args[0] = Null();
  args[1] = ODBC::GetRecordTuple( self->m_hSTMT,
                            self->columns,
                            &self->colCount,
                            self->buffer,
                            self->bufferLength);

  req_fetch->callback->Call(Context::GetCurrent()->Global(), 2, args);
  req_fetch->callback.Dispose();

  free(work_req);
  free(req_fetch);
  return;
}

Handle<Value> ODBCResult::FetchAll(const Arguments& args) {
  HandleScope scope;
  
  ODBCResult* objODBCResult = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  Fetch_Request* fetch_Request = (Fetch_Request *) calloc(1, sizeof(Fetch_Request));
  
  Local<Function> cb;
   
  if (args.Length() == 0 || !args[0]->IsFunction()) {
    return ThrowException(Exception::TypeError(
              String::New("Argument 0 must be a callback function."))
    );
  }
  
  cb = Local<Function>::Cast(args[0]);
  
  fetch_Request->callback = Persistent<Function>::New(cb);
  
  fetch_Request->objResult = objODBCResult;
  work_req->data = fetch_Request;
  
  uv_queue_work(uv_default_loop(), work_req, UV_FetchAll, UV_AfterFetchAll);

  objODBCResult->Ref();

  return scope.Close(Undefined());
}

void ODBCResult::UV_FetchAll(uv_work_t* work_req) {
  //Fetch_Request* req_fetch = (Fetch_Request *)(work_req->data);
  
  //ODBCResult* self = req_fetch->objResult->self();
  
  //req_fetch->result = SQLFetch(self->m_hSTMT);
}

void ODBCResult::UV_AfterFetchAll(uv_work_t* work_req) {
  HandleScope scope;
  
  Fetch_Request* req_fetch = (Fetch_Request *)(work_req->data);
  
  ODBCResult* self = req_fetch->objResult->self();
  
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
    
    ODBCResult* self = req_fetch->objResult->self();
    
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

    rows->Set( Integer::New(count), 
               ODBC::GetRecordTuple( self->m_hSTMT,
                                     self->columns,
                                     &self->colCount,
                                     self->buffer,
                                     self->bufferLength));

    count++;
  }
  
  Handle<Value> args[2];
  args[0] = Null();
  args[1] = rows;
    
  req_fetch->callback->Call(Context::GetCurrent()->Global(), 2, args);
  req_fetch->callback.Dispose();
  
  free(work_req);
  free(req_fetch);
  
  self->Unref();
}


Handle<Value> ODBCResult::Close(const Arguments& args) {
  HandleScope scope;
  
  ODBCResult* objODBCResult = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  
  objODBCResult->Free();
  
  return scope.Close(Undefined());
}

Handle<Value> ODBCResult::MoreResults(const Arguments& args) {
  HandleScope scope;
  
  ODBCResult* objODBCResult = ObjectWrap::Unwrap<ODBCResult>(args.Holder());
  objODBCResult->colCount = 0;
  
  SQLRETURN ret = SQLMoreResults(objODBCResult->m_hSTMT);

  return scope.Close(SQL_SUCCEEDED(ret) ? True() : False());
}

/*
 * fetchAll

//Loop through all result sets
  do {
    Local<Array> rows = Array::New();

    //Retrieve and store all columns and their attributes
    Column *columns = GetColumns(self->m_hStmt, &colCount);

    if (colCount > 0) {
      int count = 0;
      
      //I dont think odbc will tell how many rows are returned, loop until out
      while(true) {
        
        ret = SQLFetch(self->m_hStmt);
        
        //TODO: Do something to enable/disable dumping these info messages to 
        //the console.
        if (ret == SQL_SUCCESS_WITH_INFO ) {
          char errorMessage[512];
          char errorSQLState[128];

          SQLError( self->m_hEnv, 
                    self->m_hDBC, 
                    self->m_hStmt,
                    (SQLCHAR *) errorSQLState,
                    NULL,
                    (SQLCHAR *) errorMessage, 
                    sizeof(errorMessage), 
                    NULL);

          printf("UV_Query => %s\n", errorMessage);
          printf("UV_Query => %s\n", errorSQLState);
        }

        if (ret == SQL_ERROR)  {
          objError = Object::New();

          char errorMessage[512];
          char errorSQLState[128];

          SQLError( self->m_hEnv, 
                    self->m_hDBC, 
                    self->m_hStmt,
                    (SQLCHAR *) errorSQLState,
                    NULL,
                    (SQLCHAR *) errorMessage,
                    sizeof(errorMessage),
                    NULL);

          errorCount++;
          
          objError->Set(String::New("state"), String::New(errorSQLState));
          objError->Set(String::New("error"),
                        String::New("[node-odbc] Error in SQLFetch"));
          objError->Set(String::New("message"), String::New(errorMessage));
          objError->Set(String::New("query"), String::New(prep_req->sql));
          
          //emit an error event immidiately.
          Local<Value> args[1];
          args[0] = objError;
          prep_req->cb->Call(Context::GetCurrent()->Global(), 1, args);
          
          break;
        }
        
        if (ret == SQL_NO_DATA) {
          break;
        }
        
        if (self->mode == MODE_CALLBACK_FOR_EACH) {
          Handle<Value> args[2];
          
          args[0] = Null();
          args[1] = GetRecordTuple( self->m_hStmt,
                                    columns,
                                    &colCount,
                                    buf,
                                    MAX_VALUE_SIZE - 1);
          
          prep_req->cb->Call(Context::GetCurrent()->Global(), 2, args);
        }
        else {
          rows->Set( Integer::New(count), 
                    GetRecordTuple( self->m_hStmt,
                                    columns,
                                    &colCount,
                                    buf,
                                    MAX_VALUE_SIZE - 1));
        }
        
        count++;
      }
      
      FreeColumns(columns, &colCount);
    }
    
    //move to the next result set
    ret = SQLMoreResults( self->m_hStmt );
    
    //Only trigger an emit if there are columns OR if this is the last result 
    //and none others have been emitted odbc will process individual statments 
    //like select @something = 1 as a recordset even though it doesn't have any
    //columns. We don't want to emit those unless there are actually columns
    if (colCount > 0 || ( ret != SQL_SUCCESS && emitCount == 0 )) {
      emitCount++;
      
      Local<Value> args[3];
      
      if (errorCount) {
        args[0] = objError;
      }
      else {
        args[0] = Local<Value>::New(Null());
      }
      
      args[1] = rows;

      //true or false, are there more result sets to follow this emit?
      args[2] = Local<Boolean>::New((ret == SQL_SUCCESS) ? True() : False()); 

      prep_req->cb->Call(Context::GetCurrent()->Global(), 3, args);
    }
  }
  while ( self->canHaveMoreResults && ret == SQL_SUCCESS );

 */
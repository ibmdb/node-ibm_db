/*
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

#ifdef _WIN32
#include "strptime.h"
#endif

using namespace v8;
using namespace node;

uv_mutex_t ODBC::g_odbcMutex;
uv_async_t ODBC::g_async;

Persistent<FunctionTemplate> ODBC::constructor_template;

void ODBC::Init(v8::Handle<Object> target) {
  DEBUG_PRINTF("ODBC::Init\n");
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  // Constructor Template
  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->SetClassName(String::NewSymbol("Database"));

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = constructor_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);
  
  // Properties
  instance_template->SetAccessor(String::New("mode"), ModeGetter, ModeSetter);
  instance_template->SetAccessor(String::New("connected"), ConnectedGetter);
  
  // Prototype Methods
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "dispatchOpen", Open);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "dispatchClose", Close);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "dispatchQuery", Query);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "dispatchQueryAll", QueryAll);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "dispatchTables", Tables);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "dispatchColumns", Columns);

  // Attach the Database Constructor to the target object
  target->Set( v8::String::NewSymbol("Database"),
               constructor_template->GetFunction());
  
  scope.Close(Undefined());
  
#if NODE_VERSION_AT_LEAST(0, 7, 9)
  // Initialize uv_async so that we can prevent node from exiting
  uv_async_init( uv_default_loop(),
                 &ODBC::g_async,
                 ODBC::WatcherCallback);
  
  // Not sure if the init automatically calls uv_ref() because there is weird
  // behavior going on. When ODBC::Init is called which initializes the 
  // uv_async_t g_async above, there seems to be a ref which will keep it alive
  // but we only want this available so that we can uv_ref() later on when
  // we have a connection.
  // so to work around this, I am possibly mistakenly calling uv_unref() once
  // so that there are no references on the loop.
  uv_unref((uv_handle_t *)&ODBC::g_async);
#endif
  
  // Initialize the cross platform mutex provided by libuv
  uv_mutex_init(&ODBC::g_odbcMutex);
}

ODBC::~ODBC() {
  DEBUG_PRINTF("ODBC::~ODBC\n");
  this->Free();
}

void ODBC::Free() {
  DEBUG_PRINTF("ODBC::Free\n");
  if (m_hDBC || m_hEnv) {
    uv_mutex_lock(&ODBC::g_odbcMutex);
    
    if (m_hDBC) {
      SQLDisconnect(m_hDBC);
      SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
      m_hDBC = NULL;
    }
    
    if (m_hEnv) {
      SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
      m_hEnv = NULL;      
    }

    uv_mutex_unlock(&ODBC::g_odbcMutex);
  }
}

Handle<Value> ODBC::New(const Arguments& args) {
  DEBUG_PRINTF("ODBC::New\n");
  HandleScope scope;
  ODBC* dbo = new ODBC();
  
  dbo->Wrap(args.Holder());
  dbo->mode = 1;
  dbo->connected = false;
  dbo->m_hDBC = NULL;
  dbo->m_hEnv = NULL;
  
  return scope.Close(args.Holder());
}

void ODBC::WatcherCallback(uv_async_t *w, int revents) {
  DEBUG_PRINTF("ODBC::WatcherCallback\n");
  //i don't know if we need to do anything here
}

Handle<Value> ODBC::ModeGetter(Local<String> property, const AccessorInfo &info) {
  HandleScope scope;

  ODBC *obj = ObjectWrap::Unwrap<ODBC>(info.Holder());

  if (obj->mode > 0) {
      return scope.Close(Integer::New(obj->mode));
  } else {
      return Undefined();
  }
}

void ODBC::ModeSetter(Local<String> property, Local<Value> value, const AccessorInfo &info) {
  HandleScope scope;

  ODBC *obj = ObjectWrap::Unwrap<ODBC>(info.Holder());
  
  if (value->IsNumber()) {
    obj->mode = value->Int32Value();
  }
}

Handle<Value> ODBC::ConnectedGetter(Local<String> property, const AccessorInfo &info) {
  HandleScope scope;

  ODBC *obj = ObjectWrap::Unwrap<ODBC>(info.Holder());

  return scope.Close(obj->connected ? True() : False());
}

void ODBC::UV_AfterOpen(uv_work_t* req) {
  DEBUG_PRINTF("ODBC::UV_AfterOpen\n");
  HandleScope scope;
  open_request* open_req = (open_request *)(req->data);

  ODBC* self = open_req->dbo->self();
  
  Local<Value> argv[1];
  
  bool err = false;

  if (open_req->result) {
    err = true;

    SQLINTEGER i = 0;
    SQLINTEGER native;
    SQLSMALLINT len;
    SQLRETURN ret;
    char errorSQLState[7];
    char errorMessage[256];

    do {
      ret = SQLGetDiagRec( SQL_HANDLE_DBC, 
                           open_req->dbo->self()->m_hDBC,
                           ++i, 
                           (SQLCHAR *) errorSQLState,
                           &native,
                           (SQLCHAR *) errorMessage,
                           sizeof(errorMessage),
                           &len );

      if (SQL_SUCCEEDED(ret)) {
        Local<Object> objError = Object::New();

        objError->Set(String::New("error"), String::New("[node-odbc] SQL_ERROR"));
        objError->Set(String::New("message"), String::New(errorMessage));
        objError->Set(String::New("state"), String::New(errorSQLState));

        argv[0] = objError;
      }
    } while( ret == SQL_SUCCESS );
  }

  if (!err) {
    self->connected = true;
    
    //only uv_ref if the connection was successful
#if NODE_VERSION_AT_LEAST(0, 7, 9)
    uv_ref((uv_handle_t *)&ODBC::g_async);
#else
    uv_ref(uv_default_loop());
#endif
  }
  
  TryCatch try_catch;

  open_req->dbo->Unref();
  open_req->cb->Call(Context::GetCurrent()->Global(), err ? 1 : 0, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  open_req->cb.Dispose();
  
  free(open_req);
  free(req);
  scope.Close(Undefined());
}

void ODBC::UV_Open(uv_work_t* req) {
  DEBUG_PRINTF("ODBC::UV_Open\n");
  open_request* open_req = (open_request *)(req->data);
  ODBC* self = open_req->dbo->self();
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  
  int ret = SQLAllocEnv( &self->m_hEnv );

  if( ret == SQL_SUCCESS ) {
    ret = SQLAllocConnect( self->m_hEnv,&self->m_hDBC );

    if( ret == SQL_SUCCESS ) {
      SQLSetConnectOption( self->m_hDBC, SQL_LOGIN_TIMEOUT, 5 );

      char connstr[1024];

      //Attempt to connect
      ret = SQLDriverConnect( self->m_hDBC, 
                              NULL,
                              (SQLCHAR*) open_req->connection,
                              strlen(open_req->connection),
                              (SQLCHAR*) connstr,
                              1024,
                              NULL,
                              SQL_DRIVER_NOPROMPT);

      if( ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO ) {
        HSTMT hStmt;
        
        ret = SQLAllocStmt( self->m_hDBC, &hStmt );

        ret = SQLGetFunctions( self->m_hDBC,
                               SQL_API_SQLMORERESULTS, 
                               &self->canHaveMoreResults);

        if ( !SQL_SUCCEEDED(ret)) {
          self->canHaveMoreResults = 0;
        }
        
        ret = SQLFreeHandle( SQL_HANDLE_STMT, hStmt);
      }
    }
  }
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  open_req->result = ret;
}

Handle<Value> ODBC::Open(const Arguments& args) {
  DEBUG_PRINTF("ODBC::Open\n");
  HandleScope scope;

  REQ_STR_ARG(0, connection);
  REQ_FUN_ARG(1, cb);

  ODBC* dbo = ObjectWrap::Unwrap<ODBC>(args.Holder());
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  open_request* open_req = (open_request *) calloc(1, sizeof(open_request) + connection.length());

  if (!open_req) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(String::New("Could not allocate enough memory")));
  }

  strcpy(open_req->connection, *connection);
  open_req->cb = Persistent<Function>::New(cb);
  open_req->dbo = dbo;
  
  work_req->data = open_req;
  
  uv_queue_work(uv_default_loop(), work_req, UV_Open, (uv_after_work_cb)UV_AfterOpen);

  dbo->Ref();

  return scope.Close(args.Holder());
}

void ODBC::UV_AfterClose(uv_work_t* req) {
  DEBUG_PRINTF("ODBC::UV_AfterClose\n");
  HandleScope scope;

  close_request* close_req = (close_request *)(req->data);

  ODBC* dbo = close_req->dbo;
  
  Local<Value> argv[1];
  bool err = false;
  
  if (close_req->result) {
    err = true;
    argv[0] = Exception::Error(String::New("Error closing database"));
  }
  else {
    dbo->connected = false;
    
    //only unref if the connection was closed
#if NODE_VERSION_AT_LEAST(0, 7, 9)
    uv_unref((uv_handle_t *)&ODBC::g_async);
#else
    uv_unref(uv_default_loop());
#endif
  }

  TryCatch try_catch;

  close_req->dbo->Unref();
  close_req->cb->Call(Context::GetCurrent()->Global(), err ? 1 : 0, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  close_req->cb.Dispose();

  free(close_req);
  free(req);
  scope.Close(Undefined());
}

void ODBC::UV_Close(uv_work_t* req) {
  DEBUG_PRINTF("ODBC::UV_Close\n");
  close_request* close_req = (close_request *)(req->data);
  ODBC* dbo = close_req->dbo;
  
  dbo->Free();
}

Handle<Value> ODBC::Close(const Arguments& args) {
  DEBUG_PRINTF("ODBC::Close\n");
  HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBC* dbo = ObjectWrap::Unwrap<ODBC>(args.Holder());
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  close_request* close_req = (close_request *) (calloc(1, sizeof(close_request)));

  if (!close_req) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(String::New("Could not allocate enough memory")));
  }

  close_req->cb = Persistent<Function>::New(cb);
  close_req->dbo = dbo;

  work_req->data = close_req;
  
  uv_queue_work(uv_default_loop(), work_req, UV_Close, (uv_after_work_cb)UV_AfterClose);

  dbo->Ref();

  return scope.Close(Undefined());
}

void ODBC::UV_AfterQuery(uv_work_t* req) {
  query_request* prep_req = (query_request *)(req->data);
  
  HandleScope scope;
  
  //an easy reference to the Database object
  ODBC* self = prep_req->dbo->self();
  
  //used to capture the return value from various SQL function calls
  SQLRETURN ret;

  //First thing, let's check if the execution of the query returned any errors 
  //in UV_Query
  if(prep_req->result == SQL_ERROR) {
    Local<Object> objError = Object::New();
    
    char errorMessage[512];
    char errorSQLState[128];
    
    SQLError( self->m_hEnv,
              self->m_hDBC,
              prep_req->hSTMT,
              (SQLCHAR *) errorSQLState,
              NULL,
              (SQLCHAR *) errorMessage,
              sizeof(errorMessage), 
              NULL);

    objError->Set(String::New("state"), String::New(errorSQLState));
    objError->Set(String::New("error"), String::New("[node-odbc] Error in "
                                                "SQLExecDirect or SQLExecute"));
    objError->Set(String::New("message"), String::New(errorMessage));
    
    //only set the query value of the object if we actually have a query
    if (prep_req->sql != NULL) {
      objError->Set(String::New("query"), String::New(prep_req->sql));
    }
    
    //emit an error event immidiately.
    Local<Value> args[1];
    args[0] = objError;
    prep_req->cb->Call(Context::GetCurrent()->Global(), 1, args);
  }
  else {
    Local<Value> args[3];
    args[0] = External::New(self->m_hEnv);
    args[1] = External::New(self->m_hDBC);
    args[2] = External::New(prep_req->hSTMT);
    Persistent<Object> js_result(ODBCResult::constructor_template->
                              GetFunction()->NewInstance(3, args));

    args[0] = Local<Value>::New(Null());
    args[1] = Local<Object>::New(js_result);
    
    prep_req->cb->Call(Context::GetCurrent()->Global(), 2, args);
  }
  
  TryCatch try_catch;
  
  self->Unref();
  
  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
  
  prep_req->cb.Dispose();
  free(prep_req->sql);
  free(prep_req->catalog);
  free(prep_req->schema);
  free(prep_req->table);
  free(prep_req->type);
  free(prep_req);
  free(req);
  
  scope.Close(Undefined());
}

void ODBC::UV_Query(uv_work_t* req) {
  query_request* prep_req = (query_request *)(req->data);
  
  Parameter prm;
  SQLRETURN ret;
  
  uv_mutex_lock(&ODBC::g_odbcMutex);

  //allocate a new statment handle
  SQLAllocHandle( SQL_HANDLE_STMT, 
                  prep_req->dbo->m_hDBC, 
                  &prep_req->hSTMT );

  uv_mutex_unlock(&ODBC::g_odbcMutex);

  //check to see if should excute a direct or a parameter bound query
  if (!prep_req->paramCount) {
    // execute the query directly
    ret = SQLExecDirect( prep_req->hSTMT,
                         (SQLCHAR *) prep_req->sql, 
                         strlen(prep_req->sql));
  }
  else {
    // prepare statement, bind parameters and execute statement 
    ret = SQLPrepare( prep_req->hSTMT,
                      (SQLCHAR *) prep_req->sql, 
                      strlen(prep_req->sql));
    
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
      for (int i = 0; i < prep_req->paramCount; i++) {
        prm = prep_req->params[i];
        
        DEBUG_PRINTF("ODBC::UV_Query - param[%i]: c_type=%i type=%i "
                     "buffer_length=%i size=%i length=%i &length=%X\n", i, prm.c_type, prm.type, 
                     prm.buffer_length, prm.size, prm.length, &prep_req->params[i].length);

        ret = SQLBindParameter( prep_req->hSTMT,    //StatementHandle
                                i + 1,              //ParameterNumber
                                SQL_PARAM_INPUT,    //InputOutputType
                                prm.c_type,         //ValueType
                                prm.type,           //ParameterType
                                prm.size,           //ColumnSize
                                0,                  //DecimalDigits
                                prm.buffer,         //ParameterValuePtr
                                prm.buffer_length,  //BufferLength
                                //using &prm.length did not work here...
                                &prep_req->params[i].length); //StrLen_or_IndPtr
        
        if (ret == SQL_ERROR) {break;}
      }

      if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        ret = SQLExecute(prep_req->hSTMT);
      }
    }
    
    // free parameters
    for (int i = 0; i < prep_req->paramCount; i++) {
      if (prm = prep_req->params[i], prm.buffer != NULL) {
        switch (prm.c_type) {
          case SQL_C_CHAR:    free(prm.buffer);             break; 
          case SQL_C_LONG:    delete (int64_t *)prm.buffer; break;
          case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break;
          case SQL_C_BIT:     delete (bool    *)prm.buffer; break;
        }
      }
    }
    free(prep_req->params);
  }

  // this will be checked later in UV_AfterQuery
  prep_req->result = ret;
}

Handle<Value> ODBC::Query(const Arguments& args) {
  HandleScope scope;

  REQ_STR_ARG(0, sql);
  
  Local<Function> cb; 

  ODBC* dbo = ObjectWrap::Unwrap<ODBC>(args.Holder());
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  query_request* prep_req = (query_request *) calloc(1, sizeof(query_request));

  if (!prep_req) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(String::New("Could not allocate enough memory")));
  }

  // populate prep_req->params if parameters were supplied
  //
  if (args.Length() > 2) {
      if ( !args[1]->IsArray() ) {
           return ThrowException(Exception::TypeError(
                      String::New("Argument 1 must be an Array"))
           );
      }
      else if ( !args[2]->IsFunction() ) {
           return ThrowException(Exception::TypeError(
                      String::New("Argument 2 must be a Function"))
           );
      }
  
      cb = Local<Function>::Cast(args[2]);
      prep_req->params = GetParametersFromArray(Local<Array>::Cast(args[1]), &prep_req->paramCount);
  }
  else {
      if ( !args[1]->IsFunction() ) {
           return ThrowException(Exception::TypeError(
                      String::New("Argument 1 must be a Function"))
           );
      }

      cb = Local<Function>::Cast(args[1]);

      prep_req->paramCount = 0;
  }

  prep_req->sql = (char *) malloc(sql.length() +1);
  prep_req->catalog = NULL;
  prep_req->schema = NULL;
  prep_req->table = NULL;
  prep_req->type = NULL;
  prep_req->column = NULL;
  prep_req->cb = Persistent<Function>::New(cb);
  
  strcpy(prep_req->sql, *sql);
  
  prep_req->dbo = dbo;
  work_req->data = prep_req;
  
  uv_queue_work(uv_default_loop(), work_req, UV_Query, (uv_after_work_cb)UV_AfterQuery);

  dbo->Ref();

  return  scope.Close(Undefined());
}

void ODBC::UV_AfterQueryAll(uv_work_t* req) {
  query_request* prep_req = (query_request *)(req->data);
  
  HandleScope scope;
  
  //an easy reference to the Database object
  ODBC* self = prep_req->dbo->self();

  //our error object which we will use if we discover errors while processing the result set
  Local<Object> objError;
  
  //used to keep track of the number of columns received in a result set
  short colCount = 0;
  
  //used to keep track of the number of event emittions that have occurred
  short emitCount = 0;
  
  //used to keep track of the number of errors that have been found
  short errorCount = 0;
  
  //used to capture the return value from various SQL function calls
  SQLRETURN ret;
  
  //allocate a buffer for incoming column values
  uint16_t* buf = (uint16_t *) malloc(MAX_VALUE_SIZE);
  
  //check to make sure malloc succeeded
  if (buf == NULL) {
    objError = Object::New();

    //malloc failed, set an error message
    objError->Set(String::New("error"), String::New("[node-odbc] Failed Malloc"));
    objError->Set(String::New("message"), String::New("An attempt to allocate memory failed. This allocation was for a value buffer of incoming recordset values."));
    
    //emit an error event immidiately.
    Local<Value> args[3];
    args[0] = objError;
    args[1] = Local<Value>::New(Null());
    args[2] = Local<Boolean>::New(False());
    
    //emit an error event
    prep_req->cb->Call(Context::GetCurrent()->Global(), 3, args);
    
    //emit a result event
    goto cleanupshutdown;
  }
  //else {
    //malloc succeeded so let's continue 
    
    //set the first byte of the buffer to \0 instead of memsetting the entire buffer to 0
    buf[0] = '\0'; 
    
    //First thing, let's check if the execution of the query returned any errors (in UV_Query)
    if(prep_req->result == SQL_ERROR) {
      objError = Object::New();

      errorCount++;
      
      char errorMessage[512];
      char errorSQLState[128];
      SQLError(self->m_hEnv, self->m_hDBC, prep_req->hSTMT,(SQLCHAR *)errorSQLState,NULL,(SQLCHAR *)errorMessage, sizeof(errorMessage), NULL);
      objError->Set(String::New("state"), String::New(errorSQLState));
      objError->Set(String::New("error"), String::New("[node-odbc] SQL_ERROR"));
      objError->Set(String::New("message"), String::New(errorMessage));
      
      //only set the query value of the object if we actually have a query
      if (prep_req->sql != NULL) {
        objError->Set(String::New("query"), String::New(prep_req->sql));
      }
      
      //emit an error event immidiately.
      Local<Value> args[1];
      args[0] = objError;
      prep_req->cb->Call(Context::GetCurrent()->Global(), 1, args);
      goto cleanupshutdown;
    }
    
    //loop through all result sets
    do {
      Local<Array> rows = Array::New();

      // retrieve and store column attributes to build the row object
      Column *columns = GetColumns(prep_req->hSTMT, &colCount);

      if (colCount > 0) {
        int count = 0;
        
        // i dont think odbc will tell how many rows are returned, loop until out...
        while(true) {
          Local<Object> tuple = Object::New();
          ret = SQLFetch(prep_req->hSTMT);
          
          //TODO: Do something to enable/disable dumping these info messages to the console.
          if (ret == SQL_SUCCESS_WITH_INFO ) {
            char errorMessage[512];
            char errorSQLState[128];
            SQLError(self->m_hEnv, self->m_hDBC, prep_req->hSTMT,(SQLCHAR *)errorSQLState,NULL,(SQLCHAR *)errorMessage, sizeof(errorMessage), NULL);
            
            printf("UV_Query => %s\n", errorMessage);
            printf("UV_Query => %s\n", errorSQLState);
          }

          if (ret == SQL_ERROR)  {
            objError = Object::New();

            char errorMessage[512];
            char errorSQLState[128];
            SQLError(self->m_hEnv, self->m_hDBC, prep_req->hSTMT,(SQLCHAR *)errorSQLState,NULL,(SQLCHAR *)errorMessage, sizeof(errorMessage), NULL);
            
            errorCount++;
            objError->Set(String::New("state"), String::New(errorSQLState));
            objError->Set(String::New("error"), String::New("[node-odbc] SQL_ERROR"));
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

          for(int i = 0; i < colCount; i++) {
            tuple->Set( String::New((const char *) columns[i].name),
                        GetColumnValue( prep_req->hSTMT, columns[i], buf, MAX_VALUE_SIZE - 1));
          }
          
          rows->Set(Integer::New(count), tuple);
          count++;
        }
        
        for(int i = 0; i < colCount; i++) {
          delete [] columns[i].name;
        }

        delete [] columns;
      }
      
      //move to the next result set
      ret = SQLMoreResults( prep_req->hSTMT );
      
      //Only trigger an emit if there are columns OR if this is the last result and none others have been emitted
      //odbc will process individual statments like select @something = 1 as a recordset even though it doesn't have
      //any columns. We don't want to emit those unless there are actually columns
      if (colCount > 0 || ( ret != SQL_SUCCESS && emitCount == 0 )) {
        emitCount++;
        
        Local<Value> args[3];
        
        if (errorCount) {
          args[0] = objError; //(objError->IsUndefined()) ? Undefined() : ;
        }
        else {
          args[0] = Local<Value>::New(Null());
        }
        
        args[1] = rows;
        //true or false, are there more result sets to follow this emit?
        args[2] = Local<Boolean>::New(( ret == SQL_SUCCESS ) ? True() : False() ); 
        
        prep_req->cb->Call(Context::GetCurrent()->Global(), 3, args);
      }
    }
    while ( self->canHaveMoreResults && ret == SQL_SUCCESS );
  //} //end of malloc check
cleanupshutdown:
  TryCatch try_catch;
  
  self->Unref();
  
  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
  
  uv_mutex_lock(&ODBC::g_odbcMutex);

  SQLFreeHandle(SQL_HANDLE_STMT,  prep_req->hSTMT);

  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
  free(buf);
  prep_req->cb.Dispose();
  free(prep_req->sql);
  free(prep_req->catalog);
  free(prep_req->schema);
  free(prep_req->table);
  free(prep_req->type);
  free(prep_req);
  free(req);
  scope.Close(Undefined());
}

void ODBC::UV_QueryAll(uv_work_t* req) {
  DEBUG_PRINTF("ODBC::UV_QueryAll\n");
  query_request* prep_req = (query_request *)(req->data);
  
  Parameter prm;
  SQLRETURN ret;
  
  uv_mutex_lock(&ODBC::g_odbcMutex);

  SQLAllocHandle( SQL_HANDLE_STMT, prep_req->dbo->m_hDBC, &prep_req->hSTMT );

  uv_mutex_unlock(&ODBC::g_odbcMutex);

  //check to see if should excute a direct or a parameter bound query
  if (!prep_req->paramCount) {
    // execute the query directly
    ret = SQLExecDirect( prep_req->hSTMT,(SQLCHAR *)prep_req->sql, strlen(prep_req->sql) );
  }
  else {
    // prepare statement, bind parameters and execute statement 
    ret = SQLPrepare(prep_req->hSTMT, (SQLCHAR *)prep_req->sql, strlen(prep_req->sql));
    
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
      for (int i = 0; i < prep_req->paramCount; i++) {
        prm = prep_req->params[i];
        
        DEBUG_PRINTF("ODBC::UV_QueryAll - param[%i]: c_type=%i type=%i "
                     "buffer_length=%i size=%i length=%i &length=%X decimals=%i\n", i, prm.c_type, prm.type, 
                     prm.buffer_length, prm.size, prm.length, &prep_req->params[i].length, prm.decimals);

        ret = SQLBindParameter( prep_req->hSTMT,    //StatementHandle
                                i + 1,              //ParameterNumber
                                SQL_PARAM_INPUT,    //InputOutputType
                                prm.c_type,         //ValueType
                                prm.type,           //ParameterType
                                prm.size,           //ColumnSize
                                prm.decimals,       //DecimalDigits
                                prm.buffer,         //ParameterValuePtr
                                prm.buffer_length,  //BufferLength
                                //using &prm.length did not work here...
                                &prep_req->params[i].length); //StrLen_or_IndPtr

        if (ret == SQL_ERROR) {
          break;
        }
      }

      if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        ret = SQLExecute(prep_req->hSTMT);
      }
    }
    
    // free parameters
    //
    for (int i = 0; i < prep_req->paramCount; i++) {
      if (prm = prep_req->params[i], prm.buffer != NULL) {
        switch (prm.c_type) {
          case SQL_C_CHAR:    free(prm.buffer);             break; 
          case SQL_C_SBIGINT: delete (int64_t *)prm.buffer; break;
          case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break;
          case SQL_C_BIT:     delete (bool    *)prm.buffer; break;
        }
      }
    }

    free(prep_req->params);
  }

  prep_req->result = ret; // this will be checked later in UV_AfterQuery
}

Handle<Value> ODBC::QueryAll(const Arguments& args) {
  HandleScope scope;

  REQ_STR_ARG(0, sql);
  
  Local<Function> cb; 

  ODBC* dbo = ObjectWrap::Unwrap<ODBC>(args.This());
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  query_request* prep_req = (query_request *) calloc(1, sizeof(query_request));

  if (!prep_req) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(String::New("Could not allocate enough memory")));
  }

  // populate prep_req->params if parameters were supplied
  //
  if (args.Length() > 2) {
      if ( !args[1]->IsArray() ) {
           return ThrowException(Exception::TypeError(
                      String::New("Argument 1 must be an Array"))
           );
      }
      else if ( !args[2]->IsFunction() ) {
           return ThrowException(Exception::TypeError(
                      String::New("Argument 2 must be a Function"))
           );
      }

      cb = Local<Function>::Cast(args[2]);
      prep_req->params = GetParametersFromArray(Local<Array>::Cast(args[1]), &prep_req->paramCount);
  }
  else {
      if ( !args[1]->IsFunction() ) {
           return ThrowException(Exception::TypeError(
                      String::New("Argument 1 must be a Function"))
           );
      }

      cb = Local<Function>::Cast(args[1]);

      prep_req->paramCount = 0;
  }

  prep_req->sql = (char *) malloc(sql.length() +1);
  prep_req->catalog = NULL;
  prep_req->schema = NULL;
  prep_req->table = NULL;
  prep_req->type = NULL;
  prep_req->column = NULL;
  prep_req->cb = Persistent<Function>::New(cb);
  
  strcpy(prep_req->sql, *sql);
  
  prep_req->dbo = dbo;
  work_req->data = prep_req;
  
  uv_queue_work(uv_default_loop(), work_req, UV_QueryAll, (uv_after_work_cb)UV_AfterQueryAll);

  dbo->Ref();

  return  scope.Close(Undefined());
}

void ODBC::UV_Tables(uv_work_t* req) {
  query_request* prep_req = (query_request *)(req->data);
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  SQLAllocStmt(prep_req->dbo->m_hDBC,&prep_req->hSTMT );
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
  SQLRETURN ret = SQLTables( 
    prep_req->hSTMT, 
    (SQLCHAR *) prep_req->catalog,   SQL_NTS, 
    (SQLCHAR *) prep_req->schema,   SQL_NTS, 
    (SQLCHAR *) prep_req->table,   SQL_NTS, 
    (SQLCHAR *) prep_req->type,   SQL_NTS
  );
  
  // this will be checked later in UV_AfterQuery
  prep_req->result = ret; 
}

Handle<Value> ODBC::Tables(const Arguments& args) {
  HandleScope scope;

  REQ_STR_OR_NULL_ARG(0, catalog);
  REQ_STR_OR_NULL_ARG(1, schema);
  REQ_STR_OR_NULL_ARG(2, table);
  REQ_STR_OR_NULL_ARG(3, type);
  Local<Function> cb = Local<Function>::Cast(args[4]);

  ODBC* dbo = ObjectWrap::Unwrap<ODBC>(args.Holder());
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  query_request* prep_req = (query_request *) calloc(1, sizeof(query_request));
  
  if (!prep_req) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(String::New("Could not allocate enough memory")));
  }

  prep_req->sql = NULL;
  prep_req->catalog = NULL;
  prep_req->schema = NULL;
  prep_req->table = NULL;
  prep_req->type = NULL;
  prep_req->column = NULL;
  prep_req->cb = Persistent<Function>::New(cb);

  if (!String::New(*catalog)->Equals(String::New("null"))) {
    prep_req->catalog = (char *) malloc(catalog.length() +1);
    strcpy(prep_req->catalog, *catalog);
  }
  
  if (!String::New(*schema)->Equals(String::New("null"))) {
    prep_req->schema = (char *) malloc(schema.length() +1);
    strcpy(prep_req->schema, *schema);
  }
  
  if (!String::New(*table)->Equals(String::New("null"))) {
    prep_req->table = (char *) malloc(table.length() +1);
    strcpy(prep_req->table, *table);
  }
  
  if (!String::New(*type)->Equals(String::New("null"))) {
    prep_req->type = (char *) malloc(type.length() +1);
    strcpy(prep_req->type, *type);
  }
  
  prep_req->dbo = dbo;
  work_req->data = prep_req;
  
  uv_queue_work(uv_default_loop(), work_req, UV_Tables, (uv_after_work_cb)UV_AfterQueryAll);

  dbo->Ref();

  return scope.Close(Undefined());
}

void ODBC::UV_Columns(uv_work_t* req) {
  query_request* prep_req = (query_request *)(req->data);
  
  SQLAllocStmt(prep_req->dbo->m_hDBC,&prep_req->hSTMT );
  
  SQLRETURN ret = SQLColumns( 
    prep_req->hSTMT, 
    (SQLCHAR *) prep_req->catalog,   SQL_NTS, 
    (SQLCHAR *) prep_req->schema,   SQL_NTS, 
    (SQLCHAR *) prep_req->table,   SQL_NTS, 
    (SQLCHAR *) prep_req->column,   SQL_NTS
  );
  
  // this will be checked later in UV_AfterQuery
  prep_req->result = ret;
}

Handle<Value> ODBC::Columns(const Arguments& args) {
  HandleScope scope;

  REQ_STR_OR_NULL_ARG(0, catalog);
  REQ_STR_OR_NULL_ARG(1, schema);
  REQ_STR_OR_NULL_ARG(2, table);
  REQ_STR_OR_NULL_ARG(3, column);
  Local<Function> cb = Local<Function>::Cast(args[4]);
  
  ODBC* dbo = ObjectWrap::Unwrap<ODBC>(args.Holder());
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  query_request* prep_req = (query_request *) calloc(1, sizeof(query_request));
  
  if (!prep_req) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(String::New("Could not allocate enough memory")));
  }

  prep_req->sql = NULL;
  prep_req->catalog = NULL;
  prep_req->schema = NULL;
  prep_req->table = NULL;
  prep_req->type = NULL;
  prep_req->column = NULL;
  prep_req->cb = Persistent<Function>::New(cb);

  if (!String::New(*catalog)->Equals(String::New("null"))) {
    prep_req->catalog = (char *) malloc(catalog.length() +1);
    strcpy(prep_req->catalog, *catalog);
  }
  
  if (!String::New(*schema)->Equals(String::New("null"))) {
    prep_req->schema = (char *) malloc(schema.length() +1);
    strcpy(prep_req->schema, *schema);
  }
  
  if (!String::New(*table)->Equals(String::New("null"))) {
    prep_req->table = (char *) malloc(table.length() +1);
    strcpy(prep_req->table, *table);
  }
  
  if (!String::New(*column)->Equals(String::New("null"))) {
    prep_req->column = (char *) malloc(column.length() +1);
    strcpy(prep_req->column, *column);
  }
  
  prep_req->dbo = dbo;
  work_req->data = prep_req;
  
  uv_queue_work(uv_default_loop(), work_req, UV_Columns, (uv_after_work_cb)UV_AfterQueryAll);
  
  dbo->Ref();

  return scope.Close(Undefined());
}

Column* ODBC::GetColumns(SQLHSTMT hStmt, short* colCount) {
  SQLRETURN ret;
  SQLSMALLINT buflen;

  //always reset colCount for the current result set to 0;
  *colCount = 0; 

  //get the number of columns in the result set
  ret = SQLNumResultCols(hStmt, colCount);
  
  if (!SQL_SUCCEEDED(ret)) {
    return new Column[0];
  }
  
  Column *columns = new Column[*colCount];

  for (int i = 0; i < *colCount; i++) {
    //save the index number of this column
    columns[i].index = i + 1;
    columns[i].name = new unsigned char[MAX_FIELD_SIZE];
    
    //set the first byte of name to \0 instead of memsetting the entire buffer
    columns[i].name[0] = '\0';
    
    //get the column name
    ret = SQLColAttribute( hStmt,
                           columns[i].index,
                           SQL_DESC_LABEL,
                           columns[i].name,
                           (SQLSMALLINT) MAX_FIELD_SIZE,
                           (SQLSMALLINT *) &buflen,
                           NULL);
    
    //store the len attribute
    columns[i].len = buflen;
    
    //get the column type and store it directly in column[i].type
    ret = SQLColAttribute( hStmt,
                           columns[i].index,
                           SQL_COLUMN_TYPE,
                           NULL,
                           0,
                           NULL,
                           &columns[i].type);
  }
  
  return columns;
}

void ODBC::FreeColumns(Column* columns, short* colCount) {
  for(int i = 0; i < *colCount; i++) {
      delete [] columns[i].name;
  }

  delete [] columns;
  
  *colCount = 0;
}

Handle<Value> ODBC::GetColumnValue( SQLHSTMT hStmt, Column column, 
                                        uint16_t* buffer, int bufferLength) {
  HandleScope scope;
  SQLLEN len = 0;
  
  struct tm timeInfo = { 0 };

  //reset the buffer
  buffer[0] = '\0';

  //TODO: SQLGetData can supposedly return multiple chunks, need to do this to 
  //retrieve large fields
  int ret; 
  
  switch ((int) column.type) {
    case SQL_NUMERIC :
    case SQL_DECIMAL :
    case SQL_INTEGER : 
    case SQL_SMALLINT :
    case SQL_BIGINT :
    case SQL_FLOAT :
    case SQL_REAL :
    case SQL_DOUBLE :
      ret = SQLGetData( hStmt, 
                        column.index, 
                        SQL_C_CHAR,
                        (char *) buffer, 
                        bufferLength, 
                        &len);
   
      DEBUG_PRINTF("ODBC::GetColumnValue - Numeric: index=%i name=%s type=%i len=%i ret=%i\n", 
                    column.index, column.name, column.type, len, ret);

      if(ret == SQL_NULL_DATA || len < 0) {
        return scope.Close(Null());
        //return Null();
      }
      else {
        return scope.Close(Number::New(atof((char *) buffer)));
        //return Number::New(atof((char *) buffer));
      }
    case SQL_DATETIME :
    case SQL_TIMESTAMP :
      //I am not sure if this is locale-safe or cross database safe, but it 
      //works for me on MSSQL
#ifdef _WIN32
      ret = SQLGetData( hStmt, 
                        column.index, 
                        SQL_C_CHAR,
                        (char *) buffer, 
                        bufferLength, 
                        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - W32 Timestamp: index=%i name=%s type=%i len=%i\n", 
                    column.index, column.name, column.type, len);

      if(ret == SQL_NULL_DATA || len < 0) {
        //return scope.Close(Null());
        return Null();
      }
      else {
        strptime((char *) buffer, "%Y-%m-%d %H:%M:%S", &timeInfo);

        //a negative value means that mktime() should use timezone information 
        //and system databases to attempt to determine whether DST is in effect 
        //at the specified time.
        timeInfo.tm_isdst = -1;
          
        //return scope.Close(Date::New(double(mktime(&timeInfo)) * 1000));
        return Date::New((double(mktime(&timeInfo)) * 1000));
      }
#else
      SQL_TIMESTAMP_STRUCT odbcTime;
      
      ret = SQLGetData( hStmt, 
                        column.index, 
                        SQL_C_TYPE_TIMESTAMP,
                        &odbcTime, 
                        bufferLength, 
                        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - Unix Timestamp: index=%i name=%s type=%i len=%i\n", 
                    column.index, column.name, column.type, len);

      if(ret == SQL_NULL_DATA || len < 0) {
        return scope.Close(Null());
        //return Null();
      }
      else {
        timeInfo.tm_year = odbcTime.year - 1900;
        timeInfo.tm_mon = odbcTime.month - 1;
        timeInfo.tm_mday = odbcTime.day;
        timeInfo.tm_hour = odbcTime.hour;
        timeInfo.tm_min = odbcTime.minute;
        timeInfo.tm_sec = odbcTime.second;

        //a negative value means that mktime() should use timezone information 
        //and system databases to attempt to determine whether DST is in effect 
        //at the specified time.
        timeInfo.tm_isdst = -1;
          
        return scope.Close(Date::New((double(timegm(&timeInfo)) * 1000) 
                          + (odbcTime.fraction / 1000000)));
//        return Date::New((double(timegm(&timeInfo)) * 1000) 
//                          + (odbcTime.fraction / 1000000));
      }
#endif
    case SQL_BIT :
      //again, i'm not sure if this is cross database safe, but it works for 
      //MSSQL
      ret = SQLGetData( hStmt, 
                        column.index, 
                        SQL_C_CHAR,
                        (char *) buffer, 
                        bufferLength, 
                        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - Bit: index=%i name=%s type=%i len=%i\n", 
                    column.index, column.name, column.type, len);

      if(ret == SQL_NULL_DATA || len < 0) {
        return scope.Close(Null());
        //return Null();
      }
      else {
        return scope.Close(Boolean::New(( *buffer == '0') ? false : true ));
        //return Boolean::New(( *buffer == '0') ? false : true );
      }
    default :
      ret = SQLGetData( hStmt,
                        column.index,
                        SQL_C_CHAR,
                        (char *) buffer,
                        bufferLength,
                        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - String: index=%i name=%s type=%i len=%i value=%s ret=%i bufferLength=%i\n", 
                    column.index, column.name, column.type, len,(char *) buffer, ret, bufferLength);

      if(ret == SQL_NULL_DATA || len < 0) {
        return scope.Close(Null());
        //return Null();
      }
      else {
        return scope.Close(String::New((char*) buffer));
        //return String::New((char*) buffer);
      }
  }
}

Handle<Value> ODBC::GetRecordTuple ( SQLHSTMT hStmt, Column* columns, 
                                         short* colCount, uint16_t* buffer,
                                         int bufferLength) {
  HandleScope scope;
  
  Local<Object> tuple = Object::New();
        
  for(int i = 0; i < *colCount; i++) {
    tuple->Set( String::New((const char *) columns[i].name),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
  }
  
  //return tuple;
  return scope.Close(tuple);
}

Handle<Value> ODBC::GetRecordArray ( SQLHSTMT hStmt, Column* columns, 
                                         short* colCount, uint16_t* buffer,
                                         int bufferLength) {
  HandleScope scope;
  
  Local<Array> array = Array::New();
        
  for(int i = 0; i < *colCount; i++) {
    array->Set( Integer::New(i),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
  }
  
  //return array;
  return scope.Close(array);
}

Parameter* ODBC::GetParametersFromArray (Local<Array> values, int *paramCount) {
  DEBUG_PRINTF("ODBC::GetParametersFromArray\n");
  *paramCount = values->Length();
  
  Parameter * params = new Parameter[*paramCount];

  for (int i = 0; i < *paramCount; i++) {
    Local<Value> value = values->Get(i);
    
    params[i].size          = 0;
    params[i].length        = SQL_NULL_DATA;
    params[i].buffer_length = 0;
    params[i].decimals      = 0;

    DEBUG_PRINTF("ODBC::GetParametersFromArray - &param[%i].length = %X\n", i, &params[i].length);

    if (value->IsString()) {
      String::Utf8Value string(value);

      params[i].c_type        = SQL_C_CHAR;
      params[i].type          = SQL_VARCHAR;
      params[i].buffer_length = string.length() + 1;
      params[i].buffer        = malloc(params[i].buffer_length);
      params[i].size          = params[i].buffer_length;
      params[i].length        = SQL_NTS;//params[i].buffer_length;

      strcpy((char*)params[i].buffer, *string);

      DEBUG_PRINTF("ODBC::GetParametersFromArray - IsString(): params[%i] "
                   "c_type=%i type=%i buffer_length=%i size=%i length=%i "
                   "value=%s\n", i, params[i].c_type, params[i].type,
                   params[i].buffer_length, params[i].size, params[i].length, 
                   (char*) params[i].buffer);
    }
    else if (value->IsNull()) {
      params[i].c_type = SQL_C_DEFAULT;
      params[i].type   = SQL_VARCHAR;
      params[i].length = SQL_NULL_DATA;

      DEBUG_PRINTF("ODBC::GetParametersFromArray - IsNull(): params[%i] "
                   "c_type=%i type=%i buffer_length=%i size=%i length=%i\n",
                   i, params[i].c_type, params[i].type,
                   params[i].buffer_length, params[i].size, params[i].length);
    }
    else if (value->IsInt32()) {
      int64_t  *number = new int64_t(value->IntegerValue());
      params[i].c_type = SQL_C_SBIGINT;
      params[i].type   = SQL_BIGINT;
      params[i].buffer = number;
      params[i].length = 0;
      
      DEBUG_PRINTF("ODBC::GetParametersFromArray - IsInt32(): params[%i] "
                   "c_type=%i type=%i buffer_length=%i size=%i length=%i\n",
                   i, params[i].c_type, params[i].type,
                   params[i].buffer_length, params[i].size, params[i].length);
    }
    else if (value->IsNumber()) {
      double   *number   = new double(value->NumberValue());
      params[i].c_type   = SQL_C_DOUBLE;
      params[i].type     = SQL_DECIMAL;
      params[i].buffer   = number; 
      params[i].length   = 0;
      params[i].decimals = 6; //idk, i just chose this randomly.
      params[i].size     = 10; //also just a guess

      DEBUG_PRINTF("ODBC::GetParametersFromArray - IsNumber(): params[%i] "
                   "c_type=%i type=%i buffer_length=%i size=%i length=%i\n",
                   i, params[i].c_type, params[i].type,
                   params[i].buffer_length, params[i].size, params[i].length);
    }
    else if (value->IsBoolean()) {
      bool *boolean    = new bool(value->BooleanValue());
      params[i].c_type = SQL_C_BIT;
      params[i].type   = SQL_BIT;
      params[i].buffer = boolean;
      params[i].length = 0;
      
      DEBUG_PRINTF("ODBC::GetParametersFromArray - IsBoolean(): params[%i] "
                   "c_type=%i type=%i buffer_length=%i size=%i length=%i\n",
                   i, params[i].c_type, params[i].type,
                   params[i].buffer_length, params[i].size, params[i].length);
    }
  } 
  
  return params;
}

extern "C" void init (v8::Handle<Object> target) {
  ODBC::Init(target);
  ODBCResult::Init(target);
}

NODE_MODULE(odbc_bindings, init)

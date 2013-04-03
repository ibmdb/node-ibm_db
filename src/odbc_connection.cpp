/*
  Copyright (c) 2013, Dan VerWeire <dverweire@gmail.com>
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
#include "odbc_connection.h"
#include "odbc_result.h"
#include "odbc_statement.h"

using namespace v8;
using namespace node;

Persistent<FunctionTemplate> ODBCConnection::constructor_template;

void ODBCConnection::Init(v8::Handle<Object> target) {
  DEBUG_PRINTF("ODBCConnection::Init\n");
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  // Constructor Template
  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->SetClassName(String::NewSymbol("ODBCConnection"));

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = constructor_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);
  
  // Properties
  //instance_template->SetAccessor(String::New("mode"), ModeGetter, ModeSetter);
  //instance_template->SetAccessor(String::New("connected"), ConnectedGetter);
  
  // Prototype Methods
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "open", Open);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "close", Close);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "createStatement", CreateStatement);

  // Attach the Database Constructor to the target object
  target->Set( v8::String::NewSymbol("ODBCConnection"),
               constructor_template->GetFunction());
  
  scope.Close(Undefined());
}

ODBCConnection::~ODBCConnection() {
  DEBUG_PRINTF("ODBCConnection::~ODBCConnection\n");
  this->Free();
}

void ODBCConnection::Free() {
  DEBUG_PRINTF("ODBCConnection::Free\n");
  if (m_hDBC) {
    uv_mutex_lock(&ODBC::g_odbcMutex);
    
    if (m_hDBC) {
      SQLDisconnect(m_hDBC);
      SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
      m_hDBC = NULL;
    }
    
    uv_mutex_unlock(&ODBC::g_odbcMutex);
  }
}

Handle<Value> ODBCConnection::New(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::New\n");
  HandleScope scope;
  
  REQ_EXT_ARG(0, js_henv);
  REQ_EXT_ARG(1, js_hdbc);
  
  HENV hENV = static_cast<HENV>(js_henv->Value());
  HDBC hDBC = static_cast<HENV>(js_hdbc->Value());
  
  ODBCConnection* conn = new ODBCConnection(hENV, hDBC);
  
  conn->Wrap(args.Holder());
  
  return scope.Close(args.Holder());
}

Handle<Value> ODBCConnection::ConnectedGetter(Local<String> property, const AccessorInfo &info) {
  HandleScope scope;

  ODBCConnection *obj = ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  return scope.Close(obj->connected ? True() : False());
}

/*
 * Open
 * 
 */

Handle<Value> ODBCConnection::Open(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::Open\n");
  HandleScope scope;

  REQ_STR_ARG(0, connection);
  REQ_FUN_ARG(1, cb);

  //get reference to the connection object
  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
  
  //create a uv work request
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
 
  //allocate our worker data
  open_connection_work_data* data = (open_connection_work_data *) 
    calloc(1, sizeof(open_connection_work_data) + connection.length());

  //copy the connection string to the work data
  strcpy(data->connection, *connection);
  data->cb = Persistent<Function>::New(cb);
  data->conn = conn;
  
  work_req->data = data;
  
  //queue the work
  uv_queue_work(uv_default_loop(), 
    work_req, 
    UV_Open, 
    (uv_after_work_cb)UV_AfterOpen);

  conn->Ref();

  return scope.Close(args.Holder());
}

void ODBCConnection::UV_Open(uv_work_t* req) {
  DEBUG_PRINTF("ODBCConnection::UV_Open\n");
  open_connection_work_data* data = (open_connection_work_data *)(req->data);
  
  ODBCConnection* self = data->conn->self();
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  
  //TODO: make this configurable
  SQLSetConnectOption( self->m_hDBC, SQL_LOGIN_TIMEOUT, 5 );

  char connstr[1024];

  //Attempt to connect
  int ret = SQLDriverConnect( 
    self->m_hDBC, 
    NULL,
    (SQLCHAR*) data->connection,
    strlen(data->connection),
    (SQLCHAR*) connstr,
    1024,
    NULL,
    SQL_DRIVER_NOPROMPT);

  if (SQL_SUCCEEDED(ret)) {
    HSTMT hStmt;
    
    //allocate a temporary statment
    ret = SQLAllocStmt(self->m_hDBC, &hStmt);

    //try to determine if the driver can handle
    //multiple recordsets
    ret = SQLGetFunctions(
      self->m_hDBC,
      SQL_API_SQLMORERESULTS, 
      &self->canHaveMoreResults);

    if (!SQL_SUCCEEDED(ret)) {
      self->canHaveMoreResults = 0;
    }
    
    //free the handle
    ret = SQLFreeHandle( SQL_HANDLE_STMT, hStmt);
  }
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
  data->result = ret;
}

void ODBCConnection::UV_AfterOpen(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCConnection::UV_AfterOpen\n");
  HandleScope scope;
  open_connection_work_data* data = (open_connection_work_data *)(req->data);
  
  Local<Value> argv[1];
  
  bool err = false;

  if (data->result) {
    err = true;

    SQLINTEGER i = 0;
    SQLINTEGER native;
    SQLSMALLINT len;
    SQLRETURN ret;
    char errorSQLState[7];
    char errorMessage[256];

    do {
      ret = SQLGetDiagRec(
        SQL_HANDLE_DBC, 
        data->conn->self()->m_hDBC,
        ++i, 
        (SQLCHAR *) errorSQLState,
        &native,
        (SQLCHAR *) errorMessage,
        sizeof(errorMessage),
        &len);

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
   data->conn->self()->connected = true;
    
    //only uv_ref if the connection was successful
#if NODE_VERSION_AT_LEAST(0, 7, 9)
    uv_ref((uv_handle_t *)&ODBC::g_async);
#else
    uv_ref(uv_default_loop());
#endif
  }
  
  TryCatch try_catch;

  data->conn->Unref();
  data->cb->Call(Context::GetCurrent()->Global(), err ? 1 : 0, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  data->cb.Dispose();
  
  free(data);
  free(req);
  scope.Close(Undefined());
}

/*
 * Close
 * 
 */

Handle<Value> ODBCConnection::Close(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::Close\n");
  HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  close_connection_work_data* data = (close_connection_work_data *) 
    (calloc(1, sizeof(close_connection_work_data)));

  data->cb = Persistent<Function>::New(cb);
  data->conn = conn;

  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(),
    work_req,
    UV_Close,
    (uv_after_work_cb)UV_AfterClose);

  conn->Ref();

  return scope.Close(Undefined());
}

void ODBCConnection::UV_Close(uv_work_t* req) {
  DEBUG_PRINTF("ODBCConnection::UV_Close\n");
  close_connection_work_data* data = (close_connection_work_data *)(req->data);
  ODBCConnection* conn = data->conn;
  
  //TODO: check to see if there are any open statements
  //on this connection
  
  conn->Free();
  
  data->result = 0;
}

void ODBCConnection::UV_AfterClose(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCConnection::UV_AfterClose\n");
  HandleScope scope;

  close_connection_work_data* data = (close_connection_work_data *)(req->data);

  ODBCConnection* conn = data->conn;
  
  Local<Value> argv[1];
  bool err = false;
  
  if (data->result) {
    err = true;
    argv[0] = Exception::Error(String::New("Error closing database"));
  }
  else {
    conn->connected = false;
    
    //only unref if the connection was closed
#if NODE_VERSION_AT_LEAST(0, 7, 9)
    uv_unref((uv_handle_t *)&ODBC::g_async);
#else
    uv_unref(uv_default_loop());
#endif
  }

  TryCatch try_catch;

  data->conn->Unref();
  data->cb->Call(Context::GetCurrent()->Global(), err ? 1 : 0, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  data->cb.Dispose();

  free(data);
  free(req);
  scope.Close(Undefined());
}


/*
 * CreateStatement
 * 
 */

Handle<Value> ODBCConnection::CreateStatement(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::CreateStatement\n");
  HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
    
  //initialize work request
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  //initialize our data
  create_statement_work_data* data = 
    (create_statement_work_data *) (calloc(1, sizeof(create_statement_work_data)));

  data->cb = Persistent<Function>::New(cb);
  data->conn = conn;

  work_req->data = data;
  
  uv_queue_work(uv_default_loop(), work_req, UV_CreateStatement, (uv_after_work_cb)UV_AfterCreateStatement);

  conn->Ref();

  return scope.Close(Undefined());
}

void ODBCConnection::UV_CreateStatement(uv_work_t* req) {
  DEBUG_PRINTF("ODBCConnection::UV_CreateStatement\n");
  
  //get our work data
  create_statement_work_data* data = (create_statement_work_data *)(req->data);
  
  uv_mutex_lock(&ODBC::g_odbcMutex);

  //allocate a new statment handle
  SQLAllocHandle( SQL_HANDLE_STMT, 
                  data->conn->m_hDBC, 
                  &data->hSTMT );

  uv_mutex_unlock(&ODBC::g_odbcMutex);
}

void ODBCConnection::UV_AfterCreateStatement(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCConnection::UV_AfterCreateStatement\n");
  HandleScope scope;

  create_statement_work_data* data = (create_statement_work_data *)(req->data);
  
  Local<Value> args[3];
  args[0] = External::New(data->conn->m_hENV);
  args[1] = External::New(data->conn->m_hDBC);
  args[2] = External::New(data->hSTMT);
  
  Persistent<Object> js_result(ODBCStatement::constructor_template->
                            GetFunction()->NewInstance(3, args));

  args[0] = Local<Value>::New(Null());
  args[1] = Local<Object>::New(js_result);

  data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  
  data->conn->Unref();
  data->cb.Dispose();

  free(data);
  free(req);
  
  scope.Close(Undefined());
}

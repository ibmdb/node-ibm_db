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
  instance_template->SetAccessor(String::New("connected"), ConnectedGetter);
  
  // Prototype Methods
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "open", Open);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "openSync", OpenSync);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "close", Close);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "createStatement", CreateStatement);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "createStatementSync", CreateStatementSync);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "query", Query);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "querySync", QuerySync);
  
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "beginTransactionSync", BeginTransactionSync);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "endTransactionSync", EndTransactionSync);
  
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "columns", Columns);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "tables", Tables);
  
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

    Local<Object> objError = ODBC::GetSQLDiagRecError(data->conn->self()->m_hDBC);
    
    argv[0] = objError;
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
 * OpenSync
 */

Handle<Value> ODBCConnection::OpenSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::OpenSync\n");
  HandleScope scope;

  REQ_STR_ARG(0, connection);

  //get reference to the connection object
  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
 
  Local<Object> objError;
  SQLRETURN ret;
  bool err = false;
  char* connectionString;
  char connstr[1024];
  
  //allocate memory for the connection string
  connectionString = (char *) malloc(connection.length() +1);
  
  //copy the connection string to the work data
  strcpy(connectionString, *connection);
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  
  //TODO: make this configurable
  SQLSetConnectOption( conn->m_hDBC, SQL_LOGIN_TIMEOUT, 5 );

  //Attempt to connect
  ret = SQLDriverConnect( 
    conn->m_hDBC, 
    NULL,
    (SQLCHAR*) connectionString,
    strlen(connectionString),
    (SQLCHAR*) connstr,
    1024,
    NULL,
    SQL_DRIVER_NOPROMPT);

  if (!SQL_SUCCEEDED(ret)) {
    err = true;
    
    objError = ODBC::GetSQLDiagRecError(conn->self()->m_hDBC);
  }
  else {
    HSTMT hStmt;
    
    //allocate a temporary statment
    ret = SQLAllocStmt(conn->m_hDBC, &hStmt);

    //try to determine if the driver can handle
    //multiple recordsets
    ret = SQLGetFunctions(
      conn->m_hDBC,
      SQL_API_SQLMORERESULTS, 
      &conn->canHaveMoreResults);

    if (!SQL_SUCCEEDED(ret)) {
      conn->canHaveMoreResults = 0;
    }
  
    //free the handle
    ret = SQLFreeHandle( SQL_HANDLE_STMT, hStmt);
    
    conn->self()->connected = true;
    
    //only uv_ref if the connection was successful
    #if NODE_VERSION_AT_LEAST(0, 7, 9)
      uv_ref((uv_handle_t *)&ODBC::g_async);
    #else
      uv_ref(uv_default_loop());
    #endif
  }
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
  free(connectionString);

  if (err) {
    ThrowException(objError);
    return scope.Close(False());
  }
  else {
    return scope.Close(True());
  }
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
 * CreateStatementSync
 * 
 */

Handle<Value> ODBCConnection::CreateStatementSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::CreateStatementSync\n");
  HandleScope scope;

  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
   
  HSTMT hSTMT;

  SQLAllocHandle(
    SQL_HANDLE_STMT, 
    conn->m_hDBC, 
    &hSTMT);
  
  Local<Value> params[3];
  params[0] = External::New(conn->m_hENV);
  params[1] = External::New(conn->m_hDBC);
  params[2] = External::New(hSTMT);
  
  Persistent<Object> js_result(ODBCStatement::constructor_template->
                            GetFunction()->NewInstance(3, params));
  
  return scope.Close(js_result);
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
  
  uv_queue_work(
    uv_default_loop(), 
    work_req, 
    UV_CreateStatement, 
    (uv_after_work_cb)UV_AfterCreateStatement);

  conn->Ref();

  return scope.Close(Undefined());
}

void ODBCConnection::UV_CreateStatement(uv_work_t* req) {
  DEBUG_PRINTF("ODBCConnection::UV_CreateStatement\n");
  
  //get our work data
  create_statement_work_data* data = (create_statement_work_data *)(req->data);
  
  uv_mutex_lock(&ODBC::g_odbcMutex);

  DEBUG_PRINTF("ODBCConnection::UV_CreateStatement m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    data->conn->m_hENV,
    data->conn->m_hDBC,
    data->hSTMT
  );
  
  //allocate a new statment handle
  SQLAllocHandle( SQL_HANDLE_STMT, 
                  data->conn->m_hDBC, 
                  &data->hSTMT);

  DEBUG_PRINTF("ODBCConnection::UV_CreateStatement m_hDBC=%X m_hDBC=%X m_hSTMT=%X\n",
    data->conn->m_hENV,
    data->conn->m_hDBC,
    data->hSTMT
  );
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
}

void ODBCConnection::UV_AfterCreateStatement(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCConnection::UV_AfterCreateStatement\n");
  HandleScope scope;

  create_statement_work_data* data = (create_statement_work_data *)(req->data);

  DEBUG_PRINTF("ODBCConnection::UV_AfterCreateStatement m_hDBC=%X m_hDBC=%X hSTMT=%X\n",
    data->conn->m_hENV,
    data->conn->m_hDBC,
    data->hSTMT
  );
  
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

/*
 * Query
 */

Handle<Value> ODBCConnection::Query(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::Query\n");
  
  HandleScope scope;

  REQ_STR_ARG(0, sql);
  
  Local<Function> cb; 

  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  query_work_data* data = (query_work_data *) calloc(1, sizeof(query_work_data));

  // populate data->params if parameters were supplied
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
      
      data->params = ODBC::GetParametersFromArray(
        Local<Array>::Cast(args[1]),
        &data->paramCount);
  }
  else {
      if ( !args[1]->IsFunction() ) {
           return ThrowException(Exception::TypeError(
                      String::New("Argument 1 must be a Function"))
           );
      }

      cb = Local<Function>::Cast(args[1]);

      data->paramCount = 0;
  }

  data->sql = (char *) malloc(sql.length() +1);
  data->cb = Persistent<Function>::New(cb);
  
  strcpy(data->sql, *sql);
  
  data->conn = conn;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(),
    work_req, 
    UV_Query, 
    (uv_after_work_cb)UV_AfterQuery);

  conn->Ref();

  return  scope.Close(Undefined());
}

void ODBCConnection::UV_Query(uv_work_t* req) {
  DEBUG_PRINTF("ODBCConnection::UV_Query\n");
  
  query_work_data* data = (query_work_data *)(req->data);
  
  Parameter prm;
  SQLRETURN ret;
  
  uv_mutex_lock(&ODBC::g_odbcMutex);

  //allocate a new statment handle
  SQLAllocHandle( SQL_HANDLE_STMT, 
                  data->conn->m_hDBC, 
                  &data->hSTMT );

  uv_mutex_unlock(&ODBC::g_odbcMutex);

  //check to see if should excute a direct or a parameter bound query
  if (!data->paramCount) {
    // execute the query directly
    ret = SQLExecDirect(
      data->hSTMT,
      (SQLCHAR *) data->sql, 
      strlen(data->sql));
  }
  else {
    // prepare statement, bind parameters and execute statement 
    ret = SQLPrepare(
      data->hSTMT,
      (SQLCHAR *) data->sql, 
      strlen(data->sql));
    
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
      for (int i = 0; i < data->paramCount; i++) {
        prm = data->params[i];
        
        DEBUG_PRINTF(
          "ODBCConnection::UV_Query - param[%i]: c_type=%i type=%i "
          "buffer_length=%i size=%i length=%i &length=%X\n", i, prm.c_type, prm.type, 
          prm.buffer_length, prm.size, prm.length, &data->params[i].length);

        ret = SQLBindParameter(
          data->hSTMT,              //StatementHandle
          i + 1,                    //ParameterNumber
          SQL_PARAM_INPUT,          //InputOutputType
          prm.c_type,               //ValueType
          prm.type,                 //ParameterType
          prm.size,                 //ColumnSize
          0,                        //DecimalDigits
          prm.buffer,               //ParameterValuePtr
          prm.buffer_length,        //BufferLength
          //using &prm.length did not work here...
          &data->params[i].length); //StrLen_or_IndPtr
        
        if (ret == SQL_ERROR) {break;}
      }

      if (SQL_SUCCEEDED(ret)) {
        ret = SQLExecute(data->hSTMT);
      }
    }
    
    // free parameters
    for (int i = 0; i < data->paramCount; i++) {
      if (prm = data->params[i], prm.buffer != NULL) {
        switch (prm.c_type) {
          case SQL_C_CHAR:    free(prm.buffer);             break; 
          case SQL_C_LONG:    delete (int64_t *)prm.buffer; break;
          case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break;
          case SQL_C_BIT:     delete (bool    *)prm.buffer; break;
        }
      }
    }
    free(data->params);
  }

  // this will be checked later in UV_AfterQuery
  data->result = ret;
}

void ODBCConnection::UV_AfterQuery(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBCConnection::UV_AfterQuery\n");
  
  HandleScope scope;
  
  query_work_data* data = (query_work_data *)(req->data);

  //check to see if there was an error during execution
  if(data->result == SQL_ERROR) {
    ODBC::CallbackSQLError(
      data->conn->m_hENV,
      data->conn->m_hDBC,
      data->hSTMT,
      data->cb);
  }
  else {
    Local<Value> args[3];
    args[0] = External::New(data->conn->m_hENV);
    args[1] = External::New(data->conn->m_hDBC);
    args[2] = External::New(data->hSTMT);
    Persistent<Object> js_result(ODBCResult::constructor_template->
                              GetFunction()->NewInstance(3, args));

    args[0] = Local<Value>::New(Null());
    args[1] = Local<Object>::New(js_result);
    
    data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  }
  
  TryCatch try_catch;
  
  data->conn->Unref();
  
  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
  
  data->cb.Dispose();
  
  free(data->sql);
  free(data->catalog);
  free(data->schema);
  free(data->table);
  free(data->type);
  free(data->column);
  free(data);
  free(req);
  
  scope.Close(Undefined());
}


/*
 * QuerySync
 */

Handle<Value> ODBCConnection::QuerySync(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::QuerySync\n");
  
  HandleScope scope;

  REQ_STR_ARG(0, sql);

  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
  
  Local<Object> objError = Object::New();
  Parameter* params;
  Parameter prm;
  SQLRETURN ret;
  HSTMT hSTMT;
  int paramCount = 0;
  char* sqlString;
  
  // populate params if parameters were supplied
  if (args.Length() > 1) {
      if ( !args[1]->IsArray() ) {
           return ThrowException(Exception::TypeError(
                      String::New("Argument 1 must be an Array"))
           );
      }
  
      params = ODBC::GetParametersFromArray(
        Local<Array>::Cast(args[1]),
        &paramCount);
  }
  
  sqlString = (char *) malloc(sql.length() +1);

  strcpy(sqlString, *sql);

  uv_mutex_lock(&ODBC::g_odbcMutex);

  //allocate a new statment handle
  SQLAllocHandle( SQL_HANDLE_STMT, 
                  conn->m_hDBC, 
                  &hSTMT );

  uv_mutex_unlock(&ODBC::g_odbcMutex);

  //check to see if should excute a direct or a parameter bound query
  if (!paramCount) {
    // execute the query directly
    ret = SQLExecDirect(
      hSTMT,
      (SQLCHAR *) sqlString, 
      strlen(sqlString));
  }
  else {
    // prepare statement, bind parameters and execute statement 
    ret = SQLPrepare(
      hSTMT,
      (SQLCHAR *) sqlString, 
      strlen(sqlString));
    
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
      for (int i = 0; i < paramCount; i++) {
        prm = params[i];
        
        DEBUG_PRINTF(
          "ODBCConnection::UV_Query - param[%i]: c_type=%i type=%i "
          "buffer_length=%i size=%i length=%i &length=%X\n", i, prm.c_type, prm.type, 
          prm.buffer_length, prm.size, prm.length, &params[i].length);

        ret = SQLBindParameter(
          hSTMT,              //StatementHandle
          i + 1,                    //ParameterNumber
          SQL_PARAM_INPUT,          //InputOutputType
          prm.c_type,               //ValueType
          prm.type,                 //ParameterType
          prm.size,                 //ColumnSize
          0,                        //DecimalDigits
          prm.buffer,               //ParameterValuePtr
          prm.buffer_length,        //BufferLength
          //using &prm.length did not work here...
          &params[i].length); //StrLen_or_IndPtr
        
        if (ret == SQL_ERROR) {break;}
      }

      if (SQL_SUCCEEDED(ret)) {
        ret = SQLExecute(hSTMT);
      }
    }
    
    // free parameters
    for (int i = 0; i < paramCount; i++) {
      if (prm = params[i], prm.buffer != NULL) {
        switch (prm.c_type) {
          case SQL_C_CHAR:    free(prm.buffer);             break; 
          case SQL_C_LONG:    delete (int64_t *)prm.buffer; break;
          case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break;
          case SQL_C_BIT:     delete (bool    *)prm.buffer; break;
        }
      }
    }
    
    free(params);
  }

  //check to see if there was an error during execution
  if(ret == SQL_ERROR) {
    objError = ODBC::GetSQLError(
      conn->m_hENV,
      conn->m_hDBC,
      hSTMT,
      (char *) "[node-odbc] Error in ODBCConnection::QuerySync"
    );
    
    ThrowException(objError);
    
    return scope.Close(Undefined());
  }
  else {
    Local<Value> args[3];
    args[0] = External::New(conn->m_hENV);
    args[1] = External::New(conn->m_hDBC);
    args[2] = External::New(hSTMT);
    Persistent<Object> js_result(ODBCResult::constructor_template->
                              GetFunction()->NewInstance(3, args));

    return scope.Close(js_result);
  }
}

/*
 * Tables
 */

Handle<Value> ODBCConnection::Tables(const Arguments& args) {
  HandleScope scope;

  REQ_STR_OR_NULL_ARG(0, catalog);
  REQ_STR_OR_NULL_ARG(1, schema);
  REQ_STR_OR_NULL_ARG(2, table);
  REQ_STR_OR_NULL_ARG(3, type);
  Local<Function> cb = Local<Function>::Cast(args[4]);

  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  query_work_data* data = 
    (query_work_data *) calloc(1, sizeof(query_work_data));
  
  if (!data) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(String::New("Could not allocate enough memory")));
  }

  data->sql = NULL;
  data->catalog = NULL;
  data->schema = NULL;
  data->table = NULL;
  data->type = NULL;
  data->column = NULL;
  data->cb = Persistent<Function>::New(cb);

  if (!String::New(*catalog)->Equals(String::New("null"))) {
    data->catalog = (char *) malloc(catalog.length() +1);
    strcpy(data->catalog, *catalog);
  }
  
  if (!String::New(*schema)->Equals(String::New("null"))) {
    data->schema = (char *) malloc(schema.length() +1);
    strcpy(data->schema, *schema);
  }
  
  if (!String::New(*table)->Equals(String::New("null"))) {
    data->table = (char *) malloc(table.length() +1);
    strcpy(data->table, *table);
  }
  
  if (!String::New(*type)->Equals(String::New("null"))) {
    data->type = (char *) malloc(type.length() +1);
    strcpy(data->type, *type);
  }
  
  data->conn = conn;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(), 
    work_req, 
    UV_Tables, 
    (uv_after_work_cb) UV_AfterQuery);

  conn->Ref();

  return scope.Close(Undefined());
}

void ODBCConnection::UV_Tables(uv_work_t* req) {
  query_work_data* data = (query_work_data *)(req->data);
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  
  SQLAllocStmt(data->conn->m_hDBC, &data->hSTMT );
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
  SQLRETURN ret = SQLTables( 
    data->hSTMT, 
    (SQLCHAR *) data->catalog,   SQL_NTS, 
    (SQLCHAR *) data->schema,   SQL_NTS, 
    (SQLCHAR *) data->table,   SQL_NTS, 
    (SQLCHAR *) data->type,   SQL_NTS
  );
  
  // this will be checked later in UV_AfterQuery
  data->result = ret; 
}



/*
 * Columns
 */

Handle<Value> ODBCConnection::Columns(const Arguments& args) {
  HandleScope scope;

  REQ_STR_OR_NULL_ARG(0, catalog);
  REQ_STR_OR_NULL_ARG(1, schema);
  REQ_STR_OR_NULL_ARG(2, table);
  REQ_STR_OR_NULL_ARG(3, column);
  
  Local<Function> cb = Local<Function>::Cast(args[4]);
  
  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
  
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  query_work_data* data = (query_work_data *) calloc(1, sizeof(query_work_data));
  
  if (!data) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(String::New("Could not allocate enough memory")));
  }

  data->sql = NULL;
  data->catalog = NULL;
  data->schema = NULL;
  data->table = NULL;
  data->type = NULL;
  data->column = NULL;
  data->cb = Persistent<Function>::New(cb);

  if (!String::New(*catalog)->Equals(String::New("null"))) {
    data->catalog = (char *) malloc(catalog.length() +1);
    strcpy(data->catalog, *catalog);
  }
  
  if (!String::New(*schema)->Equals(String::New("null"))) {
    data->schema = (char *) malloc(schema.length() +1);
    strcpy(data->schema, *schema);
  }
  
  if (!String::New(*table)->Equals(String::New("null"))) {
    data->table = (char *) malloc(table.length() +1);
    strcpy(data->table, *table);
  }
  
  if (!String::New(*column)->Equals(String::New("null"))) {
    data->column = (char *) malloc(column.length() +1);
    strcpy(data->column, *column);
  }
  
  data->conn = conn;
  work_req->data = data;
  
  uv_queue_work(
    uv_default_loop(),
    work_req, 
    UV_Columns, 
    (uv_after_work_cb)UV_AfterQuery);
  
  conn->Ref();

  return scope.Close(Undefined());
}

void ODBCConnection::UV_Columns(uv_work_t* req) {
  query_work_data* data = (query_work_data *)(req->data);
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  
  SQLAllocStmt(data->conn->m_hDBC, &data->hSTMT );
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
  SQLRETURN ret = SQLColumns( 
    data->hSTMT, 
    (SQLCHAR *) data->catalog,   SQL_NTS, 
    (SQLCHAR *) data->schema,   SQL_NTS, 
    (SQLCHAR *) data->table,   SQL_NTS, 
    (SQLCHAR *) data->column,   SQL_NTS
  );
  
  // this will be checked later in UV_AfterQuery
  data->result = ret;
}

/*
 * BeginTransactionSync
 * 
 */

Handle<Value> ODBCConnection::BeginTransactionSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::BeginTransactionSync\n");
  HandleScope scope;

  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
  
  SQLRETURN ret;

  //set the connection manual commits
  ret = SQLSetConnectAttr(
    conn->m_hDBC,
    SQL_ATTR_AUTOCOMMIT,
    (SQLPOINTER) SQL_AUTOCOMMIT_OFF,
    SQL_NTS);
  
  if (!SQL_SUCCEEDED(ret)) {
    Local<Object> objError = ODBC::GetSQLDiagRecError(conn->m_hDBC);
    
    ThrowException(objError);
    
    return scope.Close(False());
  }
  
  return scope.Close(True());
}

/*
 * EndTransactionSync
 * 
 */

Handle<Value> ODBCConnection::EndTransactionSync(const Arguments& args) {
  DEBUG_PRINTF("ODBCConnection::EndTransactionSync\n");
  HandleScope scope;

  ODBCConnection* conn = ObjectWrap::Unwrap<ODBCConnection>(args.Holder());
  
  REQ_BOOL_ARG(0, rollback);
  
  Local<Object> objError;
  SQLRETURN ret;
  bool error = false;
  SQLSMALLINT completionType = (rollback->Value()) 
    ? SQL_ROLLBACK
    : SQL_COMMIT
    ;
  
  //Call SQLEndTran
  ret = SQLEndTran(
    SQL_HANDLE_DBC,
    conn->m_hDBC,
    completionType);
  
  //check how the transaction went
  if (!SQL_SUCCEEDED(ret)) {
    error = true;
    
    objError = ODBC::GetSQLDiagRecError(conn->m_hDBC);
  }
  
  //Reset the connection back to autocommit
  ret = SQLSetConnectAttr(
    conn->m_hDBC,
    SQL_ATTR_AUTOCOMMIT,
    (SQLPOINTER) SQL_AUTOCOMMIT_ON,
    SQL_NTS);
  
  //check how setting the connection attr went
  //but only process the code if an error has not already
  //occurred. If an error occurred during SQLEndTran,
  //that is the error that we want to throw.
  if (!SQL_SUCCEEDED(ret) && !error) {
    //TODO: if this also failed, we really should
    //be restarting the connection or something to deal with this state
    error = true;
    
    objError = ODBC::GetSQLDiagRecError(conn->m_hDBC);
  }
  
  if (error) {
    ThrowException(objError);
    
    return scope.Close(False());
  }
  else {
    return scope.Close(True());
  }
}
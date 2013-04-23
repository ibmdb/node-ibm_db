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
  constructor_template->SetClassName(String::NewSymbol("ODBC"));

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = constructor_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);
  
  // Constants
  NODE_DEFINE_CONSTANT(constructor_template, SQL_CLOSE);
  NODE_DEFINE_CONSTANT(constructor_template, SQL_DROP);
  NODE_DEFINE_CONSTANT(constructor_template, SQL_UNBIND);
  NODE_DEFINE_CONSTANT(constructor_template, SQL_RESET_PARAMS);
  NODE_DEFINE_CONSTANT(constructor_template, SQL_DESTROY); //SQL_DESTROY is non-standard
  NODE_DEFINE_CONSTANT(constructor_template, FETCH_ARRAY);
  NODE_DEFINE_CONSTANT(constructor_template, FETCH_OBJECT);
  
  // Prototype Methods
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "createConnection", CreateConnection);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "createConnectionSync", CreateConnectionSync);

  // Attach the Database Constructor to the target object
  target->Set( v8::String::NewSymbol("ODBC"),
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
  if (m_hEnv) {
    uv_mutex_lock(&ODBC::g_odbcMutex);
    
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
  dbo->m_hEnv = NULL;
  
  int ret = SQLAllocEnv( &dbo->m_hEnv );
  
  //TODO: check if ret succeeded, if not, throw error to javascript land
  if (!SQL_SUCCEEDED(ret)) {
    //TODO: do something.
  }
  
  return scope.Close(args.Holder());
}

void ODBC::WatcherCallback(uv_async_t *w, int revents) {
  DEBUG_PRINTF("ODBC::WatcherCallback\n");
  //i don't know if we need to do anything here
}

/*
 * CreateConnection
 */

Handle<Value> ODBC::CreateConnection(const Arguments& args) {
  DEBUG_PRINTF("ODBC::CreateConnection\n");
  HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBC* dbo = ObjectWrap::Unwrap<ODBC>(args.Holder());
  
  //initialize work request
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  //initialize our data
  create_connection_work_data* data = 
    (create_connection_work_data *) (calloc(1, sizeof(create_connection_work_data)));

  data->cb = Persistent<Function>::New(cb);
  data->dbo = dbo;

  work_req->data = data;
  
  uv_queue_work(uv_default_loop(), work_req, UV_CreateConnection, (uv_after_work_cb)UV_AfterCreateConnection);

  dbo->Ref();

  return scope.Close(Undefined());
}

void ODBC::UV_CreateConnection(uv_work_t* req) {
  DEBUG_PRINTF("ODBC::UV_CreateConnection\n");
  
  //get our work data
  create_connection_work_data* data = (create_connection_work_data *)(req->data);
  
  uv_mutex_lock(&ODBC::g_odbcMutex);

  //allocate a new connection handle
  int ret = SQLAllocConnect(data->dbo->m_hEnv, &data->hDBC);
  
  if (!SQL_SUCCEEDED(ret)) {
   //TODO: do something. 
  }
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
}

void ODBC::UV_AfterCreateConnection(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBC::UV_AfterCreateConnection\n");
  HandleScope scope;

  create_connection_work_data* data = (create_connection_work_data *)(req->data);
  
  Local<Value> args[2];
  args[0] = External::New(data->dbo->m_hEnv);
  args[1] = External::New(data->hDBC);
  
  Persistent<Object> js_result(ODBCConnection::constructor_template->
                            GetFunction()->NewInstance(2, args));

  args[0] = Local<Value>::New(Null());
  args[1] = Local<Object>::New(js_result);

  data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  
  data->dbo->Unref();
  data->cb.Dispose();

  free(data);
  free(req);
  
  scope.Close(Undefined());
}

/*
 * CreateConnectionSync
 */

Handle<Value> ODBC::CreateConnectionSync(const Arguments& args) {
  DEBUG_PRINTF("ODBC::CreateConnectionSync\n");
  HandleScope scope;

  ODBC* dbo = ObjectWrap::Unwrap<ODBC>(args.Holder());
   
  HDBC hDBC;
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  
  //allocate a new connection handle
  SQLRETURN ret = SQLAllocConnect(dbo->m_hEnv, &hDBC);
  
  if (!SQL_SUCCEEDED(ret)) {
    //TODO: do something!
  }
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);

  Local<Value> params[2];
  params[0] = External::New(dbo->m_hEnv);
  params[1] = External::New(hDBC);

  Persistent<Object> js_result(ODBCConnection::constructor_template->
                            GetFunction()->NewInstance(2, params));

  return scope.Close(js_result);
}

/*
 * GetColumns
 */

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
    //TODO:that's a lot of memory for each field name....
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

/*
 * FreeColumns
 */

void ODBC::FreeColumns(Column* columns, short* colCount) {
  for(int i = 0; i < *colCount; i++) {
      delete [] columns[i].name;
  }

  delete [] columns;
  
  *colCount = 0;
}

/*
 * GetColumnValue
 */

Handle<Value> ODBC::GetColumnValue( SQLHSTMT hStmt, Column column, 
                                        uint16_t* buffer, int bufferLength) {
  //HandleScope scope;
  SQLLEN len = 0;

  //reset the buffer
  buffer[0] = '\0';

  //TODO: SQLGetData can supposedly return multiple chunks, need to do this to 
  //retrieve large fields
  int ret; 
  
  switch ((int) column.type) {
    case SQL_INTEGER : 
    case SQL_SMALLINT :
    case SQL_TINYINT : {
        long value;
        
        ret = SQLGetData(
          hStmt, 
          column.index, 
          SQL_C_SLONG,
          &value, 
          sizeof(value), 
          &len);
        
        DEBUG_PRINTF("ODBC::GetColumnValue - Integer: index=%i name=%s type=%i len=%i ret=%i\n", 
                    column.index, column.name, column.type, len, ret);
        
        if (ret == SQL_NULL_DATA || len < 0) {
          //return scope.Close(Null());
          return Null();
        }
        else {
          //return scope.Close(Integer::New(value));
          return Integer::New(value);
        }
      }
      break;
    case SQL_NUMERIC :
    case SQL_DECIMAL :
    case SQL_BIGINT :
    case SQL_FLOAT :
    case SQL_REAL :
    case SQL_DOUBLE : {
        double value;
        
        ret = SQLGetData(
          hStmt, 
          column.index, 
          SQL_C_DOUBLE,
          &value, 
          sizeof(value), 
          &len);
        
         DEBUG_PRINTF("ODBC::GetColumnValue - Integer: index=%i name=%s type=%i len=%i ret=%i\n", 
                    column.index, column.name, column.type, len, ret);
        
        if(ret == SQL_NULL_DATA || len < 0) {
           //return scope.Close(Null());
          return Null();
        }
        else {
          //return scope.Close(Number::New(value));
          return Number::New(value);
        }
      }
      break;
    case SQL_DATETIME :
    case SQL_TIMESTAMP : {
      struct tm timeInfo = { 0 };
      //I am not sure if this is locale-safe or cross database safe, but it 
      //works for me on MSSQL
#ifdef _WIN32
      ret = SQLGetData(
        hStmt, 
        column.index, 
        SQL_C_CHAR,
        (char *) buffer, 
        bufferLength, 
        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - W32 Timestamp: index=%i name=%s type=%i len=%i\n", 
                    column.index, column.name, column.type, len);

      if(ret == SQL_NULL_DATA || len < 0) {
        return Null();
      }
      else {
        strptime((char *) buffer, "%Y-%m-%d %H:%M:%S", &timeInfo);

        //a negative value means that mktime() should use timezone information 
        //and system databases to attempt to determine whether DST is in effect 
        //at the specified time.
        timeInfo.tm_isdst = -1;
          
        return Date::New((double(mktime(&timeInfo)) * 1000));
      }
#else
      SQL_TIMESTAMP_STRUCT odbcTime;
      
      ret = SQLGetData(
        hStmt, 
        column.index, 
        SQL_C_TYPE_TIMESTAMP,
        &odbcTime, 
        bufferLength, 
        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - Unix Timestamp: index=%i name=%s type=%i len=%i\n", 
                    column.index, column.name, column.type, len);

      if(ret == SQL_NULL_DATA || len < 0) {
        //return scope.Close(Null());
        return Null();
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
          
        //return scope.Close(Date::New((double(timegm(&timeInfo)) * 1000) 
        //                  + (odbcTime.fraction / 1000000)));
        return Date::New((double(timegm(&timeInfo)) * 1000) 
                          + (odbcTime.fraction / 1000000));
      }
#endif
    } break;
    case SQL_BIT :
      //again, i'm not sure if this is cross database safe, but it works for 
      //MSSQL
      ret = SQLGetData(
        hStmt, 
        column.index, 
        SQL_C_CHAR,
        (char *) buffer, 
        bufferLength, 
        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - Bit: index=%i name=%s type=%i len=%i\n", 
                    column.index, column.name, column.type, len);

      if(ret == SQL_NULL_DATA || len < 0) {
        //return scope.Close(Null());
        return Null();
      }
      else {
        //return scope.Close(Boolean::New(( *buffer == '0') ? false : true ));
        return Boolean::New(( *buffer == '0') ? false : true );
      }
    default :
      ret = SQLGetData(
        hStmt,
        column.index,
        SQL_C_CHAR,
        (char *) buffer,
        bufferLength,
        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - String: index=%i name=%s type=%i len=%i value=%s ret=%i bufferLength=%i\n", 
                    column.index, column.name, column.type, len,(char *) buffer, ret, bufferLength);

      if(ret == SQL_NULL_DATA || len < 0) {
        //return scope.Close(Null());
        return Null();
      }
      else {
        //return scope.Close(String::New((char*) buffer));
        return String::New((char*) buffer);
      }
  }
}

/*
 * GetRecordTuple
 */

Local<Object> ODBC::GetRecordTuple ( SQLHSTMT hStmt, Column* columns, 
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

/*
 * GetRecordArray
 */

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

/*
 * GetParametersFromArray
 */

Parameter* ODBC::GetParametersFromArray (Local<Array> values, int *paramCount) {
  DEBUG_PRINTF("ODBC::GetParametersFromArray\n");
  *paramCount = values->Length();
  
  Parameter* params = (Parameter *) malloc(*paramCount * sizeof(Parameter));

  for (int i = 0; i < *paramCount; i++) {
    Local<Value> value = values->Get(i);
    
    params[i].size          = 0;
    params[i].length        = SQL_NULL_DATA;
    params[i].buffer_length = 0;
    params[i].decimals      = 0;

    DEBUG_PRINTF("ODBC::GetParametersFromArray - &param[%i].length = %X\n",
                 i, &params[i].length);

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
                   "c_type=%i type=%i buffer_length=%i size=%i length=%i "
                   "value=%lld\n", i, params[i].c_type, params[i].type,
                   params[i].buffer_length, params[i].size, params[i].length,
                   *number);
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

/*
 * CallbackSQLError
 */

Handle<Value> ODBC::CallbackSQLError (HENV hENV, 
                                     HDBC hDBC, 
                                     HSTMT hSTMT, 
                                     Persistent<Function> cb) {
  HandleScope scope;
  
  Local<Object> objError = ODBC::GetSQLError(
    hENV, 
    hDBC, 
    hSTMT,
    (char *) "[node-odbc] Error in some module"
  );
  
  Local<Value> args[1];
  args[0] = objError;
  cb->Call(Context::GetCurrent()->Global(), 1, args);
  
  return scope.Close(Undefined());
}

/*
 * GetSQLError
 */

Local<Object> ODBC::GetSQLError (HENV hENV, 
                                HDBC hDBC, 
                                HSTMT hSTMT,
                                char* message) {
  HandleScope scope;
  
  Local<Object> objError = Object::New();
  
  char errorMessage[512];
  char errorSQLState[128];
  
  SQLError(
    hENV,
    hDBC,
    hSTMT,
    (SQLCHAR *) errorSQLState,
    NULL,
    (SQLCHAR *) errorMessage,
    sizeof(errorMessage), 
    NULL);
  
  objError->Set(String::New("state"), String::New(errorSQLState));
  objError->Set(String::New("error"), String::New(message));
  objError->Set(String::New("message"), String::New(errorMessage));
  
  return scope.Close(objError);
}

/*
 * GetSQLDiagRecError
 */

Local<Object> ODBC::GetSQLDiagRecError (HDBC hDBC) {
  HandleScope scope;
  
  Local<Object> objError = Object::New();
  
  SQLINTEGER i = 0;
  SQLINTEGER native;
  SQLSMALLINT len;
  SQLRETURN ret;
  char errorSQLState[7];
  char errorMessage[256];

  do {
    ret = SQLGetDiagRec(
      SQL_HANDLE_DBC, 
      hDBC,
      ++i, 
      (SQLCHAR *) errorSQLState,
      &native,
      (SQLCHAR *) errorMessage,
      sizeof(errorMessage),
      &len);

    if (SQL_SUCCEEDED(ret)) {
      objError->Set(String::New("error"), String::New("[node-odbc] SQL_ERROR"));
      objError->Set(String::New("message"), String::New(errorMessage));
      objError->Set(String::New("state"), String::New(errorSQLState));
    }
  } while( ret == SQL_SUCCESS );
  
  return scope.Close(objError);
}

/*
 * GetAllRecordsSync
 */

Local<Array> ODBC::GetAllRecordsSync (HENV hENV, 
                                     HDBC hDBC, 
                                     HSTMT hSTMT,
                                     uint16_t* buffer,
                                     int bufferLength) {
  DEBUG_PRINTF("ODBC::GetAllRecordsSync\n");
  
  HandleScope scope;
  
  Local<Object> objError = Object::New();
  
  int count = 0;
  int errorCount = 0;
  short colCount = 0;
  
  Column* columns = GetColumns(hSTMT, &colCount);
  
  Local<Array> rows = Array::New();
  
  //loop through all records
  while (true) {
    SQLRETURN ret = SQLFetch(hSTMT);
    
    //check to see if there was an error
    if (ret == SQL_ERROR)  {
      //TODO: what do we do when we actually get an error here...
      //should we throw??
      
      errorCount++;
      
      objError = ODBC::GetSQLError(
        hENV, 
        hDBC, 
        hSTMT,
        (char *) "[node-odbc] Error in ODBC::GetAllRecordsSync"
      );
      
      break;
    }
    
    //check to see if we are at the end of the recordset
    if (ret == SQL_NO_DATA) {
      ODBC::FreeColumns(columns, &colCount);
      
      break;
    }

    rows->Set(
      Integer::New(count), 
      ODBC::GetRecordTuple(
        hSTMT,
        columns,
        &colCount,
        buffer,
        bufferLength)
    );

    count++;
  }
  //TODO: what do we do about errors!?!
  scope.Close(rows);
}

extern "C" void init (v8::Handle<Object> target) {
  ODBC::Init(target);
  ODBCResult::Init(target);
  ODBCConnection::Init(target);
  ODBCStatement::Init(target);
}

NODE_MODULE(odbc_bindings, init)

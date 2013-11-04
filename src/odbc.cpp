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

#ifdef dynodbc
#include "dynodbc.h"
#endif

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
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  
  int ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &dbo->m_hEnv);
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
  if (!SQL_SUCCEEDED(ret)) {
    DEBUG_PRINTF("ODBC::New - ERROR ALLOCATING ENV HANDLE!!\n");
    
    Local<Object> objError = ODBC::GetSQLError(SQL_HANDLE_ENV, dbo->m_hEnv);
    
    ThrowException(objError);
  }
  
  SQLSetEnvAttr(dbo->m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_UINTEGER);
  
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
  data->result = SQLAllocHandle(SQL_HANDLE_DBC, data->dbo->m_hEnv, &data->hDBC);
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
}

void ODBC::UV_AfterCreateConnection(uv_work_t* req, int status) {
  DEBUG_PRINTF("ODBC::UV_AfterCreateConnection\n");
  HandleScope scope;

  create_connection_work_data* data = (create_connection_work_data *)(req->data);
  
  TryCatch try_catch;
  
  if (!SQL_SUCCEEDED(data->result)) {
    Local<Value> args[1];
    
    args[0] = ODBC::GetSQLError(SQL_HANDLE_ENV, data->dbo->m_hEnv);
    
    data->cb->Call(Context::GetCurrent()->Global(), 1, args);
  }
  else {
    Local<Value> args[2];
    args[0] = External::New(data->dbo->m_hEnv);
    args[1] = External::New(data->hDBC);
    
    Local<Object> js_result(ODBCConnection::constructor_template->
                              GetFunction()->NewInstance(2, args));

    args[0] = Local<Value>::New(Null());
    args[1] = Local<Object>::New(js_result);

    data->cb->Call(Context::GetCurrent()->Global(), 2, args);
  }
  
  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  
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
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, dbo->m_hEnv, &hDBC);
  
  if (!SQL_SUCCEEDED(ret)) {
    //TODO: do something!
  }
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);

  Local<Value> params[2];
  params[0] = External::New(dbo->m_hEnv);
  params[1] = External::New(hDBC);

  Local<Object> js_result(ODBCConnection::constructor_template->
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
#ifdef STRICT_COLUMN_NAMES
                           SQL_DESC_NAME,
#else
                           SQL_DESC_LABEL,
#endif
                           columns[i].name,
                           (SQLSMALLINT) MAX_FIELD_SIZE,
                           (SQLSMALLINT *) &buflen,
                           NULL);
    
    //store the len attribute
    columns[i].len = buflen;
    
    //get the column type and store it directly in column[i].type
    ret = SQLColAttribute( hStmt,
                           columns[i].index,
                           SQL_DESC_TYPE,
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
  HandleScope scope;
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
        
        if (len == SQL_NULL_DATA) {
          return scope.Close(Null());
          //return Null();
        }
        else {
          return scope.Close(Integer::New(value));
          //return Integer::New(value);
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
        
         DEBUG_PRINTF("ODBC::GetColumnValue - Number: index=%i name=%s type=%i len=%i ret=%i val=%f\n", 
                    column.index, column.name, column.type, len, ret, value);
        
        if(len == SQL_NULL_DATA) {
          return scope.Close(Null());
          //return Null();
        }
        else {
          return scope.Close(Number::New(value));
          //return Number::New(value);
        }
      }
      break;
    case SQL_DATETIME :
    case SQL_TIMESTAMP : {
      //I am not sure if this is locale-safe or cross database safe, but it 
      //works for me on MSSQL
#ifdef _WIN32
      struct tm timeInfo = {};

      ret = SQLGetData(
        hStmt, 
        column.index, 
        SQL_C_CHAR,
        (char *) buffer, 
        bufferLength, 
        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - W32 Timestamp: index=%i name=%s type=%i len=%i\n", 
                    column.index, column.name, column.type, len);

      if(len == SQL_NULL_DATA) {
        return scope.Close(Null());
        //return Null();
      }
      else {
        strptime((char *) buffer, "%Y-%m-%d %H:%M:%S", &timeInfo);

        //a negative value means that mktime() should use timezone information 
        //and system databases to attempt to determine whether DST is in effect 
        //at the specified time.
        timeInfo.tm_isdst = -1;
          
        //return Date::New((double(mktime(&timeInfo)) * 1000));
        return scope.Close(Date::New((double(mktime(&timeInfo)) * 1000)));
      }
#else
      struct tm timeInfo = { 
        tm_sec : 0
        , tm_min : 0
        , tm_hour : 0
        , tm_mday : 0
        , tm_mon : 0
        , tm_year : 0
        , tm_wday : 0
        , tm_yday : 0
        , tm_isdst : 0
        , tm_gmtoff : 0
        , tm_zone : 0
      };

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

      if(len == SQL_NULL_DATA) {
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
#ifdef TIMEGM
        return scope.Close(Date::New((double(timegm(&timeInfo)) * 1000)
                          + (odbcTime.fraction / 1000000)));
#else
        return scope.Close(Date::New((double(timelocal(&timeInfo)) * 1000)
                          + (odbcTime.fraction / 1000000)));
#endif
        //return Date::New((double(timegm(&timeInfo)) * 1000) 
        //                  + (odbcTime.fraction / 1000000));
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

      if(len == SQL_NULL_DATA) {
        return scope.Close(Null());
        //return Null();
      }
      else {
        return scope.Close(Boolean::New(( *buffer == '0') ? false : true ));
        //return Boolean::New(( *buffer == '0') ? false : true );
      }
    default :
      Local<String> str;
      int count = 0;
      
      do {
        ret = SQLGetData(
          hStmt,
          column.index,
          SQL_C_TCHAR,
          (char *) buffer,
          bufferLength,
          &len);

        DEBUG_PRINTF("ODBC::GetColumnValue - String: index=%i name=%s type=%i len=%i value=%s ret=%i bufferLength=%i\n", 
                      column.index, column.name, column.type, len,(char *) buffer, ret, bufferLength);

        if(len == SQL_NULL_DATA) {
          return scope.Close(Null());
          //return Null();
        }
        
        if (SQL_NO_DATA == ret) {
          //we have captured all of the data
          break;
        }
        else if (SQL_SUCCEEDED(ret)) {
          //we have not captured all of the data yet
          
          if (count == 0) {
            //no concatenation required, this is our first pass
#ifdef UNICODE
            str = String::New((uint16_t*) buffer);
#else
            str = String::New((char *) buffer);
#endif
          }
          else {
            //we need to concatenate
#ifdef UNICODE
            str = String::Concat(str, String::New((uint16_t*) buffer));
#else
            str = String::Concat(str, String::New((char *) buffer));
#endif
          }
          
          count += 1;
        }
        else {
          //an error has occured
          //possible values for ret are SQL_ERROR (-1) and SQL_INVALID_HANDLE (-2)

          //If we have an invalid handle, then stuff is way bad and we should abort
          //immediately. Memory errors are bound to follow as we must be in an
          //inconsisant state.
          assert(ret != SQL_INVALID_HANDLE);

          //Not sure if throwing here will work out well for us but we can try
          //since we should have a valid handle and the error is something we 
          //can look into
          return ThrowException(ODBC::GetSQLError(
             SQL_HANDLE_STMT,
             hStmt,
             (char *) "[node-odbc] Error in ODBC::GetColumnValue"
           ));
          
          break;
        }
      } while (true);
      
      return scope.Close(str);
      //return str;
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
#ifdef UNICODE
    tuple->Set( String::New((uint16_t *) columns[i].name),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
#else
    tuple->Set( String::New((const char *) columns[i].name),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
#endif
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
      Local<String> string = value->ToString();
      int length = string->Length();
      
      params[i].c_type        = SQL_C_TCHAR;
#ifdef UNICODE
      params[i].type          = (length >= 8000) ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
      params[i].buffer_length = (length * sizeof(uint16_t)) + sizeof(uint16_t);
#else
      params[i].type          = (length >= 8000) ? SQL_LONGVARCHAR : SQL_VARCHAR;
      params[i].buffer_length = string->Utf8Length() + 1;
#endif
      params[i].buffer        = malloc(params[i].buffer_length);
      params[i].size          = params[i].buffer_length;
      params[i].length        = SQL_NTS;//params[i].buffer_length;

#ifdef UNICODE
      string->Write((uint16_t *) params[i].buffer);
#else
      string->WriteUtf8((char *) params[i].buffer);
#endif

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
      double *number   = new double(value->NumberValue());
      
      params[i].c_type        = SQL_C_DOUBLE;
      params[i].type          = SQL_DECIMAL;
      params[i].buffer        = number;
      params[i].buffer_length = sizeof(double);
      params[i].length        = params[i].buffer_length;
      params[i].decimals      = 7;
      params[i].size          = sizeof(double);

      DEBUG_PRINTF("ODBC::GetParametersFromArray - IsNumber(): params[%i] "
                  "c_type=%i type=%i buffer_length=%i size=%i length=%i "
		  "value=%f\n",
                  i, params[i].c_type, params[i].type,
                  params[i].buffer_length, params[i].size, params[i].length,
		  *number);
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

Handle<Value> ODBC::CallbackSQLError (SQLSMALLINT handleType,
                                      SQLHANDLE handle, 
                                      Persistent<Function> cb) {
  HandleScope scope;
  
  return scope.Close(CallbackSQLError(
    handleType,
    handle,
    (char *) "[node-odbc] SQL_ERROR",
    cb));
}

Handle<Value> ODBC::CallbackSQLError (SQLSMALLINT handleType,
                                      SQLHANDLE handle,
                                      char* message,
                                      Persistent<Function> cb) {
  HandleScope scope;
  
  Local<Object> objError = ODBC::GetSQLError(
    handleType, 
    handle, 
    message
  );
  
  Local<Value> args[1];
  args[0] = objError;
  cb->Call(Context::GetCurrent()->Global(), 1, args);
  
  return scope.Close(Undefined());
}

/*
 * GetSQLError
 */

Local<Object> ODBC::GetSQLError (SQLSMALLINT handleType, SQLHANDLE handle) {
  HandleScope scope;
  
  return scope.Close(GetSQLError(
    handleType,
    handle,
    (char *) "[node-odbc] SQL_ERROR"));
}

Local<Object> ODBC::GetSQLError (SQLSMALLINT handleType, SQLHANDLE handle, char* message) {
  HandleScope scope;
  
  DEBUG_PRINTF("ODBC::GetSQLError : handleType=%i, handle=%p\n", handleType, handle);
  
  Local<Object> objError = Object::New();
  
  SQLINTEGER i = 0;
  SQLINTEGER native;
  
  SQLSMALLINT len;
  SQLINTEGER numfields;
  SQLRETURN ret;
  char errorSQLState[14];
  char errorMessage[512];

  ret = SQLGetDiagField(
    handleType,
    handle,
    0,
    SQL_DIAG_NUMBER,
    &numfields,
    SQL_IS_INTEGER,
    &len);

  // Windows seems to define SQLINTEGER as long int, unixodbc as just int... %i should cover both
  DEBUG_PRINTF("ODBC::GetSQLError : called SQLGetDiagField; ret=%i\n", ret);
  
  for (i = 0; i < numfields; i++){
    DEBUG_PRINTF("ODBC::GetSQLError : calling SQLGetDiagRec; i=%i, numfields=%i\n", i, numfields);
    
    ret = SQLGetDiagRec(
      handleType, 
      handle,
      i + 1, 
      (SQLTCHAR *) errorSQLState,
      &native,
      (SQLTCHAR *) errorMessage,
      sizeof(errorMessage),
      &len);
    
    DEBUG_PRINTF("ODBC::GetSQLError : after SQLGetDiagRec; i=%i\n", i);

    if (SQL_SUCCEEDED(ret)) {
      DEBUG_TPRINTF(SQL_T("ODBC::GetSQLError : errorMessage=%s, errorSQLState=%s\n"), errorMessage, errorSQLState);
      
      objError->Set(String::New("error"), String::New(message));
#ifdef UNICODE
      objError->SetPrototype(Exception::Error(String::New((uint16_t *) errorMessage)));
      objError->Set(String::New("message"), String::New((uint16_t *) errorMessage));
      objError->Set(String::New("state"), String::New((uint16_t *) errorSQLState));
#else
      objError->SetPrototype(Exception::Error(String::New(errorMessage)));
      objError->Set(String::New("message"), String::New(errorMessage));
      objError->Set(String::New("state"), String::New(errorSQLState));
#endif
    } else if (ret == SQL_NO_DATA) {
      break;
    }
  }
  
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
        SQL_HANDLE_STMT, 
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
  //we throw them
  return scope.Close(rows);
}

#ifdef dynodbc
Handle<Value> ODBC::LoadODBCLibrary(const Arguments& args) {
  HandleScope scope;
  
  REQ_STR_ARG(0, js_library);
  
  bool result = DynLoadODBC(*js_library);
  
  return scope.Close((result) ? True() : False());
}
#endif

extern "C" void init (v8::Handle<Object> target) {
#ifdef dynodbc
  target->Set(String::NewSymbol("loadODBCLibrary"),
        FunctionTemplate::New(ODBC::LoadODBCLibrary)->GetFunction());
#endif
  
  ODBC::Init(target);
  ODBCResult::Init(target);
  ODBCConnection::Init(target);
  ODBCStatement::Init(target);
}

NODE_MODULE(odbc_bindings, init)

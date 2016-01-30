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

Nan::Persistent<Function> ODBC::constructor;

void ODBC::Init(v8::Handle<Object> exports) {
  DEBUG_PRINTF("ODBC::Init\n");
  Nan::HandleScope scope;

  Local<FunctionTemplate> constructor_template = Nan::New<FunctionTemplate>(New);

  // Constructor Template
  constructor_template->SetClassName(Nan::New("ODBC").ToLocalChecked());

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = constructor_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);
  
  // Constants
#if (NODE_MODULE_VERSION < NODE_0_12_MODULE_VERSION)

#else

#endif
  PropertyAttribute constant_attributes = static_cast<PropertyAttribute>(ReadOnly | DontDelete);
  constructor_template->Set(Nan::New<String>("SQL_CLOSE").ToLocalChecked(), Nan::New<Number>(SQL_CLOSE), constant_attributes);
  constructor_template->Set(Nan::New<String>("SQL_DROP").ToLocalChecked(), Nan::New<Number>(SQL_DROP), constant_attributes);
  constructor_template->Set(Nan::New<String>("SQL_UNBIND").ToLocalChecked(), Nan::New<Number>(SQL_UNBIND), constant_attributes);
  constructor_template->Set(Nan::New<String>("SQL_RESET_PARAMS").ToLocalChecked(), Nan::New<Number>(SQL_RESET_PARAMS), constant_attributes);
  constructor_template->Set(Nan::New<String>("SQL_DESTROY").ToLocalChecked(), Nan::New<Number>(SQL_DESTROY), constant_attributes);
  constructor_template->Set(Nan::New<String>("FETCH_ARRAY").ToLocalChecked(), Nan::New<Number>(FETCH_ARRAY), constant_attributes);
  NODE_ODBC_DEFINE_CONSTANT(constructor_template, FETCH_OBJECT);
  
  // Prototype Methods
  Nan::SetPrototypeMethod(constructor_template, "createConnection", CreateConnection);
  Nan::SetPrototypeMethod(constructor_template, "createConnectionSync", CreateConnectionSync);

  // Attach the Database Constructor to the target object
  constructor.Reset(constructor_template->GetFunction());
  exports->Set(Nan::New("ODBC").ToLocalChecked(),
               constructor_template->GetFunction());
  
#if NODE_VERSION_AT_LEAST(0, 7, 9)
  // Initialize uv_async so that we can prevent node from exiting
  //uv_async_init( uv_default_loop(),
  //               &ODBC::g_async,
  //               ODBC::WatcherCallback);
  
  // Not sure if the init automatically calls uv_ref() because there is weird
  // behavior going on. When ODBC::Init is called which initializes the 
  // uv_async_t g_async above, there seems to be a ref which will keep it alive
  // but we only want this available so that we can uv_ref() later on when
  // we have a connection.
  // so to work around this, I am possibly mistakenly calling uv_unref() once
  // so that there are no references on the loop.
  //uv_unref((uv_handle_t *)&ODBC::g_async);
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
      m_hEnv = (SQLHENV)NULL;      
    }

    uv_mutex_unlock(&ODBC::g_odbcMutex);
  }
}

NAN_METHOD(ODBC::New) {
  DEBUG_PRINTF("ODBC::New\n");
  Nan::HandleScope scope;
  ODBC* dbo = new ODBC();
  
  dbo->Wrap(info.Holder());

  dbo->m_hEnv = (SQLHENV)NULL;
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  
  // Initialize the Environment handle
  int ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &dbo->m_hEnv);
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
  if (!SQL_SUCCEEDED(ret)) {
    DEBUG_PRINTF("ODBC::New - ERROR ALLOCATING ENV HANDLE!!\n");
    
    Local<Value> objError = ODBC::GetSQLError(SQL_HANDLE_ENV, dbo->m_hEnv);
    
    return Nan::ThrowError(objError);
  }
  
  // Use ODBC 3.x behavior
  SQLSetEnvAttr(dbo->m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_UINTEGER);
  
  info.GetReturnValue().Set(info.Holder());
}

//void ODBC::WatcherCallback(uv_async_t *w, int revents) {
//  DEBUG_PRINTF("ODBC::WatcherCallback\n");
//  //i don't know if we need to do anything here
//}

/*
 * CreateConnection
 */

NAN_METHOD(ODBC::CreateConnection) {
  DEBUG_PRINTF("ODBC::CreateConnection\n");
  Nan::HandleScope scope;

  Local<Function> cb = info[0].As<Function>();
  Nan::Callback *callback = new Nan::Callback(cb);
  //REQ_FUN_ARG(0, cb);

  ODBC* dbo = Nan::ObjectWrap::Unwrap<ODBC>(info.Holder());
  
  //initialize work request
  uv_work_t* work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
  
  //initialize our data
  create_connection_work_data* data = 
    (create_connection_work_data *) (calloc(1, sizeof(create_connection_work_data)));

  data->cb = callback;
  data->dbo = dbo;

  work_req->data = data;
  
  uv_queue_work(uv_default_loop(), work_req, UV_CreateConnection, (uv_after_work_cb)UV_AfterCreateConnection);

  dbo->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
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
  Nan::HandleScope scope;

  create_connection_work_data* data = (create_connection_work_data *)(req->data);
  
  Nan::TryCatch try_catch;
  
  if (!SQL_SUCCEEDED(data->result)) {
    Local<Value> info[1];
    
    info[0] = ODBC::GetSQLError(SQL_HANDLE_ENV, data->dbo->m_hEnv);
    
    data->cb->Call(1, info);
  }
  else {
    Local<Value> info[2];
    info[0] = Nan::New<External>((void*)(intptr_t)data->dbo->m_hEnv);
    info[1] = Nan::New<External>((void*)(intptr_t)data->hDBC);
    
    Local<Object> js_result = Nan::New<Function>(ODBCConnection::constructor)->NewInstance(2, info);

    info[0] = Nan::Null();
    info[1] = js_result;

    data->cb->Call(2, info);
  }
  
  if (try_catch.HasCaught()) {
      Nan::FatalException(try_catch);
  }

  
  data->dbo->Unref();
  delete data->cb;

  free(data);
  free(req);
}

/*
 * CreateConnectionSync
 */

NAN_METHOD(ODBC::CreateConnectionSync) {
  DEBUG_PRINTF("ODBC::CreateConnectionSync\n");
  Nan::HandleScope scope;

  ODBC* dbo = Nan::ObjectWrap::Unwrap<ODBC>(info.Holder());
   
  SQLHDBC hDBC;
  
  uv_mutex_lock(&ODBC::g_odbcMutex);
  
  //allocate a new connection handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, dbo->m_hEnv, &hDBC);
  
  if (!SQL_SUCCEEDED(ret)) {
    //TODO: do something!
  }
  
  uv_mutex_unlock(&ODBC::g_odbcMutex);

  Local<Value> params[2];
  params[0] = Nan::New<External>((void*)(intptr_t)dbo->m_hEnv);
  params[1] = Nan::New<External>((void*)(intptr_t)hDBC);

  Local<Object> js_result = Nan::New<Function>(ODBCConnection::constructor)->NewInstance(2, params);

  info.GetReturnValue().Set(js_result);
}

/*
 * GetColumns
 */

Column* ODBC::GetColumns(SQLHSTMT hStmt, short* colCount) {
  SQLRETURN ret;
  SQLSMALLINT buflen;
  SQLSMALLINT typebuflen;

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
                           SQL_DESC_CONCISE_TYPE,
                           NULL,
                           0,
                           NULL,
                           &columns[i].type);
						   
	columns[i].type_name = new unsigned char[(MAX_FIELD_SIZE)];
    
    //set the first byte of type_name to \0 instead of memsetting the entire buffer
    columns[i].type_name[0] = '\0';
	
	ret = SQLColAttribute( hStmt,
                           columns[i].index,
                           SQL_DESC_TYPE_NAME,
                           columns[i].type_name,
                           (SQLSMALLINT) (MAX_FIELD_SIZE),
                           (SQLSMALLINT *) &typebuflen,
                           NULL);
  }
  
  return columns;
}

/*
 * FreeColumns
 */

void ODBC::FreeColumns(Column* columns, short* colCount) {
  for(int i = 0; i < *colCount; i++) {
      delete [] columns[i].name;
      delete [] columns[i].type_name;
  }

  delete [] columns;
  
  *colCount = 0;
}

/*
 * GetColumnValue
 */

Handle<Value> ODBC::GetColumnValue( SQLHSTMT hStmt, Column column, 
                                        uint16_t* buffer, int bufferLength) {
  Nan::EscapableHandleScope scope;
  SQLLEN len = 0;

  //reset the buffer
  buffer[0] = '\0';

  //TODO: SQLGetData can supposedly return multiple chunks, need to do this to 
  //retrieve large fields
  int ret; 
  DEBUG_PRINTF("Column Type : %i\t%i\t%i\t%i\n",column.type, SQL_DATETIME, SQL_TIMESTAMP, SQL_TYPE_TIME);
  switch ((int) column.type) {
    case SQL_INTEGER : 
    case SQL_SMALLINT :
    case SQL_TINYINT : {
        SQLINTEGER value;
        
        ret = SQLGetData(
          hStmt, 
          column.index, 
          SQL_C_SLONG,
          &value, 
          sizeof(value), 
          &len);
        
        DEBUG_PRINTF("ODBC::GetColumnValue - Integer: index=%i name=%s type=%i len=%i ret=%i\n", 
                    column.index, column.name, column.type, len, ret);
        
        if ((int)len == SQL_NULL_DATA) {
          return scope.Escape(Nan::Null());
        }
        else if((int)len == sizeof(int)){
          return scope.Escape(Nan::New<Number>((int)value));
        }
        else if((int)len == sizeof(short)){
          return scope.Escape(Nan::New<Number>((short)value));
        }
        else if((int)len == sizeof(long)){
          return scope.Escape(Nan::New<Number>((long)value));
        }
        else {
          return scope.Escape(Nan::New<Number>(value));
        }
      }
      break;
	/*
	case SQL_NUMERIC :
    case SQL_DECIMAL :
    case SQL_BIGINT :
    */
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
        
        if((int)len == SQL_NULL_DATA) {
          return scope.Escape(Nan::Null());
          //return Null();
        }
        else {
          return scope.Escape(Nan::New<Number>(value));
          //return Number::New(value);
        }
      }
      break;
    case SQL_DATETIME :
    case SQL_TIMESTAMP : {
      //I am not sure if this is locale-safe or cross database safe, but it 
      //works for me on MSSQL

       SQL_TIMESTAMP_STRUCT odbcTime;

#ifdef _WIN32
      struct tm timeInfo = {};

      ret = SQLGetData(hStmt, column.index, SQL_C_CHAR, 
              &odbcTime, bufferLength, &len);
#else
  #ifdef _AIX
      struct tm timeInfo = {0,0,0,0,0,0,0,0,0};
  #else
      struct tm timeInfo = {0,0,0,0,0,0,0,0,0,0,0};
  #endif
      ret = SQLGetData(hStmt, column.index, SQL_C_TYPE_TIMESTAMP, 
              &odbcTime, bufferLength, &len);
#endif

      DEBUG_PRINTF("ODBC::GetColumnValue - Unix Timestamp: index=%i name=%s type=%i len=%i\n", column.index, column.name, column.type, len);

      if((int)len == SQL_NULL_DATA) {
        return scope.Escape(Nan::Null());
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
        return scope.Escape(Nan::New<Date>((double(timegm(&timeInfo)) * 1000) + (odbcTime.fraction / 1000000)).ToLocalChecked());
#else
        return scope.Escape(Nan::New<Date>((double(mktime(&timeInfo)) * 1000) + (odbcTime.fraction / 1000000)).ToLocalChecked());

#endif
      }
    } 
    
    break;
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

      if((int)len == SQL_NULL_DATA) {
        return scope.Escape(Nan::Null());
        //return Null();
      }
      else {
        return scope.Escape(Nan::New((*buffer == '0') ? false : true));
      }
	case SQL_TYPE_TIME:
		DEBUG_PRINTF("SQL_TIME SELECTED");
    case SQL_NUMERIC :
    case SQL_DECIMAL :
    case SQL_BIGINT :
		DEBUG_PRINTF("BIG NUMERIC VALUE SELECTED SELECTED");
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

        if((int)len == SQL_NULL_DATA) {
          return scope.Escape(Nan::Null());
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
            str = Nan::New((uint16_t*) buffer).ToLocalChecked();
#else
            str = Nan::New((char *) buffer).ToLocalChecked();
#endif
          }
          else {
            //we need to concatenate
#ifdef UNICODE
            str = String::Concat(str, Nan::New((uint16_t*) buffer).ToLocalChecked());
#else
            str = String::Concat(str, Nan::New((char *) buffer).ToLocalChecked());
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
          Nan::ThrowError(ODBC::GetSQLError(
             SQL_HANDLE_STMT,
             hStmt,
             (char *) "[node-odbc] Error in ODBC::GetColumnValue"
           ));
          return scope.Escape(Nan::Undefined());
          break;
        }
      } while (true);
      
      return scope.Escape(str);
  }
}

/*
 * GetRecordTuple
 */

Local<Object> ODBC::GetRecordTuple ( SQLHSTMT hStmt, Column* columns, 
                                         short* colCount, uint16_t* buffer,
                                         int bufferLength) {
  Nan::EscapableHandleScope scope;
  
  Local<Object> tuple = Nan::New<Object>();
        
  for(int i = 0; i < *colCount; i++) {
#ifdef UNICODE
    tuple->Set( Nan::New((uint16_t *) columns[i].name).ToLocalChecked(),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
#else
    tuple->Set( Nan::New((const char *) columns[i].name).ToLocalChecked(),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
#endif
  }
  
  return scope.Escape(tuple);
}

/*
 * GetRecordArray
 */

Local<Value> ODBC::GetRecordArray ( SQLHSTMT hStmt, Column* columns, 
                                         short* colCount, uint16_t* buffer,
                                         int bufferLength) {
  Nan::EscapableHandleScope scope;
  
  Local<Array> array = Nan::New<Array>();
        
  for(int i = 0; i < *colCount; i++) {
    array->Set( Nan::New(i),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
  }
  
  return scope.Escape(array);
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
                                      Nan::Callback* cb) {
  Nan::EscapableHandleScope scope;
  
  Local<Value> objError = CallbackSQLError(
    handleType,
    handle,
    (char *) "[node-odbc] SQL_ERROR",
    cb);
  return scope.Escape(objError);
}

Local<Value> ODBC::CallbackSQLError (SQLSMALLINT handleType,
                                      SQLHANDLE handle,
                                      char* message,
                                      Nan::Callback* cb) {
  Nan::EscapableHandleScope scope;
  
  Local<Value> objError = ODBC::GetSQLError(
    handleType, 
    handle, 
    message
  );
  
  Local<Value> info[1];
  info[0] = objError;
  cb->Call(1, info);
  
  return scope.Escape(Nan::Undefined());
}

/*
 * GetSQLError
 */

Local<Value> ODBC::GetSQLError (SQLSMALLINT handleType, SQLHANDLE handle) {
  Nan::EscapableHandleScope scope;
  
  return scope.Escape(GetSQLError(
    handleType,
    handle,
    (char *) "[node-odbc] SQL_ERROR"));
}

Local<Value> ODBC::GetSQLError (SQLSMALLINT handleType, SQLHANDLE handle, char* message) {
  Nan::EscapableHandleScope scope;
  
  DEBUG_PRINTF("ODBC::GetSQLError : handleType=%i, handle=%p\n", handleType, handle);
  
  Local<Object> objError = Nan::New<Object>();

  SQLINTEGER i = 0;
  SQLINTEGER native;
  
  SQLSMALLINT len;
  SQLINTEGER numfields;
  SQLRETURN ret;
  char errorSQLState[14];
  char errorMessage[SQL_MAX_MESSAGE_LENGTH];

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
  Local<Array> errors = Nan::New<Array>();
  objError->Set(Nan::New("errors").ToLocalChecked(), errors);
  
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
      
      objError->Set(Nan::New("error").ToLocalChecked(), Nan::New(message).ToLocalChecked());
#ifdef UNICODE
      objError->SetPrototype(Exception::Error(Nan::New((uint16_t *) errorMessage).ToLocalChecked()));
      objError->Set(Nan::New("message").ToLocalChecked(), Nan::New((uint16_t *) errorMessage).ToLocalChecked());
      objError->Set(Nan::New("state").ToLocalChecked(), Nan::New((uint16_t *) errorSQLState).ToLocalChecked());
#else
      objError->SetPrototype(Exception::Error(Nan::New(errorMessage).ToLocalChecked()));
      objError->Set(Nan::New("message").ToLocalChecked(), Nan::New(errorMessage).ToLocalChecked());
      objError->Set(Nan::New("state").ToLocalChecked(), Nan::New(errorSQLState).ToLocalChecked());
#endif
    } else if (ret == SQL_NO_DATA) {
      break;
    }
  }
  
  return scope.Escape(objError);
}

/*
 * GetAllRecordsSync
 */

Local<Array> ODBC::GetAllRecordsSync (SQLHENV hENV, 
                                     SQLHDBC hDBC, 
                                     SQLHSTMT hSTMT,
                                     uint16_t* buffer,
                                     int bufferLength) {
  DEBUG_PRINTF("ODBC::GetAllRecordsSync\n");
  
  Nan::EscapableHandleScope scope;
  
  Local<Value> objError = Nan::New<Object>();
  
  int count = 0;
  int errorCount = 0;
  short colCount = 0;
  
  Column* columns = GetColumns(hSTMT, &colCount);
  
  Local<Array> rows = Nan::New<Array>();
  
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
      Nan::New(count), 
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
  return scope.Escape(rows);
}

#ifdef dynodbc
NAN_METHOD(ODBC::LoadODBCLibrary) {
  Nan::HandleScope scope;
  
  REQ_STR_ARG(0, js_library);
  
  bool result = DynLoadODBC(*js_library);
  
  info.GetReturnValue().Set((result) ? True() : False());
}
#endif

extern "C" void init(v8::Handle<Object> exports) {
#ifdef dynodbc
  exports->Set(Nan::New("loadODBCLibrary").ToLocalChecked(),
        FunctionTemplate::New(ODBC::LoadODBCLibrary)->GetFunction());
#endif
  
  ODBC::Init(exports);
  ODBCResult::Init(exports);
  ODBCConnection::Init(exports);
  ODBCStatement::Init(exports);
}

NODE_MODULE(odbc_bindings, init)

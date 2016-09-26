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

#define FILE_PARAM 3

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
  char colname[MAX_FIELD_SIZE];

  for (int i = 0; i < *colCount; i++) {
    //save the index number of this column
    columns[i].index = i + 1;
    colname[0] = '\0';
    buflen = 0;
    
    //get the column name
    ret = SQLColAttribute( hStmt,
                           columns[i].index,
#ifdef STRICT_COLUMN_NAMES
                           SQL_DESC_NAME,
#else
                           SQL_DESC_LABEL,
#endif
                           colname,
                           (SQLSMALLINT) MAX_FIELD_SIZE,
                           &buflen,
                           NULL);
    
    //store the len attribute
    columns[i].len = buflen;
    if(buflen> 0)
    {
        columns[i].name = new unsigned char[buflen+2];
        memcpy(columns[i].name, colname, buflen);
        columns[i].name[buflen] = '\0';
        columns[i].name[buflen+1] = '\0';
    }
    DEBUG_PRINTF("ODBC::GetColumns index = %i, buflen=%i\n", columns[i].index, buflen);
    
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
                           &typebuflen,
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
                                    uint16_t* buffer, int bufferLength) 
{
  Nan::EscapableHandleScope scope;
  SQLLEN len = 0;
  int ret; 
  Local<String> str;
  SQLSMALLINT ctype = SQL_C_TCHAR;
  char * errmsg = (char *) "[node-odbc] Error in ODBC::GetColumnValue";
#ifdef UNICODE
  int terCharLen = 2;
#else
  int terCharLen = 1;
#endif

  DEBUG_PRINTF("Column Type : %i\t%i\t%i\t%i\n",column.type, SQL_DATETIME, 
                SQL_TIMESTAMP, SQL_TYPE_TIME);
  //reset the buffer
  buffer[0] = '\0';

  switch ((int) column.type) 
  {
    case SQL_INTEGER : 
    case SQL_SMALLINT :
    case SQL_TINYINT : 
      {
        SQLINTEGER value;
        ret = SQLGetData( hStmt, 
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

    case SQL_FLOAT :
    case SQL_REAL :
    case SQL_DOUBLE : 
      {
        double value;
        ret = SQLGetData( hStmt, 
                          column.index, 
                          SQL_C_DOUBLE,
                          &value, 
                          sizeof(value), 
                          &len);
        
        DEBUG_PRINTF("ODBC::GetColumnValue - Number: index=%i name=%s type=%i len=%i ret=%i val=%f\n", 
                     column.index, column.name, column.type, len, ret, value);
        
        if((int)len == SQL_NULL_DATA) {
          return scope.Escape(Nan::Null());
        }
        else {
          return scope.Escape(Nan::New<Number>(value));
        }
      }
      break;
    case SQL_DATETIME :
    case SQL_TIMESTAMP : 
      {
        SQL_TIMESTAMP_STRUCT odbcTime;

        #ifdef _WIN32
        struct tm timeInfo = {};
        ret = SQLGetData(hStmt, column.index, SQL_C_CHAR, 
                         &odbcTime, sizeof(odbcTime), &len);
        #else
          #ifdef _AIX
          struct tm timeInfo = {0,0,0,0,0,0,0,0,0};
          #else
          struct tm timeInfo = {0,0,0,0,0,0,0,0,0,0,0};
          #endif
          ret = SQLGetData(hStmt, column.index, SQL_C_TYPE_TIMESTAMP, 
                           &odbcTime, sizeof(odbcTime), &len);
        #endif

        DEBUG_PRINTF("ODBC::GetColumnValue - Unix Timestamp: index=%i name=%s "
                     "type=%i len=%i\n", column.index, column.name, column.type, len);

        if((int)len == SQL_NULL_DATA) {
          return scope.Escape(Nan::Null());
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
          return scope.Escape(Nan::New<Date>((double(timegm(&timeInfo)) * 1000) + 
                      (odbcTime.fraction / 1000000)).ToLocalChecked());
          #else
          return scope.Escape(Nan::New<Date>((double(mktime(&timeInfo)) * 1000) + 
                      (odbcTime.fraction / 1000000)).ToLocalChecked());

          #endif
        }
      } 
      break;

    case SQL_BIT :
      {
        char bit[4] = {'\0'};
        ret = SQLGetData( hStmt,
                          column.index,
                          SQL_C_CHAR,
                          &bit,
                          4,
                          &len);

        DEBUG_PRINTF("ODBC::GetColumnValue - Bit: index=%i name=%s type=%i len=%i\n", 
                      column.index, column.name, column.type, len);

        if((int)len == SQL_NULL_DATA) {
            return scope.Escape(Nan::Null());
        }
        else {
            return scope.Escape(Nan::New((*bit == '0') ? false : true));
        }
      }
      break;
	/*
	case SQL_NUMERIC :
    case SQL_DECIMAL :
    case SQL_BIGINT :
    case SQL_BLOB :
    */
	case SQL_TYPE_TIME:
		DEBUG_PRINTF("SQL_TIME SELECTED\n");
    case SQL_NUMERIC :
        if((int) column.type == SQL_NUMERIC)
          DEBUG_PRINTF("NUMERIC DATA SELECTED\n");
    case SQL_DECIMAL :
        if((int) column.type == SQL_DECIMAL)
          DEBUG_PRINTF("DECIMAL DATA SELECTED\n");
    case SQL_BIGINT :
        if((int) column.type == SQL_BIGINT)
          DEBUG_PRINTF("BIGINT DATA SELECTED\n");
    case SQL_DBCLOB:
        if((int) column.type == SQL_DBCLOB)
        {
            ctype = SQL_C_DBCHAR;
            terCharLen = 2;
            DEBUG_PRINTF("DBCLOB DATA SELECTED\n");
        }
    case SQL_BLOB :
        if((int) column.type == SQL_BLOB)
        {
            terCharLen = 0;
            ctype = SQL_C_BINARY;
            DEBUG_PRINTF("BLOB DATA SELECTED\n");
        }
    default :
      uint16_t * tmp_out_ptr = NULL;
      int newbufflen = 0;
      int secondGetData = 0;
      len = 0;
      ret = SQLGetData( hStmt,
                        column.index,
                        ctype,
                        (char *) buffer,
                        bufferLength + terCharLen,
                        &len);

      DEBUG_PRINTF("ODBC::GetColumnValue - String: index=%i name=%s type=%i len=%i "
                   "ret=%i bufferLength=%i\n", column.index, column.name, 
                   column.type, len, ret, bufferLength);
      newbufflen = len;

      if( ret == SQL_SUCCESS_WITH_INFO )
      {
          newbufflen = len + bufferLength;
          if(newbufflen + terCharLen > 0x3fffffff)
              terCharLen = 0x3fffffff - terCharLen;
          tmp_out_ptr = (uint16_t *)malloc( newbufflen + terCharLen);
          if(tmp_out_ptr == NULL)
          {
            ret = -3;
            errmsg = (char*)"Failed to allocate memory buffer for column data.";
            DEBUG_PRINTF("Failed to allocate memory buffer of size %d\n", newbufflen + terCharLen);
          }
          else
          {
            memcpy(tmp_out_ptr, (char *) buffer, bufferLength);
            //free((uint8_t *)buffer);
            buffer = tmp_out_ptr;
            len = 0;
            ret = SQLGetData( hStmt,
                              column.index,
                              ctype,
                              (char *) buffer + bufferLength,
                              newbufflen + terCharLen,
                              &len);
            DEBUG_PRINTF("ODBC::GetColumnValue - String: index=%i name=%s type=%i len=%i "
                         "ret=%i bufferLength=%i\n", column.index, column.name, 
                         column.type, len, ret, newbufflen);
            newbufflen = len + bufferLength;
            secondGetData = 1;
          }
      }

      if((int)len == SQL_NULL_DATA) {
          return scope.Escape(Nan::Null());
      }
      // In case of secondGetData, we already have result from first getdata
      // so return the result irrespective of ret as we already have some data.
      else if (SQL_SUCCEEDED(ret) || secondGetData) 
      {
          if(ctype == SQL_C_BINARY)
              str = Nan::NewOneByteString((uint8_t *) buffer, newbufflen).ToLocalChecked();
          else {
            #ifdef UNICODE
            str = Nan::New((uint16_t *) buffer).ToLocalChecked();
            #else
            str = Nan::New((char *) buffer).ToLocalChecked();
            #endif
          }
          if(tmp_out_ptr) free(tmp_out_ptr);
          //return scope.Escape(Nan::CopyBuffer((char*)buffer, 39767).ToLocalChecked());
      }
      else 
      {
        DEBUG_PRINTF("ODBC::GetColumnValue - An error has occurred, ret = %i\n", ret);
        //an error has occured
        //possible values for ret are SQL_ERROR (-1) and SQL_INVALID_HANDLE (-2)
        //If we have an invalid handle, then stuff is way bad and we should abort
        //immediately. Memory errors are bound to follow as we must be in an
        //inconsisant state.
        if(ret == SQL_INVALID_HANDLE)
        {
          fprintf(stdout, "Invalid Handle: SQLGetData retrun code = %i, stmt handle = %i:%i"
                  ", columnType = %i, index = %i\n", ret, hStmt >> 16 & 0x0000ffff, 
                  hStmt & 0x0000ffff, (int) column.type, column.index);
          assert(ret != SQL_INVALID_HANDLE);
        }
        Nan::ThrowError(ODBC::GetSQLError( SQL_HANDLE_STMT, hStmt, errmsg));
        return scope.Escape(Nan::Undefined());
        break;
      }
      return scope.Escape(str);
  }
}

/*
 * GetOutputParameter
 */

Handle<Value> ODBC::GetOutputParameter( Parameter prm ) 
{
  Nan::EscapableHandleScope scope;
  Local<String> str;

  DEBUG_PRINTF("SQL Type of parameter: %i\n",prm.type);
  switch ((int) prm.type) 
  {
    case SQL_INTEGER : 
    case SQL_SMALLINT :
    case SQL_TINYINT : 
    case SQL_NUMERIC :
        if((int) prm.type == SQL_NUMERIC)
          DEBUG_PRINTF("NUMERIC DATA SELECTED\n");
    case SQL_BIGINT :
        if((int) prm.type == SQL_BIGINT)
          DEBUG_PRINTF("BIGINT DATA SELECTED\n");
      {
        if((int)prm.length == SQL_NULL_DATA)
        {
          DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                       "c_type=%i type=%i buf_len=%i len=%i val=%i\n", 
                       prm.paramtype, prm.c_type, prm.type, prm.buffer_length,
                       prm.length, prm.buffer);
          return scope.Escape(Nan::Null());
        }
        else if((int)prm.length == sizeof(int)){
          DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                       "c_type=%i type=%i buf_len=%i len=%i intval=%i\n", 
                       prm.paramtype, prm.c_type, prm.type, prm.buffer_length,
                       prm.length, *(int*)prm.buffer);
          return scope.Escape(Nan::New<Number>(*(int*)prm.buffer));
        }
        else if((int)prm.length == sizeof(short)){
          DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                       "c_type=%i type=%i buf_len=%i len=%i shortval=%i\n", 
                       prm.paramtype, prm.c_type, prm.type, prm.buffer_length,
                       prm.length, *(short*)prm.buffer);
          return scope.Escape(Nan::New<Number>(*(short*)prm.buffer));
        }
        else if((int)prm.length == sizeof(long)){
          DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                       "c_type=%i type=%i buf_len=%i len=%i longval=%i\n", 
                       prm.paramtype, prm.c_type, prm.type, prm.buffer_length,
                       prm.length, *(long*)prm.buffer);
          return scope.Escape(Nan::New<Number>(*(long*)prm.buffer));
        }
        else {
          DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                       "c_type=%i type=%i buf_len=%i len=%i charval=%s\n", 
                       prm.paramtype, prm.c_type, prm.type, prm.buffer_length,
                       prm.length, *(char*)prm.buffer);
          return scope.Escape(Nan::New<Number>(*(char *)prm.buffer));
        }
      }
      break;

    case SQL_DECIMAL :
        if((int) prm.type == SQL_DECIMAL)
          DEBUG_PRINTF("DECIMAL DATA SELECTED\n");
    case SQL_FLOAT :
    case SQL_REAL :
    case SQL_DOUBLE : 
      {
        if((int)prm.length == SQL_NULL_DATA)
        {
          DEBUG_PRINTF("ODBC::GetOutputParameter - Number: paramtype=%i c_type=%i "
                       "type=%i buf_len=%i len=%i nullval=%f\n", 
                       prm.paramtype, prm.c_type, prm.type, prm.buffer_length,
                       prm.length, prm.buffer);
          return scope.Escape(Nan::Null());
        }
        else {
          DEBUG_PRINTF("ODBC::GetOutputParameter - Number: paramtype=%i c_type=%i "
                       "type=%i buf_len=%i len=%i floatval=%f\n", 
                       prm.paramtype, prm.c_type, prm.type, prm.buffer_length,
                       prm.length, *(double*)prm.buffer);
          return scope.Escape(Nan::New<Number>(*(double*)prm.buffer));
        }
      }
      break;

    case SQL_BIT :
      {
        DEBUG_PRINTF("ODBC::GetOutputParameter - Bit: paramtype=%i c_type=%i type=%i buf_len=%i len=%i val=%f\n", 
                     prm.paramtype, prm.c_type, prm.type, prm.buffer_length,
                     prm.length, prm.buffer);
        if((int)prm.length == SQL_NULL_DATA) {
            return scope.Escape(Nan::Null());
        }
        else {
            return scope.Escape(Nan::New((*((char*)prm.buffer) == '0') ? false : true));
        }
      }
      break;
	/*
    case SQL_BLOB :
    case SQL_DATETIME :
    case SQL_TIMESTAMP : 
    */
    case SQL_TYPE_TIME:
		DEBUG_PRINTF("SQL_TIME SELECTED\n");
    case SQL_DBCLOB:
        if((int) prm.type == SQL_DBCLOB)
        {
            DEBUG_PRINTF("DBCLOB DATA SELECTED\n");
        }
    case SQL_BLOB :
        if((int) prm.type == SQL_BLOB)
        {
            DEBUG_PRINTF("BLOB DATA SELECTED\n");
        }
    default :
      DEBUG_PRINTF("ODBC::GetOutputParameter - String: paramtype=%i c_type=%i type=%i buf_len=%i len=%i val=%f\n", 
                   prm.paramtype, prm.c_type, prm.type, prm.buffer_length,
                   prm.length, prm.buffer);
      if((int)prm.length == SQL_NULL_DATA) {
          return scope.Escape(Nan::Null());
      }
      if(prm.c_type == SQL_C_BINARY)
          str = Nan::NewOneByteString((uint8_t *) prm.buffer, prm.length).ToLocalChecked();
      else {
        #ifdef UNICODE
        str = Nan::New((uint16_t *) prm.buffer).ToLocalChecked();
        #else
        str = Nan::New((char *) prm.buffer).ToLocalChecked();
        #endif
      }
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
        
  uv_mutex_lock(&ODBC::g_odbcMutex);
  for(int i = 0; i < *colCount; i++) {
#ifdef UNICODE
    tuple->Set( Nan::New((uint16_t *) columns[i].name).ToLocalChecked(),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
#else
    tuple->Set( Nan::New((const char *) columns[i].name).ToLocalChecked(),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
#endif
  }
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
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
        
  uv_mutex_lock(&ODBC::g_odbcMutex);
  for(int i = 0; i < *colCount; i++) {
    array->Set( Nan::New(i),
                GetColumnValue( hStmt, columns[i], buffer, bufferLength));
  }
  uv_mutex_unlock(&ODBC::g_odbcMutex);
  
  return scope.Escape(array);
}

/*
 * GetParametersFromArray
 */

Parameter* ODBC::GetParametersFromArray (Local<Array> values, int *paramCount) {
  DEBUG_PRINTF("ODBC::GetParametersFromArray\n");
  *paramCount = values->Length();
  
  Parameter* params = (Parameter *) malloc(*paramCount * sizeof(Parameter));
  memset(params, '\0', *paramCount * sizeof(Parameter));

  for (int i = 0; i < *paramCount; i++) {
    Local<Value> value = values->Get(i);
    
    params[i].paramtype     = SQL_PARAM_INPUT;
    params[i].size          = 0;
    params[i].length        = SQL_NULL_DATA;
    params[i].buffer_length = 0;
    params[i].decimals      = 0;

    DEBUG_PRINTF("ODBC::GetParametersFromArray - &param[%i].length = %p\n",
                 i, &params[i].length);

    if (value->IsArray()) 
    {
      Local<Value> val;
      Local<Array> paramArray = Local<Array>::Cast(value);
      int arrlen = paramArray->Length();
      if(arrlen < 4)
      {
          DEBUG_PRINTF("ODBC::GetParametersFromArray - arrlen = %i\n", arrlen);
          Nan::ThrowError("Wrong param format!");
          return params;
      }

      val =  paramArray->Get(0); 
      if(val->IsInt32())
          params[i].paramtype = val->IntegerValue();

      val =  paramArray->Get(1); 
      if(val->IsInt32())
          params[i].c_type = val->IntegerValue();
      else
          params[i].c_type = SQL_C_CHAR;

      val =  paramArray->Get(2); 
      if(val->IsInt32())
          params[i].type = val->IntegerValue();
      else
          params[i].type = SQL_CHAR;

      if(arrlen == 5)
      {
          val =  paramArray->Get(4); 
          if(val->IsInt32())
              params[i].buffer_length = val->IntegerValue();
      }

      val =  paramArray->Get(3); 
      if (val->IsNull()) {
          GetNullParam(&params[i], i+1);
      }
      else if (val->IsInt32()) {
          GetInt32Param(val, &params[i], i+1);
      }
      else if (val->IsNumber()) {
          GetNumberParam(val, &params[i], i+1);
      }
      else if (val->IsBoolean()) {
          GetBoolParam(val, &params[i], i+1);
      }
      else
      {
          GetStringParam(val, &params[i], i+1);
      }
    }
    else if (value->IsString()) {
        GetStringParam(value, &params[i], i+1);
    }
    else if (value->IsNull()) {
        GetNullParam(&params[i], i+1);
    }
    else if (value->IsInt32()) {
        GetInt32Param(value, &params[i], i+1);
    }
    else if (value->IsNumber()) {
        GetNumberParam(value, &params[i], i+1);
    }
    else if (value->IsBoolean()) {
        GetBoolParam(value, &params[i], i+1);
    }
  } 
  return params;
}

void ODBC::GetStringParam(Local<Value> value, Parameter * param, int num)
{
    Local<String> string = value->ToString();
    int length = string->Length();
    int bufflen = 0;
      
    param->length        = SQL_NTS;
    if(!param->c_type || (param->c_type == SQL_CHAR))
        param->c_type = SQL_C_TCHAR;
    #ifdef UNICODE
    if(!param->type || (param->type == SQL_CHAR))
        param->type = (length >= 8000) ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
    if(param->c_type != SQL_C_BINARY)
        bufflen = (length * sizeof(uint16_t)) + sizeof(uint16_t);
    #else
    if(!param->type || (param->type == SQL_CHAR))
        param->type = (length >= 8000) ? SQL_LONGVARCHAR : SQL_VARCHAR;
    if(param->c_type != SQL_C_BINARY)
        bufflen = string->Utf8Length() + 1;
    #endif
    if(bufflen < param->buffer_length && (param->paramtype % 2 == 0))
        bufflen = param->buffer_length;
    param->buffer_length = bufflen;

    if(param->c_type == SQL_C_BINARY || param->paramtype == FILE_PARAM)
    {
        param->buffer_length = length;
        param->length        = length; 
    }
    param->size          = param->buffer_length;
    param->buffer        = malloc(param->buffer_length);

    if(param->paramtype == FILE_PARAM)
        string->WriteUtf8((char *) param->buffer);
    else if(param->c_type == SQL_C_BINARY)
    {
#if (NODE_MODULE_VERSION < NODE_0_12_MODULE_VERSION)
        memcpy(param->buffer, &string, param->buffer_length);
        //param->buffer = &string;
#else
        string->WriteOneByte((uint8_t *)param->buffer);
#endif
    }
    else
    {
        #ifdef UNICODE
        string->Write((uint16_t *) param->buffer);
        #else
        string->WriteUtf8((char *) param->buffer);
        #endif
    }

    if(param->paramtype == FILE_PARAM)  // For SQLBindFileToParam()
    {
    /*
       SQLRETURN SQLBindFileToParam (
                 SQLHSTMT          StatementHandle,   // hstmt 
                 SQLUSMALLINT      TargetType,        // i 
                 SQLSMALLINT       DataType,          // type 
                 SQLCHAR           *FileName,         // buffer
                 SQLSMALLINT       *FileNameLength,   // NULL
                 SQLUINTEGER       *FileOptions,      // SQL_FILE_READ = 2 -> fileOption
                 SQLSMALLINT       MaxFileNameLength, // buffer_length -> decimals
                 SQLINTEGER        *IndicatorValue);  // 0 -> fileIndicator
    */
        param->decimals = param->buffer_length;
        param->fileOption = SQL_FILE_READ;
        param->fileIndicator = 0;
    }

    DEBUG_PRINTF("ODBC::GetStringParam: param%u : paramtype=%u, c_type=%i, "
                 "type=%i, size=%i, decimals=%i, buffer=%s, buffer_length=%i, "
                 "length=%i\n", num, param->paramtype, param->c_type, 
                 param->type, param->size, param->decimals, 
                 (char *)param->buffer, param->buffer_length, param->length);
}

void ODBC::GetNullParam(Parameter * param, int num)
{
    param->c_type = SQL_C_DEFAULT;
    param->type   = SQL_VARCHAR;
    param->length = SQL_NULL_DATA;

    DEBUG_PRINTF("ODBC::GetNullParam: param%u : paramtype=%u, c_type=%i, "
                 "type=%i, size=%i, decimals=%i, buffer_length=%i, length=%i\n",
                 num, param->paramtype, param->c_type, param->type, param->size,
                 param->decimals, param->buffer_length, param->length);
}

void ODBC::GetInt32Param(Local<Value> value, Parameter * param, int num)
{
    int64_t  *number = new int64_t(value->IntegerValue());
    param->c_type = SQL_C_SBIGINT;
    if(!param->type || (param->type == 1)) 
        param->type = SQL_BIGINT;
    param->buffer = number;
    param->length = sizeof(number);
      
    DEBUG_PRINTF("ODBC::GetInt32Param: param%u : paramtype=%u, c_type=%i, "
                 "type=%i, size=%i, decimals=%i, buffer=%lld, buffer_length=%i, "
                 "length=%i\n",
                 num, param->paramtype, param->c_type, param->type, param->size,
                 param->decimals, *number, param->buffer_length, param->length);
}

void ODBC::GetNumberParam(Local<Value> value, Parameter * param, int num)
{
    double *number   = new double(value->NumberValue());
      
    if(!param->c_type || (param->c_type == SQL_C_CHAR)) 
        param->c_type    = SQL_C_DOUBLE;
    if(!param->type || (param->type == SQL_CHAR)) 
        param->type      = SQL_DECIMAL;
    param->buffer        = number;
    param->buffer_length = sizeof(double);
    param->length        = param->buffer_length;
    param->decimals      = 7;
    param->size          = sizeof(double);

    DEBUG_PRINTF("ODBC::GetNumberParam: param%u : paramtype=%u, c_type=%i, "
                 "type=%i, size=%i, decimals=%i, buffer=%f, buffer_length=%i, "
                 "length=%i\n",
                 num, param->paramtype, param->c_type, param->type, param->size,
                 param->decimals, *number, param->buffer_length, param->length);
}

void ODBC::GetBoolParam(Local<Value> value, Parameter * param, int num)
{
    bool *boolean    = new bool(value->BooleanValue());
    param->c_type = SQL_C_BIT;
    if(!param->type || (param->type == SQL_CHAR)) 
        param->type   = SQL_BIT;
    param->buffer = boolean;
    param->length = 0;
      
    DEBUG_PRINTF("ODBC::GetBoolParam: param%u : paramtype=%u, c_type=%i, "
                 "type=%i, size=%i, decimals=%i, buffer_length=%i, length=%i\n",
                 num, param->paramtype, param->c_type, param->type, param->size,
                 param->decimals, param->buffer_length, param->length);
}

SQLRETURN ODBC::BindParameters(SQLHSTMT hSTMT, Parameter params[], int count)
{
    SQLRETURN ret = SQL_SUCCESS;
    Parameter prm;

    for (int i = 0; i < count; i++) 
    {
        prm = params[i];

        DEBUG_PRINTF(
          "ODBCConnection::UV_Query - param[%i]: c_type=%i type=%i "
          "buffer_length=%i size=%i length=%i &length=%p\n", i, prm.c_type, prm.type,
          prm.buffer_length, prm.size, prm.length, &params[i].length);

        if(prm.paramtype == FILE_PARAM)  // FILE
        {
            ret = SQLBindFileToParam (
                      hSTMT,               // StatementHandle, 
                      i + 1,               // TargetType, 
                      prm.type,            // DataType, 
                      (SQLCHAR *)prm.buffer,          // *FileName, 
                      NULL,                // *FileNameLength, // NULL or SQL_NTS
                      &params[i].fileOption,     // *FileOptions,  // SQL_FILE_READ = 2
                      prm.decimals,        // MaxFileNameLength, 
                      &params[i].fileIndicator); // *IndicatorValue); // 0 
        }
        else
            ret = SQLBindParameter(
                      hSTMT,                    //StatementHandle
                      i + 1,                    //ParameterNumber
                      prm.paramtype,            //InputOutputType
                      prm.c_type,               //ValueType
                      prm.type,                 //ParameterType
                      prm.size,                 //ColumnSize
                      prm.decimals,             //DecimalDigits
                      prm.buffer,               //ParameterValuePtr
                      prm.buffer_length,        //BufferLength
                      //using &prm.length did not work here...
                      &params[i].length);       //StrLen_or_IndPtr

        if (ret == SQL_ERROR) {break;}
    }
    return ret;
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
  
  DEBUG_PRINTF("ODBC::GetSQLError : handleType=%i, handle=%i\n", handleType, handle);
  
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
    } else {
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

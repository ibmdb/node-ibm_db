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
#ifdef __MVS__
#define _AE_BIMODAL
#include <_Nascii.h>
#endif

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

#define FILE_PARAM 3

using namespace v8;
using namespace node;

uv_mutex_t ODBC::g_odbcMutex;
uv_async_t ODBC::g_async;

Nan::Persistent<Function> ODBC::constructor;

NAN_MODULE_INIT(ODBC::Init)
{
  DEBUG_PRINTF("ODBC::Init\n");
  Nan::HandleScope scope;

  Local<FunctionTemplate> constructor_template = Nan::New<FunctionTemplate>(New);

  // Constructor Template
  constructor_template->SetClassName(Nan::New("ODBC").ToLocalChecked());

  // Reserve space for one Handle<Value>
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

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
  Local<Function> function = Nan::GetFunction(constructor_template).ToLocalChecked();
  constructor.Reset(function);
  Nan::Set(target, Nan::New<String>("ODBC").ToLocalChecked(), function);

#if NODE_VERSION_AT_LEAST(0, 7, 9)
  // Initialize uv_async so that we can prevent node from exiting
  // uv_async_init( uv_default_loop(),
  //               &ODBC::g_async,
  //               ODBC::WatcherCallback);

  // Not sure if the init automatically calls uv_ref() because there is weird
  // behavior going on. When ODBC::Init is called which initializes the
  // uv_async_t g_async above, there seems to be a ref which will keep it alive
  // but we only want this available so that we can uv_ref() later on when
  // we have a connection.
  // so to work around this, I am possibly mistakenly calling uv_unref() once
  // so that there are no references on the loop.
  // uv_unref((uv_handle_t *)&ODBC::g_async);
#endif

  // Initialize the cross platform mutex provided by libuv
  uv_mutex_init(&ODBC::g_odbcMutex);
}

ODBC::~ODBC()
{
  DEBUG_PRINTF("ODBC::~ODBC\n");
  this->Free();
}

void ODBC::Free()
{
  DEBUG_PRINTF("ODBC::Free: m_hEnv = %X\n", m_hEnv);
  if (m_hEnv)
  {
    SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
    m_hEnv = (SQLHENV)NULL;
  }
}

NAN_METHOD(ODBC::New)
{
  DEBUG_PRINTF("ODBC::New - Entry\n");
  Nan::HandleScope scope;
  ODBC *dbo = new ODBC();

  dbo->Wrap(info.Holder());

  dbo->m_hEnv = (SQLHENV)NULL;

#ifdef __MVS__
  int ori_mode = __ae_thread_swapmode(__AE_EBCDIC_MODE);
#endif
  // Initialize the Environment handle
  int ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &dbo->m_hEnv);
#ifdef __MVS__
  __ae_thread_swapmode(ori_mode);
#endif
  if (!SQL_SUCCEEDED(ret))
  {
    DEBUG_PRINTF("ODBC::New - ERROR ALLOCATING ENV HANDLE!!\n");

    Local<Value> objError = ODBC::GetSQLError(SQL_HANDLE_ENV, dbo->m_hEnv);

    return Nan::ThrowError(objError);
  }

  // Use ODBC 3.x behavior
  SQLSetEnvAttr(dbo->m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_UINTEGER);

  info.GetReturnValue().Set(info.Holder());
  DEBUG_PRINTF("ODBC::New - Exit\n");
}

// void ODBC::WatcherCallback(uv_async_t *w, int revents) {
//   DEBUG_PRINTF("ODBC::WatcherCallback\n");
//   //i don't know if we need to do anything here
// }

/*
 * CreateConnection
 */

NAN_METHOD(ODBC::CreateConnection)
{
  DEBUG_PRINTF("ODBC::CreateConnection - Entry\n");
  Nan::HandleScope scope;

  Local<Function> cb = Nan::To<v8::Function>(info[0]).ToLocalChecked();

  ODBC *dbo = Nan::ObjectWrap::Unwrap<ODBC>(info.Holder());

  // initialize work request
  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  // initialize our data
  create_connection_work_data *data =
      (create_connection_work_data *)(calloc(1, sizeof(create_connection_work_data)));

  if (!data)
    free(work_req); // Memcheck macro will log error and return;
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->dbo = dbo;

  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_CreateConnection, (uv_after_work_cb)UV_AfterCreateConnection);

  dbo->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBC::CreateConnection - Exit\n");
}

void ODBC::UV_CreateConnection(uv_work_t *req)
{
  DEBUG_PRINTF("ODBC::UV_CreateConnection - Entry\n");

  // get our work data
  create_connection_work_data *data = (create_connection_work_data *)(req->data);

  // allocate a new connection handle
  data->result = SQLAllocHandle(SQL_HANDLE_DBC, data->dbo->m_hEnv, &data->hDBC);
  DEBUG_PRINTF("ODBC::UV_CreateConnection - Exit: hDBC = %X\n", data->hDBC);
}

void ODBC::UV_AfterCreateConnection(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBC::UV_AfterCreateConnection - Entry\n");
  Nan::HandleScope scope;

  create_connection_work_data *data = (create_connection_work_data *)(req->data);

  Nan::TryCatch try_catch;

  if (!SQL_SUCCEEDED(data->result))
  {
    Local<Value> info[1];

    info[0] = ODBC::GetSQLError(SQL_HANDLE_ENV, data->dbo->m_hEnv);
    DEBUG_PRINTF("ODBC::UV_AfterCreateConnection - Got error in connection.\n");

    data->cb->Call(1, info);
  }
  else
  {
    Local<Value> info[2];
    info[0] = Nan::New<External>((void *)(intptr_t)data->dbo->m_hEnv);
    info[1] = Nan::New<External>((void *)(intptr_t)data->hDBC);

    Local<Object> js_result = Nan::NewInstance(Nan::New(ODBCConnection::constructor), 2, info).ToLocalChecked();

    info[0] = Nan::Null();
    info[1] = js_result;

    data->cb->Call(2, info);
  }

  if (try_catch.HasCaught())
  {
    DEBUG_PRINTF("ODBC::UV_AfterCreateConnection - Received FatalException.\n");
    Nan::FatalException(try_catch);
  }

  data->dbo->Unref();
  delete data->cb;

  free(data);
  free(req);
  DEBUG_PRINTF("ODBC::UV_AfterCreateConnection - Exit\n");
}

/*
 * CreateConnectionSync
 */

NAN_METHOD(ODBC::CreateConnectionSync)
{
  DEBUG_PRINTF("ODBC::CreateConnectionSync - Entry\n");
  Nan::HandleScope scope;

  ODBC *dbo = Nan::ObjectWrap::Unwrap<ODBC>(info.Holder());

  SQLHDBC hDBC;

  // allocate a new connection handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, dbo->m_hEnv, &hDBC);

  if (!SQL_SUCCEEDED(ret))
  {
    // TODO: do something!
  }

  Local<Value> params[2];
  params[0] = Nan::New<External>((void *)(intptr_t)dbo->m_hEnv);
  params[1] = Nan::New<External>((void *)(intptr_t)hDBC);

  Local<Object> js_result = Nan::NewInstance(Nan::New(ODBCConnection::constructor), 2, params).ToLocalChecked();

  DEBUG_PRINTF("ODBC::CreateConnectionSync - Exit: hDBC = %X\n", hDBC);
  info.GetReturnValue().Set(js_result);
}

/*
 * GetColumns
 */

Column *ODBC::GetColumns(SQLHSTMT hStmt, short *colCount)
{
  SQLRETURN ret;
  SQLSMALLINT buflen;
  SQLSMALLINT typebuflen;
  DEBUG_PRINTF("ODBC::GetColumns - Entry\n");

  // always reset colCount for the current result set to 0;
  *colCount = 0;

  // get the number of columns in the result set
  ret = SQLNumResultCols(hStmt, colCount);

  if (!SQL_SUCCEEDED(ret))
  {
    return new Column[0];
  }

  Column *columns = new Column[*colCount];
  char colname[MAX_FIELD_SIZE];

  for (int i = 0; i < *colCount; i++)
  {
    // save the index number of this column
    columns[i].index = i + 1;
    columns[i].getData = false;
    colname[0] = '\0';
    buflen = 0;

    // get the column name
    ret = SQLColAttribute(hStmt,
                          columns[i].index,
#ifdef STRICT_COLUMN_NAMES
                          SQL_DESC_NAME,
#else
                          SQL_DESC_LABEL,
#endif
                          colname,
                          (SQLSMALLINT)MAX_FIELD_SIZE,
                          &buflen,
                          NULL);

    // store the len attribute
    columns[i].name_len = buflen;
    if (buflen > 0)
    {
      columns[i].name = new unsigned char[buflen + 2];
      memcpy(columns[i].name, colname, buflen);
      columns[i].name[buflen] = '\0';
      columns[i].name[buflen + 1] = '\0';
    }
    DEBUG_PRINTF("ODBC::GetColumns index = %i, buflen=%i\n", columns[i].index, buflen);

    // get max column length
    ret = SQLColAttribute(hStmt,
                          columns[i].index,
                          SQL_DESC_DISPLAY_SIZE,
                          NULL,
                          0,
                          NULL,
                          &columns[i].max_display_len);

    // get column precision
    ret = SQLColAttribute(hStmt,
                          columns[i].index,
                          SQL_DESC_PRECISION,
                          NULL,
                          0,
                          NULL,
                          &columns[i].precision);

    // get column scale
    ret = SQLColAttribute(hStmt,
                          columns[i].index,
                          SQL_DESC_SCALE,
                          NULL,
                          0,
                          NULL,
                          &columns[i].scale);

    // get column length
    ret = SQLColAttribute(hStmt,
                          columns[i].index,
                          SQL_DESC_LENGTH,
                          NULL,
                          0,
                          NULL,
                          &columns[i].field_len);

    // get the column type and store it directly in column[i].type
    ret = SQLColAttribute(hStmt,
                          columns[i].index,
                          SQL_DESC_CONCISE_TYPE,
                          NULL,
                          0,
                          NULL,
                          &columns[i].type);

    columns[i].type_name = new unsigned char[(MAX_FIELD_SIZE)];

    // set the first byte of type_name to \0 instead of memsetting the entire buffer
    columns[i].type_name[0] = '\0';

    ret = SQLColAttribute(hStmt,
                          columns[i].index,
                          SQL_DESC_TYPE_NAME,
                          columns[i].type_name,
                          (SQLSMALLINT)(MAX_FIELD_SIZE),
                          &typebuflen,
                          NULL);
  }
  DEBUG_PRINTF("ODBC::GetColumns - Exit\n");
  return columns;
}

/*
 * FreeColumns
 */

void ODBC::FreeColumns(Column *&columns, short *colCount)
{
  DEBUG_PRINTF("ODBC::FreeColumns - Entry\n");
  for (int i = 0; i < *colCount; i++)
  {
    delete[] columns[i].name;
    delete[] columns[i].type_name;
  }

  if (columns != NULL)
  {
    delete[] columns;
    columns = NULL;
  }
  *colCount = 0;
  DEBUG_PRINTF("ODBC::FreeColumns - Exit\n");
}

/*
 * GetColumnValue
 * Function to read the value of Column field from the
 * result of a select or CALL statement.
 */

Local<Value> ODBC::GetColumnValue(SQLHSTMT hStmt, Column column,
                                  uint16_t *buffer, size_t bufferLength)
{
  Nan::EscapableHandleScope scope;
  SQLLEN len = 0; // To be used as last argument of SQLGetData. It can be -ve.
  SQLLEN maxBuffSize = 0;
  unsigned short terCharLen = 1;
  SQLRETURN ret = SQL_SUCCESS;
  Local<String> str;
  SQLSMALLINT ctype = SQL_C_TCHAR;
  char *errmsg = (char *)"[node-ibm_db] Error in ODBC::GetColumnValue";

#ifdef UNICODE
  terCharLen = 2;
#endif

  DEBUG_PRINTF("Column Type : %i\t%i\t%i\t%i\n", column.type, SQL_DATETIME,
               SQL_TIMESTAMP, SQL_TYPE_TIME);

  switch (column.type)
  {
  case SQL_BLOB:
    DEBUG_PRINTF("BLOB DATA SELECTED\n");
    terCharLen = 0;
    ctype = SQL_C_BINARY;
    break;
  case SQL_BINARY:
    DEBUG_PRINTF("BINARY DATA SELECTED\n");
    terCharLen = 0;
    ctype = SQL_C_BINARY;
    break;
  case SQL_VARBINARY:
    DEBUG_PRINTF("VARBINARY DATA SELECTED\n");
    terCharLen = 0;
    ctype = SQL_C_BINARY;
    break;
  case SQL_LONGVARBINARY:
    DEBUG_PRINTF("LONGVARBINARY DATA SELECTED\n");
    terCharLen = 0;
    ctype = SQL_C_BINARY;
    break;
  }

  switch (column.type)
  {
  case SQL_INTEGER:
  case SQL_SMALLINT:
  case SQL_TINYINT:
  {
    SQLINTEGER value;
    ret = SQLGetData(hStmt,
                     column.index,
                     SQL_C_SLONG,
                     &value,
                     sizeof(value),
                     &len);

    DEBUG_PRINTF("ODBC::GetColumnValue - Integer: index=%i name=%s type=%i len=%i ret=%i, val = %ld\n",
                 column.index, column.name, column.type, len, ret, value);

    if (len == SQL_NULL_DATA)
    {
      return scope.Escape(Nan::Null());
    }
    else if (len == sizeof(int))
    {
      return scope.Escape(Nan::New<Number>((int)value));
    }
    else if (len == sizeof(short))
    {
      return scope.Escape(Nan::New<Number>((short)value));
    }
    else if (len == sizeof(long))
    {
      return scope.Escape(Nan::New<Number>((long)value));
    }
    else
    {
      return scope.Escape(Nan::New<Number>(value));
    }
  }
  break;

  case SQL_NUMERIC:
    DEBUG_PRINTF("NUMERIC DATA SELECTED\n");
  case SQL_DECIMAL:
    DEBUG_PRINTF("DECIMAL DATA SELECTED\n");
  case SQL_DECFLOAT:
    DEBUG_PRINTF("DECFLOAT DATA SELECTED\n");
  case SQL_FLOAT:
  case SQL_REAL:
  case SQL_DOUBLE:
  {
    double value;
    ret = SQLGetData(hStmt,
                     column.index,
                     SQL_C_DOUBLE,
                     &value,
                     sizeof(value),
                     &len);

    DEBUG_PRINTF("ODBC::GetColumnValue - Number: index=%i name=%s type=%i len=%i ret=%i val=%f\n",
                 column.index, column.name, column.type, len, ret, value);

    if (len == SQL_NULL_DATA)
    {
      return scope.Escape(Nan::Null());
    }
    else
    {
      return scope.Escape(Nan::New<Number>(value));
    }
  }
  break;
  case SQL_DATETIME:
  case SQL_TIMESTAMP:
  {
    SQL_TIMESTAMP_STRUCT odbcTime;

#ifdef _WIN32
    struct tm timeInfo = {};
    ret = SQLGetData(hStmt, column.index, SQL_C_CHAR,
                     &odbcTime, sizeof(odbcTime), &len);
#else
#if defined(_AIX) || defined(__MVS__)
    struct tm timeInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
    struct tm timeInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
    ret = SQLGetData(hStmt, column.index, SQL_C_TYPE_TIMESTAMP,
                     &odbcTime, sizeof(odbcTime), &len);
#endif

    DEBUG_PRINTF("ODBC::GetColumnValue - Unix Timestamp: index=%i name=%s "
                 "type=%i len=%i\n",
                 column.index, column.name, column.type, len);

    if (len == SQL_NULL_DATA)
    { // SQL_NULL_DATA = -1
      return scope.Escape(Nan::Null());
    }
    else
    {
      timeInfo.tm_year = odbcTime.year - 1900;
      timeInfo.tm_mon = odbcTime.month - 1;
      timeInfo.tm_mday = odbcTime.day;
      timeInfo.tm_hour = odbcTime.hour;
      timeInfo.tm_min = odbcTime.minute;
      timeInfo.tm_sec = odbcTime.second;

      // a negative value means that mktime() should use timezone information
      // and system databases to attempt to determine whether DST is in effect
      // at the specified time.
      timeInfo.tm_isdst = -1;

#ifdef TIMEGM
      return scope.Escape(Nan::New<Date>((double(timegm(&timeInfo)) * 1000) +
                                         (odbcTime.fraction / 1000000))
                              .ToLocalChecked());
#else
      return scope.Escape(Nan::New<Date>((double(mktime(&timeInfo)) * 1000) +
                                         (odbcTime.fraction / 1000000))
                              .ToLocalChecked());

#endif
    }
  }
  break;

  case SQL_BIT:
  {
    char bit[4] = {'\0'};
    ret = SQLGetData(hStmt,
                     column.index,
                     SQL_C_CHAR,
                     &bit,
                     4,
                     &len);

    DEBUG_PRINTF("ODBC::GetColumnValue - Bit: index=%i name=%s type=%i len=%i\n",
                 column.index, column.name, column.type, len);

    if (len == SQL_NULL_DATA)
    {
      return scope.Escape(Nan::Null());
    }
    else
    {
      return scope.Escape(Nan::New((*bit == '0') ? false : true));
    }
  }
  break;
  case SQL_TYPE_TIME:
    DEBUG_PRINTF("SQL_TIME SELECTED\n");
  case SQL_BIGINT:
    if (column.type == SQL_BIGINT)
    {
      ctype = SQL_C_CHAR;
      DEBUG_PRINTF("BIGINT DATA SELECTED\n");
    }
  case SQL_DBCLOB:
    if (column.type == SQL_DBCLOB)
    {
      ctype = SQL_C_CHAR;
      DEBUG_PRINTF("DBCLOB DATA SELECTED\n");
    }
  default:
    uint16_t *tmp_out_ptr = NULL;
    uint32_t bufferSize = 0;
    uint32_t newbufflen = 0;
    unsigned short secondGetData = 0;
    len = 0;
    bufferSize = bufferLength + terCharLen;

    if (buffer == NULL)
    {
      buffer = (uint16_t *)malloc(bufferSize);
      if (buffer == NULL)
      {
        errmsg = (char *)"Failed to allocate memory buffer for column data.";
        DEBUG_PRINTF("Failed to allocate memory buffer of size %d\n", bufferSize);
        Nan::ThrowError(ODBC::GetSQLError(SQL_HANDLE_STMT, hStmt, errmsg));
        return scope.Escape(Nan::Undefined());
      }
    }
    maxBuffSize = bufferSize;
    memset(buffer, '\0', bufferSize);
    ret = SQLGetData(hStmt,
                     column.index,
                     ctype,
                     (char *)buffer,
                     maxBuffSize,
                     &len);

    DEBUG_PRINTF("ODBC::GetColumnValue - String: index=%i name=%s type=%i len=%i "
                 "ret=%i bufferLength=%i, maxBuffSize=%i\n",
                 column.index, column.name,
                 column.type, len, ret, bufferLength, maxBuffSize);
    // For ret = SQL_SUCCESS_WITH_INFO, len will have the total size of data to be retrieved.
    // So, allocate memory of size "len" and call SQLGetData again to retrieve the remaining data.

    if (column.getData)
    {
      secondGetData = 1; // Don't go for second SQLGetData call.
    }
    else if (ret == SQL_SUCCESS_WITH_INFO && len > 0) // There is pending data, need to call SQLGetData again.
    {
      // We have len byte of data for this column out of which, only bufferLength byte extracted.
      // So, need to allocate larger memory and call SQLGetData again to retrieve the remaining data.
      if (len + terCharLen > 0x3fffffff)
        newbufflen = 0x3fffffff; // 0x3fffffff = 1073741823 = 1GB - Can't allocate larger memory
      else
        newbufflen = len + terCharLen; // len has the total byte of data to be retrieved.

      tmp_out_ptr = (uint16_t *)malloc(newbufflen);
      if (tmp_out_ptr == NULL)
      {
        // Failed to allocate new memory so don't call SQLGetData and return the buffer data.
        ret = -3;
        errmsg = (char *)"Failed to allocate memory buffer for column data.";
        DEBUG_PRINTF("Failed to allocate memory buffer of size %d\n", newbufflen);
        newbufflen = bufferSize;
      }
      else
      {
        memcpy(tmp_out_ptr, (char *)buffer, bufferLength);
        free((uint16_t *)buffer);
        buffer = tmp_out_ptr;
        maxBuffSize = newbufflen - bufferLength;
        len = 0;
        ret = SQLGetData(hStmt,
                         column.index,
                         ctype,
                         (char *)buffer + bufferLength,
                         maxBuffSize,
                         &len);
        DEBUG_PRINTF("ODBC::GetColumnValue - String: index=%i name=%s type=%i len=%i "
                     "ret=%i newbufflen=%i, maxBuffSize=%i\n",
                     column.index, column.name,
                     column.type, len, ret, newbufflen, maxBuffSize);

        ret = SQL_SUCCESS; // Ignore any error and return the data from buffer.
        len = newbufflen;
        secondGetData = 1;
      }
    }
    if (len >= 0)
      newbufflen = len; // size of binary data returned in buffer.

    if (len == SQL_NULL_DATA)
    {
      FREE(buffer);
      return scope.Escape(Nan::Null());
    }
    // In case of secondGetData, we already have result from first getdata
    // so return the result irrespective of ret as we already have some data.
    else if (SQL_SUCCEEDED(ret) || secondGetData)
    {
      if (ctype == SQL_C_BINARY)
      { // Return binary data as buffer.
        // str = Nan::NewOneByteString((uint8_t *) buffer, newbufflen).ToLocalChecked();
        return scope.Escape(Nan::NewBuffer((char *)buffer, newbufflen).ToLocalChecked());
        // buffer will be freed by Garbage collector, no need to free here.
      }
      else
      {
        str = Nan::New((UNICHAR *)buffer).ToLocalChecked();
      }
      FREE(buffer);
    }
    else
    {
      DEBUG_PRINTF("ODBC::GetColumnValue - An error has occurred, ret = %i\n", ret);
      FREE(buffer);
      // an error has occured
      // possible values for ret are SQL_ERROR (-1) and SQL_INVALID_HANDLE (-2)
      // If we have an invalid handle, then stuff is way bad and we should abort
      // immediately. Memory errors are bound to follow as we must be in an
      // inconsisant state.
      if (ret == SQL_INVALID_HANDLE)
      {
#ifdef __MVS__
        // Workaround(zOS): The ascii conversion tool has issues with %i in the following
        // fprintf string.  Forcing it to __fprintf_a and setting _AE_BIMODAL
        __fprintf_a(stdout, "Invalid Handle: SQLGetData retrun code = %i, stmt handle = %ld:%ld"
                            ", columnType = %i, index = %i\n",
                    ret, (long)(hStmt) >> 16 & 0x0000ffff,
                    (long)(hStmt) & 0x0000ffff, (int)column.type, column.index);
#else
        fprintf(stdout, "Invalid Handle: SQLGetData retrun code = %i, stmt handle = %ld:%ld"
                        ", columnType = %i, index = %i\n",
                ret, (long)(hStmt) >> 16 & 0x0000ffff,
                (long)(hStmt) & 0x0000ffff, (int)column.type, column.index);
#endif
        assert(ret != SQL_INVALID_HANDLE);
      }
      Nan::ThrowError(ODBC::GetSQLError(SQL_HANDLE_STMT, hStmt, errmsg));
      return scope.Escape(Nan::Undefined());
      break;
    }
    return scope.Escape(str);
  }
}

/*
 * GetOutputParameter
 * Function to retrieve the INOUT and OUTPUT parameters of CALL statement
 * It is called by ODBCConnection::Query and ODBCStatement::Execute APIs
 */

Local<Value> ODBC::GetOutputParameter(Parameter *prm)
{
  Nan::EscapableHandleScope scope;
  bool sqlTypeBinary = false;

  DEBUG_PRINTF("SQL Type of parameter: %i\n", prm->type);
  switch (prm->type)
  {
  case SQL_BLOB:
    DEBUG_PRINTF("BLOB DATA in ResultSet\n");
    sqlTypeBinary = true;
    break;
  case SQL_BINARY:
    DEBUG_PRINTF("BINARY DATA in ResultSet\n");
    sqlTypeBinary = true;
    break;
  case SQL_VARBINARY:
    DEBUG_PRINTF("VARBINARY DATA in ResultSet\n");
    sqlTypeBinary = true;
    break;
  case SQL_LONGVARBINARY:
    DEBUG_PRINTF("LONGVARBINARY DATA in ResultSet\n");
    sqlTypeBinary = true;
    break;
    /*
      case SQL_DATETIME :
      case SQL_TIMESTAMP :
      */
  case SQL_TYPE_TIME:
    DEBUG_PRINTF("SQL_TIME in ResultSet\n");
    break;
  case SQL_DBCLOB:
    DEBUG_PRINTF("DBCLOB DATA in ResultSet\n");
    break;
  case SQL_DECIMAL:
    DEBUG_PRINTF("DECIMAL DATA in ResultSet\n");
    break;
  }

  // If length is SQL_NULL_DATA, return null
  // If buffer is NULL, can't return any data
  if (((int)prm->length == SQL_NULL_DATA) || !prm->buffer)
  {
    DEBUG_PRINTF("NULL DATA in ResultSet\n");
    DEBUG_PRINTF("ODBC::GetOutputParameter - NullData: paramtype=%i "
                 "c_type=%i type=%i buf_len=%i len=%i val=%i\n",
                 prm->paramtype, prm->c_type, prm->type, prm->buffer_length,
                 prm->length, prm->buffer);
    return scope.Escape(Nan::Null());
  }

  switch (prm->type)
  {
  case SQL_INTEGER:
  case SQL_SMALLINT:
  case SQL_TINYINT:
  case SQL_BOOLEAN:
  case SQL_NUMERIC:
  case SQL_BIGINT:
    if (prm->type == SQL_NUMERIC)
      DEBUG_PRINTF("NUMERIC DATA in ResultSet\n");
    if (prm->type == SQL_BIGINT)
      DEBUG_PRINTF("BIGINT DATA in ResultSet, length = %i\n", prm->length);
    if (prm->length == sizeof(int))
    {
      DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                   "c_type=%i type=%i buf_len=%i len=%i intval=%i\n",
                   prm->paramtype, prm->c_type, prm->type, prm->buffer_length,
                   prm->length, *(int *)prm->buffer);
      return scope.Escape(Nan::New<Number>(*(int *)prm->buffer));
    }
    else if (prm->length == sizeof(short))
    {
      DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                   "c_type=%i type=%i buf_len=%i len=%i shortval=%i\n",
                   prm->paramtype, prm->c_type, prm->type, prm->buffer_length,
                   prm->length, *(short *)prm->buffer);
      return scope.Escape(Nan::New<Number>(*(short *)prm->buffer));
    }
    else if (prm->length == sizeof(long))
    { // long is 4 byte on windows, 8 byte on linux
      DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                   "c_type=%i type=%i buf_len=%i len=%i longval=%i\n",
                   prm->paramtype, prm->c_type, prm->type, prm->buffer_length,
                   prm->length, *(long *)prm->buffer);
      return scope.Escape(Nan::New<Number>(*(long *)prm->buffer));
    }
    else if (prm->length == sizeof(long long))
    { // 8 byte on all x64
      // Fix for issue #816 on Windows
      DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                   "c_type=%i type=%i buf_len=%i len=%i longval=%i\n",
                   prm->paramtype, prm->c_type, prm->type, prm->buffer_length,
                   prm->length, *(long long *)prm->buffer);
      return scope.Escape(Nan::New<Number>(*(long long *)prm->buffer));
    }
    else
    {
      DEBUG_PRINTF("ODBC::GetOutputParameter - Integer: paramtype=%i "
                   "c_type=%i type=%i buf_len=%i len=%i charval=%s\n",
                   prm->paramtype, prm->c_type, prm->type, prm->buffer_length,
                   prm->length, *(char *)prm->buffer);
      return scope.Escape(Nan::New<Number>(*(char *)prm->buffer));
    }
    break;

  case SQL_DECIMAL:
  case SQL_FLOAT:
  case SQL_DECFLOAT:
  case SQL_REAL:
  case SQL_DOUBLE:
  {
    DEBUG_PRINTF("ODBC::GetOutputParameter - Number: paramtype=%i c_type=%i "
                 "type=%i buf_len=%i len=%i floatval=%f\n",
                 prm->paramtype, prm->c_type, prm->type, prm->buffer_length,
                 prm->length, *(double *)prm->buffer);
    return scope.Escape(Nan::New<Number>(*(double *)prm->buffer));
  }
  break;

  case SQL_BIT:
  {
    DEBUG_PRINTF("ODBC::GetOutputParameter - Bit: paramtype=%i c_type=%i type=%i buf_len=%i len=%i val=%f\n",
                 prm->paramtype, prm->c_type, prm->type, prm->buffer_length,
                 prm->length, prm->buffer);
    return scope.Escape(Nan::New((*((char *)prm->buffer) == '0') ? false : true));
  }
  break;

  default:
    DEBUG_PRINTF("ODBC::GetOutputParameter - String: paramtype=%i c_type=%i \
              type=%i buf_len=%i len=%i val=%p\n",
                 prm->paramtype, prm->c_type, prm->type, prm->buffer_length,
                 prm->length, prm->buffer);

    // If Buffer was passed by JS to C++, return Buffer only; for any datatype.
    // Do not free such char* as ownership is getting transferred to JS.
    if (prm->isBuffer)
    { // Buffer was passed by NodeJS to C++
      return scope.Escape(Nan::CopyBuffer((char *)prm->buffer, prm->length).ToLocalChecked());
    }
    else if (sqlTypeBinary || prm->c_type == SQL_C_BINARY)
    {
      // Return binary data as Buffer only for any datatype.
      prm->isBuffer = true; // Don't free it in FREE_PARAMS
      return scope.Escape(Nan::NewBuffer((char *)prm->buffer, prm->length).ToLocalChecked());
    }
    else
    {
      // If String was passed by JS to C++, return OUTPUT as char*
      return scope.Escape(Nan::New((UNICHAR *)prm->buffer).ToLocalChecked());
    }
  }
}

/*
 * GetRecordTuple
 * Function to return the row data as JSON object
 * It get called by ODBCResult::Fetch/FetchAll APIs
 */

Local<Object> ODBC::GetRecordTuple(SQLHSTMT hStmt, Column *columns,
                                   short *colCount, uint16_t *buffer,
                                   size_t bufferLength)
{
  Nan::EscapableHandleScope scope;

  Local<Object> tuple = Nan::New<Object>();

  for (int i = 0; i < *colCount; i++)
  {
#ifdef UNICODE
    Nan::Set(tuple, Nan::New((uint16_t *)columns[i].name).ToLocalChecked(),
             GetColumnValue(hStmt, columns[i], buffer, bufferLength));
#else
    Nan::Set(tuple, Nan::New((const char *)columns[i].name).ToLocalChecked(),
             GetColumnValue(hStmt, columns[i], buffer, bufferLength));
#endif
  }

  return scope.Escape(tuple);
}

/*
 * GetRecordArray
 * Function to return the row data as Array
 * It get called by ODBCResult::Fetch/FetchAll APIs
 */

Local<Value> ODBC::GetRecordArray(SQLHSTMT hStmt, Column *columns,
                                  short *colCount, uint16_t *buffer,
                                  size_t bufferLength)
{
  Nan::EscapableHandleScope scope;

  Local<Array> array = Nan::New<Array>();

  for (int i = 0; i < *colCount; i++)
  {
    Nan::TryCatch try_catch;
    Nan::Set(array, Nan::New(i),
             GetColumnValue(hStmt, columns[i], buffer, bufferLength));
    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
      break;
    }
  }

  return scope.Escape(array);
}

/*
 * GetParametersFromArray
 * Function to populate Parameter from bindParameters Array
 * It reads Bind Parameters passed to Query or Bind API.
 * It is called by ODBCConnection::Query & ODBCStatement::Bind
 */

Parameter *ODBC::GetParametersFromArray(Local<Array> values, int *paramCount)
{
  DEBUG_PRINTF("ODBC::GetParametersFromArray\n");
  *paramCount = values->Length();

  Parameter *params = (Parameter *)malloc(*paramCount * sizeof(Parameter));
  if (*paramCount != 0 && !params)
  {
    Nan::LowMemoryNotification();
    Nan::ThrowError("Could not allocate enough memory for params in ODBC::GetParametersFromArray.");
    return params;
  }
  memset(params, '\0', *paramCount * sizeof(Parameter));
  Local<Value> value;
  Local<Value> val;
  Local<Array> paramArray;
  int arrlen = 0;

  for (int i = 0; i < *paramCount; i++)
  {
    value = Nan::Get(values, i).ToLocalChecked();
    // Initialise the Parameters
    params[i].paramtype = SQL_PARAM_INPUT;
    params[i].size = 0;
    params[i].length = SQL_NULL_DATA;
    params[i].buffer_length = 0;
    params[i].decimals = 0;
    params[i].isBuffer = false;

    DEBUG_PRINTF("ODBC::GetParametersFromArray - param[%i].length = %d\n",
                 i, params[i].length);

    if (value->IsArray())
    {
      paramArray = Local<Array>::Cast(value);
      arrlen = paramArray->Length();
      if (arrlen < 4)
      {
        DEBUG_PRINTF("ODBC::GetParametersFromArray - arrlen = %i\n", arrlen);
        Nan::ThrowError("Wrong param format!");
        return params;
      }

      val = Nan::Get(paramArray, 0).ToLocalChecked();
      if (val->IsInt32())
        params[i].paramtype = Nan::To<int32_t>(val).FromJust();

      val = Nan::Get(paramArray, 1).ToLocalChecked();
      if (val->IsInt32())
        params[i].c_type = Nan::To<int32_t>(val).FromJust();
      else
        params[i].c_type = SQL_C_CHAR;

      val = Nan::Get(paramArray, 2).ToLocalChecked();
      if (val->IsInt32())
        params[i].type = Nan::To<int32_t>(val).FromJust();
      else
        params[i].type = SQL_CHAR;

      if (arrlen == 5)
      {
        val = Nan::Get(paramArray, 4).ToLocalChecked();
        if (val->IsInt32())
          params[i].buffer_length = Nan::To<int32_t>(val).FromJust();
      }

      val = Nan::Get(paramArray, 3).ToLocalChecked();
      if (val->IsNull())
      {
        GetNullParam(&params[i], i + 1);
      }
      else if (val->IsInt32() && params[i].type != 3)
      { // type=3 means DECIMAL
        GetInt32Param(val, &params[i], i + 1);
      }
      else if (val->IsNumber())
      {
        GetNumberParam(val, &params[i], i + 1);
      }
      else if (val->IsBoolean())
      {
        GetBoolParam(val, &params[i], i + 1);
      }
      else if (val->IsArray())
      {
        GetArrayParam(val, &params[i], i + 1);
      }
      else if (Buffer::HasInstance(val))
      {
        GetBufferParam(val, &params[i], i + 1);
      }
      else
      {
        GetStringParam(val, &params[i], i + 1);
      }
    }
    else if (Buffer::HasInstance(value))
    {
      GetBufferParam(value, &params[i], i + 1);
    }
    else if (value->IsString())
    {
      GetStringParam(value, &params[i], i + 1);
    }
    else if (value->IsNull())
    {
      GetNullParam(&params[i], i + 1);
    }
    else if (value->IsInt32())
    {
      GetInt32Param(value, &params[i], i + 1);
    }
    else if (value->IsNumber())
    {
      GetNumberParam(value, &params[i], i + 1);
    }
    else if (value->IsBoolean())
    {
      GetBoolParam(value, &params[i], i + 1);
    }
  }
  return params;
}

void ODBC::GetStringParam(Local<Value> value, Parameter *param, int num)
{
  // Nan::Utf8String string(value);
  // int length = string.length();
  Local<String> string = value->TOSTRING;
  int length = string->Length();
  int bufflen = length;
  int terCharLen = 1;

  param->length = length;
  if (!param->c_type || (param->c_type == SQL_CHAR))
    param->c_type = SQL_C_TCHAR;
#ifdef UNICODE
  param->length = SQL_NTS; // Required for unicode string.
  if (!param->type || (param->type == SQL_CHAR))
    param->type = (length >= 8000) ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
  if (param->c_type != SQL_C_BINARY)
    bufflen = (length + 1) * sizeof(uint16_t);
  terCharLen = 2;
#else
  if (!param->type || (param->type == SQL_CHAR))
    param->type = (length >= 8000) ? SQL_LONGVARCHAR : SQL_VARCHAR;
  if (param->c_type != SQL_C_BINARY)
  {
    bufflen = string->Utf8Length(ISOLATE) + 1;
    param->length = bufflen - 1;
  }
#endif
  if (param->c_type == SQL_C_BINARY)
  {
    bufflen = length + 1;
  }
  if (bufflen <= param->buffer_length &&
      ((param->paramtype % 2 == 0) || (param->arraySize > 0)))
  {
    bufflen = param->buffer_length + terCharLen;
  }
  param->buffer_length = bufflen;
  param->size = bufflen;

  param->buffer = malloc(param->buffer_length);
  MEMCHECK(param->buffer);

  if (param->paramtype == FILE_PARAM)
  {
    string->WriteUtf8(ISOLATECOMMA(char *) param->buffer);
  }
  else if (param->c_type == SQL_C_BINARY)
  {
    string->WriteOneByte(ISOLATECOMMA(uint8_t *) param->buffer);
  }
  else
  {
#ifdef UNICODE
    string->Write(ISOLATECOMMA(uint16_t *) param->buffer);
#else
    string->WriteUtf8(ISOLATECOMMA(char *) param->buffer);
#endif
  }
#ifdef UNICODE
  ((char *)param->buffer)[param->buffer_length - 2] = '\0';
#endif
  ((char *)param->buffer)[param->buffer_length - 1] = '\0';

  if (param->paramtype == FILE_PARAM) // For SQLBindFileToParam()
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
               "length=%i\n",
               num, param->paramtype, param->c_type,
               param->type, param->size, param->decimals,
               (char *)param->buffer, param->buffer_length, param->length);
}

void ODBC::GetBufferParam(Local<Value> value, Parameter *param, int num)
{
  DEBUG_PRINTF("ODBC::GetBufferParam - It's a buffer.\n");
  // Get pointer to the underlying memory buffer of Node.js Buffer object
  // and assign it to param->buffer. No need to allocate memory here for it.
  param->buffer = (char *)Buffer::Data(value);
  param->length = Buffer::Length(value);
  int bufflen = param->length;
  param->isBuffer = true; // Dont free it in FREE_PARAMS

  if (bufflen <= param->buffer_length &&
      ((param->paramtype % 2 == 0) || (param->arraySize > 0)))
  {
    bufflen = param->buffer_length;
  }
  param->buffer_length = bufflen;
  param->size = bufflen;

  if (!param->c_type || (param->c_type == SQL_CHAR))
    param->c_type = SQL_C_TCHAR;
#ifdef UNICODE
  param->length = SQL_NTS; // Required for unicode string.
  if (!param->type || (param->type == SQL_CHAR))
    param->type = (param->length >= 8000) ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
  if (param->c_type != SQL_C_BINARY)
    param->size /= 2;
#else
  if (!param->type || (param->type == SQL_CHAR))
    param->type = (param->length >= 8000) ? SQL_LONGVARCHAR : SQL_VARCHAR;
#endif

  if (param->paramtype == FILE_PARAM) // For SQLBindFileToParam()
  {
    param->decimals = param->buffer_length;
    param->fileOption = SQL_FILE_READ;
    param->fileIndicator = 0;
  }

  DEBUG_PRINTF("ODBC::GetBufferParam: param%u : paramtype=%u, c_type=%i, "
               "type=%i, size=%i, decimals=%i, buffer=%s, buffer_length=%i, "
               "length=%i\n",
               num, param->paramtype, param->c_type,
               param->type, param->size, param->decimals,
               (char *)param->buffer, param->buffer_length, param->length);
}

void ODBC::GetNullParam(Parameter *param, int num)
{
  param->c_type = param->c_type ? param->c_type : SQL_C_DEFAULT;
  param->type = param->type ? param->type : SQL_VARCHAR;
  param->length = SQL_NULL_DATA;

  // For INOUT or OUTPUT parameter of SP, allocate memory for output data
  if (param->paramtype % 2 == 0)
  {
    if (param->type == SQL_BIGINT || param->type == SQL_INTEGER ||
        param->type == SQL_BOOLEAN || param->type == SQL_SMALLINT ||
        param->type == SQL_TINYINT)
    {
      param->buffer = new int64_t(0);
    }
    else if (param->type == SQL_DECIMAL || param->type == SQL_DOUBLE ||
             param->type == SQL_REAL || param->type == SQL_FLOAT ||
             param->type == SQL_NUMERIC || param->type == SQL_DECFLOAT)
    {
      param->buffer = new double(0);
    }
    else if (param->buffer_length > 0)
    {
      param->buffer = malloc(param->buffer_length);
    }
    else
      param->buffer = malloc(8);
  }

  DEBUG_PRINTF("ODBC::GetNullParam: param%u : paramtype=%u, c_type=%i, "
               "type=%i, size=%i, decimals=%i, buffer_length=%i, length=%i\n",
               num, param->paramtype, param->c_type, param->type, param->size,
               param->decimals, param->buffer_length, param->length);
}

void ODBC::GetInt32Param(Local<Value> value, Parameter *param, int num)
{
  int64_t *number = new int64_t(Nan::To<int32_t>(value).FromJust());
  param->c_type = SQL_C_SBIGINT;
  if (!param->type || (param->type == 1))
    param->type = SQL_BIGINT;
  param->buffer = number;
  param->length = sizeof(number);

  DEBUG_PRINTF("ODBC::GetInt32Param: param%u : paramtype=%u, c_type=%i, "
               "type=%i, size=%i, decimals=%i, buffer=%lld, buffer_length=%i, "
               "length=%i\n",
               num, param->paramtype, param->c_type, param->type, param->size,
               param->decimals, *number, param->buffer_length, param->length);
}

void ODBC::GetNumberParam(Local<Value> value, Parameter *param, int num)
{
  double *number = new double(Nan::To<double>(value).FromJust());
  // Find the scale of decimal number
  Nan::Utf8String string(value);
  int length = string.length();
  char *ptr = strchr(*string, '.');

  if (!param->c_type || (param->c_type == SQL_C_CHAR))
    param->c_type = SQL_C_DOUBLE;
  if (!param->type || (param->type == SQL_CHAR))
    param->type = SQL_DECIMAL;
  param->buffer = number;
  param->buffer_length = sizeof(double);
  param->length = param->buffer_length;
  param->size = sizeof(double);
  if (ptr)
  {
    param->decimals = length - (ptr - *string) - 1;
    if (param->type == SQL_DECIMAL)
    {
      param->size = length - 1;
    }
  }
  else
  {
    param->decimals = 0;
    if (param->type == SQL_DECIMAL)
    {
      param->size = length;
    }
  }

  DEBUG_PRINTF("ODBC::GetNumberParam: param%u : paramtype=%u, c_type=%i, "
               "type=%i, size=%i, decimals=%i, buffer=%f, buffer_length=%i, "
               "length=%i\n",
               num, param->paramtype, param->c_type, param->type, param->size,
               param->decimals, *number, param->buffer_length, param->length);
}

void ODBC::GetBoolParam(Local<Value> value, Parameter *param, int num)
{
  bool boolean = Nan::To<bool>(value).FromJust();
  int64_t *number = new int64_t(0);
  if (boolean == true)
  {
    delete number;
    number = new int64_t(1);
  }
  // Db2 stores boolean data in the form of 0 or 1 only.
  // So overwrite the intered true or fase with 1 or 0 as bigint
  param->c_type = SQL_C_SBIGINT;
  if (!param->type || (param->type == 1))
    param->type = SQL_BIGINT;
  param->buffer = number;
  param->length = sizeof(number);

  DEBUG_PRINTF("ODBC::GetBoolParam: param%u : paramtype=%u, c_type=%i, "
               "type=%i, size=%i, decimals=%i, buffer=%lld, buffer_length=%i, length=%i\n",
               num, param->paramtype, param->c_type, param->type, param->size,
               param->decimals, *number, param->buffer_length, param->length);
}

void ODBC::GetArrayParam(Local<Value> value, Parameter *param, int num)
{
  Local<Array> valueArray = Local<Array>::Cast(value);
  Local<Value> val = Nan::Null();
  int arrlen = valueArray->Length();

  param->arraySize = arrlen;
  if (arrlen <= 0)
  {
    param->buffer = new int[0];
    return;
  }

  param->strLenArray = new SQLINTEGER[arrlen];

  for (int j = 0; j < arrlen; j++)
  {
    val = Nan::Get(valueArray, j).ToLocalChecked();
    if (val->IsNull())
    {
      param->strLenArray[j] = SQL_NULL_DATA;
      if (j == arrlen - 1)
      { // i.e. All data in array is null
        GetNullParam(param, num);
        param->buffer = new int[arrlen];
      }
      continue;
    }
    else if (val->IsInt32() && param->type != 3)
    { // type=3 means DECIMAL
      int64_t *number = new int64_t[arrlen];
      for (int i = j; i < arrlen; i++)
      {
        if (i != j)
        {
          val = Nan::Get(valueArray, i).ToLocalChecked();
          if (val->IsNull())
          {
            param->strLenArray[i] = SQL_NULL_DATA;
            continue;
          }
        }
        number[i] = Nan::To<int64_t>(val).FromJust();
        param->strLenArray[i] = sizeof(int64_t);
      }
      param->c_type = SQL_C_SBIGINT;
      if (!param->type || (param->type == 1))
        param->type = SQL_BIGINT;
      param->buffer = number;
    }
    else if (val->IsNumber())
    {
      double *number = new double[arrlen];
      int decimals = 0;

      for (int i = j; i < arrlen; i++)
      {
        if (i != j)
        {
          val = Nan::Get(valueArray, i).ToLocalChecked();
          if (val->IsNull())
          {
            param->strLenArray[i] = SQL_NULL_DATA;
            continue;
          }
        }
        GetNumberParam(val, param, num);
        number[i] = *(double *)param->buffer;
        param->strLenArray[i] = param->length;
        delete (double *)param->buffer;
        param->buffer = NULL;
        decimals = (decimals < param->decimals) ? param->decimals : decimals;
      }

      param->decimals = decimals;
      param->buffer = number;
    }
    else if (val->IsBoolean())
    {
      int64_t *boolean = new int64_t[arrlen];
      for (int i = j; i < arrlen; i++)
      {
        if (i != j)
        {
          val = Nan::Get(valueArray, i).ToLocalChecked();
          if (val->IsNull())
          {
            param->strLenArray[i] = SQL_NULL_DATA;
            continue;
          }
        }
        GetBoolParam(val, param, num);
        boolean[i] = *(int64_t *)(param->buffer);
        param->strLenArray[i] = (SQLINTEGER)param->length;
        delete (int64_t *)param->buffer;
        param->buffer = NULL;
      }
      param->buffer = boolean;
    }
    else
    {
      SQLPOINTER *buff = (SQLPOINTER *)NULL;
      SQLULEN bufflen = 0;
      SQLULEN cbValueMax = 0;
      // char **buff = (char **) new int64_t[arrlen];
      for (int i = j; i < arrlen; i++)
      {
        if (i != j)
        {
          val = Nan::Get(valueArray, i).ToLocalChecked();
          if (val->IsNull())
          {
            param->strLenArray[i] = SQL_NULL_DATA;
            continue;
          }
        }
        if (Buffer::HasInstance(val))
        {
          GetBufferParam(val, param, num);
        }
        else
        {
          GetStringParam(val, param, num);
          // Reset buffer_length which was incremented by GetStringParam
#ifdef UNICODE
          param->buffer_length -= 2;
#else
          param->buffer_length -= 1;
#endif
        }
        if (i == j)
        {
          cbValueMax = param->buffer_length; // Length of each element of array
          bufflen = arrlen * cbValueMax;
          buff = (SQLPOINTER *)realloc(buff, bufflen);
          DEBUG_PRINTF("ODBC::GetArrayParam Function: param%u : bufflen=%i, "
                       "cbValueMax=%i\n",
                       num, bufflen, cbValueMax);
        }
        bufflen = cbValueMax; // Length of max data to be copied.
        if (bufflen > (SQLUINTEGER)param->length)
        {
          bufflen = param->length;
        }
        memcpy((char *)buff + i * cbValueMax, param->buffer, bufflen);
        param->strLenArray[i] = bufflen;
        // Free the memory param->buffer as data is already copied
        if (param->isBuffer != true)
          FREE(param->buffer);
      }
      param->buffer = buff;
      param->buffer_length = cbValueMax;
      param->size = cbValueMax;
    }
    break; // From outer for loop.
  }

  DEBUG_PRINTF("ODBC::GetArrayParam: param%u : paramtype=%u, c_type=%i, "
               "type=%i, size=%i, decimals=%i, buffer=%lld, buffer_length=%i"
               ", length=%i, arraySize=%d, strLenArray[0]=%i\n",
               num, param->paramtype, param->c_type, param->type, param->size,
               param->decimals, *((char *)param->buffer), param->buffer_length,
               param->length, param->arraySize, param->strLenArray[0]);
}

SQLRETURN ODBC::BindParameters(SQLHSTMT hSTMT, Parameter params[], int count)
{
  SQLRETURN ret = SQL_SUCCESS;
  Parameter prm;

  for (int i = 0; i < count; i++)
  {
    prm = params[i];

    DEBUG_PRINTF(
        "ODBC::BindParameters - param[%i]: c_type=%i type=%i "
        "buffer_length=%i size=%i length=%i length=%i\n",
        i, prm.c_type, prm.type,
        prm.buffer_length, prm.size, prm.length, params[i].length);

    if (prm.paramtype == FILE_PARAM) // FILE
    {
      ret = SQLBindFileToParam(
          hSTMT,                     // StatementHandle,
          i + 1,                     // TargetType,
          prm.type,                  // DataType,
          (SQLCHAR *)prm.buffer,     // *FileName,
          NULL,                      // *FileNameLength, // NULL or SQL_NTS
          &params[i].fileOption,     // *FileOptions,  // SQL_FILE_READ = 2
          prm.decimals,              // MaxFileNameLength,
          &params[i].fileIndicator); // *IndicatorValue); // 0
    }
    else if (prm.arraySize > 0) // Bind Array Parameter
    {
      ret = SQLBindParameter(
          hSTMT,             // StatementHandle
          i + 1,             // ParameterNumber
          prm.paramtype,     // InputOutputType
          prm.c_type,        // ValueType
          prm.type,          // ParameterType
          prm.size,          // ColumnSize
          prm.decimals,      // DecimalDigits
          prm.buffer,        // ParameterValuePtr
          prm.buffer_length, // BufferLength
          prm.strLenArray);  // StrLen_or_IndPtr
    }
    else
      ret = SQLBindParameter(
          hSTMT,             // StatementHandle
          i + 1,             // ParameterNumber
          prm.paramtype,     // InputOutputType
          prm.c_type,        // ValueType
          prm.type,          // ParameterType
          prm.size,          // ColumnSize
          prm.decimals,      // DecimalDigits
          prm.buffer,        // ParameterValuePtr
          prm.buffer_length, // BufferLength
          // using &prm.length did not work here...
          &params[i].length); // StrLen_or_IndPtr

    if (ret == SQL_ERROR)
    {
      break;
    }
  }
  return ret;
}

/*
 * CallbackSQLError
 */

Local<Value> ODBC::CallbackSQLError(
    SQLSMALLINT handleType,
    SQLHANDLE handle,
    Nan::Callback *cb)
{
  Nan::EscapableHandleScope scope;

  Local<Value> objError = CallbackSQLError(
      handleType,
      handle,
      (char *)"[node-ibm_db] SQL_ERROR",
      cb);
  return scope.Escape(objError);
}

Local<Value> ODBC::CallbackSQLError(SQLSMALLINT handleType,
                                    SQLHANDLE handle,
                                    char *message,
                                    Nan::Callback *cb)
{
  Nan::EscapableHandleScope scope;

  Local<Value> objError = ODBC::GetSQLError(
      handleType,
      handle,
      message);

  Local<Value> info[1];
  info[0] = objError;
  cb->Call(1, info);

  return scope.Escape(Nan::Undefined());
}

/*
 * GetSQLError
 */

Local<Value> ODBC::GetSQLError(SQLSMALLINT handleType, SQLHANDLE handle)
{
  Nan::EscapableHandleScope scope;

  return scope.Escape(GetSQLError(
      handleType,
      handle,
      (char *)"[ibm_db] SQL_ERROR"));
}

Local<Value> ODBC::GetSQLError(SQLSMALLINT handleType, SQLHANDLE handle, char *message)
{
  Nan::EscapableHandleScope scope;

  DEBUG_PRINTF("ODBC::GetSQLError : handleType=%i, handle=%X\n", handleType, handle);

  Local<Object> objError = Nan::New<Object>();

  SQLINTEGER i = 0;
  SQLINTEGER native;

  SQLSMALLINT len;
  SQLRETURN ret;
  char errorSQLState[14];
  char errorMessage[SQL_MAX_MESSAGE_LENGTH];

  // Windows seems to define SQLINTEGER as long int, unixodbc as just int... %i should cover both
  // Local<Array> errors = Nan::New<Array>();
  // objError->Set(Nan::New("errors").ToLocalChecked(), errors);

  do
  {
    DEBUG_PRINTF("ODBC::GetSQLError : calling SQLGetDiagRec; i=%i\n", i);

    ret = SQLGetDiagRec(
        handleType,
        handle,
        i + 1,
        (SQLTCHAR *)errorSQLState,
        &native,
        (SQLTCHAR *)errorMessage,
        sizeof(errorMessage),
        &len);

    DEBUG_PRINTF("ODBC::GetSQLError : after SQLGetDiagRec; i=%i, ret=%i\n", i, ret);

    if (ret == -2 && i == 0)
    {
      strcpy(errorMessage, "CLI0600E Invalid connection handle or connection is closed. SQLSTATE=S1000");
      strcpy(errorSQLState, "S1000");
      native = -1;
      ret = 0;
    }
    if (SQL_SUCCEEDED(ret))
    {
      if (i == 0)
      {
        DEBUG_TPRINTF(SQL_T("ODBC::GetSQLError : errorMessage=%s, errorSQLState=%s\n"), errorMessage, errorSQLState);

        Nan::Set(objError, Nan::New("error").ToLocalChecked(), Nan::New(message).ToLocalChecked());
        Nan::Set(objError, Nan::New("sqlcode").ToLocalChecked(), Nan::New((double)native));
#ifdef UNICODE
        Nan::SetPrototype(objError, Exception::Error(Nan::New((uint16_t *)errorMessage).ToLocalChecked()));
        Nan::Set(objError, Nan::New("message").ToLocalChecked(), Nan::New((uint16_t *)errorMessage).ToLocalChecked());
        Nan::Set(objError, Nan::New("sqlstate").ToLocalChecked(), Nan::New((uint16_t *)errorSQLState).ToLocalChecked());
#else
        Nan::SetPrototype(objError, Exception::Error(Nan::New(errorMessage).ToLocalChecked()));
        Nan::Set(objError, Nan::New("message").ToLocalChecked(), Nan::New(errorMessage).ToLocalChecked());
        Nan::Set(objError, Nan::New("sqlstate").ToLocalChecked(), Nan::New(errorSQLState).ToLocalChecked());
#endif
      }
      if (native == -1)
      {
        break;
      }
    }
    else
    {
      break;
    }
    i++;
  } while (ret != SQL_NO_DATA);

  return scope.Escape(objError);
}

/*
 * GetAllRecordsSync
 */

Local<Array> ODBC::GetAllRecordsSync(SQLHENV hENV,
                                     SQLHDBC hDBC,
                                     SQLHSTMT hSTMT,
                                     uint16_t *buffer,
                                     size_t bufferLength)
{
  DEBUG_PRINTF("ODBC::GetAllRecordsSync\n");

  Nan::EscapableHandleScope scope;

  Local<Value> objError = Nan::New<Object>();

  int count = 0;
  int errorCount = 0;
  short colCount = 0;

  Column *columns = GetColumns(hSTMT, &colCount);

  Local<Array> rows = Nan::New<Array>();

  // loop through all records
  while (true)
  {
    SQLRETURN ret = SQLFetch(hSTMT);

    // check to see if there was an error
    if (ret == SQL_ERROR)
    {
      errorCount++;

      objError = ODBC::GetSQLError(
          SQL_HANDLE_STMT,
          hSTMT,
          (char *)"[node-ibm_db] Error in ODBC::GetAllRecordsSync");

      break;
    }

    // check to see if we are at the end of the recordset
    if (ret == SQL_NO_DATA)
    {
      break;
    }

    Nan::Set(rows,
             Nan::New(count),
             ODBC::GetRecordTuple(
                 hSTMT,
                 columns,
                 &colCount,
                 buffer,
                 bufferLength));

    count++;
  }
  ODBC::FreeColumns(columns, &colCount);
  return scope.Escape(rows);
}

extern "C" NODE_MODULE_EXPORT void NODE_MODULE_INITIALIZER(Local<Object> exports, Local<Value> module, Local<Context> context)
{
  ODBC::Init(exports);
  ODBCResult::Init(exports);
  ODBCConnection::Init(exports);
  ODBCStatement::Init(exports);
}

extern "C" NAN_MODULE_INIT(init)
{
  ODBC::Init(target);
  ODBCResult::Init(target);
  ODBCConnection::Init(target);
  ODBCStatement::Init(target);
}

NODE_MODULE(odbc_bindings, init)

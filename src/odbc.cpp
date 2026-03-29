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

// On AIX, GCC auto-generates _GLOBAL__FI_*/_GLOBAL__FD_* init/fini functions
// in every shared library via crtcxa.o. The fini function calls __cxa_finalize
// which crashes with SIGILL (jump to 0x0) because atexit handlers registered
// by libdb2.a become invalid during process exit. (#439, #1045)
// We override the init/fini with this no-op to prevent the crash.
#ifdef _AIX
extern "C" void __ibmdb_aix_noop(void) {}
#endif

#include <string.h>
#include <stdint.h>
#include <type_traits>
#include <time.h>
#include <cassert>
#include <cstdlib>
#include <uv.h>
#ifdef _AIX
#include <signal.h>
#include <unistd.h>
#endif

#include "odbc.h"
#include "odbc_connection.h"
#include "odbc_result.h"
#include "odbc_statement.h"

#ifdef _WIN32
#include "strptime.h"
#endif

#define FILE_PARAM 3
#define DATA_AT_EXEC_PARAM 5

namespace {
template <typename HandleType>
unsigned long long HandleToLogValueImpl(HandleType handle, std::true_type)
{
  return static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(handle));
}

template <typename HandleType>
unsigned long long HandleToLogValueImpl(HandleType handle, std::false_type)
{
  return static_cast<unsigned long long>(handle);
}

template <typename HandleType>
unsigned long long HandleToLogValue(HandleType handle)
{
  return HandleToLogValueImpl(handle, typename std::is_pointer<HandleType>::type());
}
}

bool g_shuttingDown = false;

uv_mutex_t ODBC::g_odbcMutex;
uv_async_t ODBC::g_async;

Napi::FunctionReference* ODBC::constructor = nullptr;

Napi::Object ODBC::Init(Napi::Env env, Napi::Object exports)
{
  DEBUG_PRINTF("ODBC::Init\n");

  Napi::Function func = DefineClass(env, "ODBC", {
    InstanceMethod("createConnection", &ODBC::CreateConnection, NAPI_METHOD_ATTR),
    InstanceMethod("createConnectionSync", &ODBC::CreateConnectionSync, NAPI_METHOD_ATTR),
    StaticValue("SQL_CLOSE", Napi::Number::New(env, SQL_CLOSE), napi_enumerable),
    StaticValue("SQL_DROP", Napi::Number::New(env, SQL_DROP), napi_enumerable),
    StaticValue("SQL_UNBIND", Napi::Number::New(env, SQL_UNBIND), napi_enumerable),
    StaticValue("SQL_RESET_PARAMS", Napi::Number::New(env, SQL_RESET_PARAMS), napi_enumerable),
    StaticValue("SQL_DESTROY", Napi::Number::New(env, SQL_DESTROY), napi_enumerable),
    StaticValue("FETCH_ARRAY", Napi::Number::New(env, FETCH_ARRAY), napi_enumerable),
    StaticValue("FETCH_OBJECT", Napi::Number::New(env, FETCH_OBJECT), napi_enumerable),
  });

  constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  constructor->SuppressDestruct();
  exports.Set("ODBC", func);

  // Initialize the cross platform mutex provided by libuv
  uv_mutex_init(&ODBC::g_odbcMutex);

  return exports;
}

ODBC::~ODBC()
{
  DEBUG_PRINTF("ODBC::~ODBC\n");
  this->Free();
}

void ODBC::Free()
{
  DEBUG_PRINTF("ODBC::Free: m_hEnv = 0x%llx\n", HandleToLogValue(m_hEnv));
  if (m_hEnv)
  {
    if (!g_shuttingDown) {
      SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
    }
    m_hEnv = (SQLHENV)NULL;
  }
}

ODBC::ODBC(const Napi::CallbackInfo &info) : Napi::ObjectWrap<ODBC>(info)
{
  Napi::Env env = info.Env();
  DEBUG_PRINTF("ODBC::New - Entry\n");

  m_hEnv = (SQLHENV)NULL;

#ifdef __MVS__
  int ori_mode = __ae_thread_swapmode(__AE_EBCDIC_MODE);
#endif
  int ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);
#ifdef __MVS__
  __ae_thread_swapmode(ori_mode);
#endif
  if (!SQL_SUCCEEDED(ret))
  {
    DEBUG_PRINTF("ODBC::New - ERROR ALLOCATING ENV HANDLE!!\n");
    Napi::Value objError = ODBC::GetSQLError(env, SQL_HANDLE_ENV, m_hEnv);
    Napi::Error::New(env, objError.ToString()).ThrowAsJavaScriptException();
    return;
  }

  ret = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_UINTEGER);

  if (!SQL_SUCCEEDED(ret))
  {
    DEBUG_PRINTF("ODBC::New - ERROR SETTING ODBC VERSION!!\n");
    Napi::Value objError = ODBC::GetSQLError(env, SQL_HANDLE_ENV, m_hEnv);
    SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
    m_hEnv = (SQLHENV)NULL;
    Napi::Error::New(env, objError.ToString()).ThrowAsJavaScriptException();
    return;
  }

  DEBUG_PRINTF("ODBC::New - Exit\n");
}

/*
 * CreateConnection
 */
Napi::Value ODBC::CreateConnection(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBC::CreateConnection - Entry\n");
  Napi::Env env = info.Env();

  REQ_FUN_ARG(0, cb);

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  create_connection_work_data *data =
      (create_connection_work_data *)(calloc(1, sizeof(create_connection_work_data)));
  if (!data) free(work_req);
  MEMCHECK(data);

  data->cb = new Napi::FunctionReference(Napi::Persistent(cb));
  data->dbo = this;
  data->env = env;

  work_req->data = data;

  uv_queue_work(uv_default_loop(), work_req, UV_CreateConnection, (uv_after_work_cb)UV_AfterCreateConnection);

  this->Ref();

  DEBUG_PRINTF("ODBC::CreateConnection - Exit\n");
  return env.Undefined();
}

void ODBC::UV_CreateConnection(uv_work_t *req)
{
  DEBUG_PRINTF("ODBC::UV_CreateConnection - Entry\n");
  create_connection_work_data *data = (create_connection_work_data *)(req->data);
  data->result = SQLAllocHandle(SQL_HANDLE_DBC, data->dbo->m_hEnv, &data->hDBC);
  DEBUG_PRINTF("ODBC::UV_CreateConnection - Exit: hDBC = 0x%llx\n", HandleToLogValue(data->hDBC));
}

void ODBC::UV_AfterCreateConnection(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBC::UV_AfterCreateConnection - Entry\n");
  create_connection_work_data *data = (create_connection_work_data *)(req->data);
  Napi::Env env(data->env);
  Napi::HandleScope scope(env);

  if (!SQL_SUCCEEDED(data->result))
  {
    Napi::Value error = ODBC::GetSQLError(env, SQL_HANDLE_ENV, data->dbo->m_hEnv);
    data->cb->Call({error});
  }
  else
  {
    Napi::Value args[2] = {
      Napi::External<void>::New(env, (void *)(intptr_t)data->dbo->m_hEnv),
      Napi::External<void>::New(env, (void *)(intptr_t)data->hDBC)
    };
    Napi::Object js_result = ODBCConnection::constructor->New({args[0], args[1]});
    data->cb->Call({env.Null(), js_result});
  }
  PropagateCallbackException(env);

  data->dbo->Unref();
  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBC::UV_AfterCreateConnection - Exit\n");
}

/*
 * CreateConnectionSync
 */
Napi::Value ODBC::CreateConnectionSync(const Napi::CallbackInfo &info)
{
  DEBUG_PRINTF("ODBC::CreateConnectionSync - Entry\n");
  Napi::Env env = info.Env();

  SQLHDBC hDBC;
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &hDBC);

  if (!SQL_SUCCEEDED(ret))
  {
    // TODO: do something!
  }

  Napi::Value params[2] = {
    Napi::External<void>::New(env, (void *)(intptr_t)m_hEnv),
    Napi::External<void>::New(env, (void *)(intptr_t)hDBC)
  };
  Napi::Object js_result = ODBCConnection::constructor->New({params[0], params[1]});

  DEBUG_PRINTF("ODBC::CreateConnectionSync - Exit: hDBC = 0x%llx\n", HandleToLogValue(hDBC));
  return js_result;
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

  *colCount = 0;
  ret = SQLNumResultCols(hStmt, colCount);

  if (!SQL_SUCCEEDED(ret))
  {
    return new Column[0];
  }

  Column *columns = new Column[*colCount];
  char colname[MAX_FIELD_SIZE];

  for (int i = 0; i < *colCount; i++)
  {
    columns[i].index = i + 1;
    columns[i].getData = false;
    columns[i].isFileBound = false;
    colname[0] = '\0';
    buflen = 0;

    ret = SQLColAttribute(hStmt, columns[i].index,
#ifdef STRICT_COLUMN_NAMES
                          SQL_DESC_NAME,
#else
                          SQL_DESC_LABEL,
#endif
                          colname, (SQLSMALLINT)MAX_FIELD_SIZE, &buflen, NULL);

    columns[i].name_len = buflen;
    if (buflen > 0)
    {
      columns[i].name = new unsigned char[buflen + 2];
      memcpy(columns[i].name, colname, buflen);
      columns[i].name[buflen] = '\0';
      columns[i].name[buflen + 1] = '\0';
    }
    DEBUG_PRINTF("ODBC::GetColumns index = %i, buflen=%i\n", columns[i].index, buflen);

    ret = SQLColAttribute(hStmt, columns[i].index, SQL_DESC_DISPLAY_SIZE,
                          NULL, 0, NULL, &columns[i].max_display_len);
    ret = SQLColAttribute(hStmt, columns[i].index, SQL_DESC_PRECISION,
                          NULL, 0, NULL, &columns[i].precision);
    ret = SQLColAttribute(hStmt, columns[i].index, SQL_DESC_SCALE,
                          NULL, 0, NULL, &columns[i].scale);
    ret = SQLColAttribute(hStmt, columns[i].index, SQL_DESC_LENGTH,
                          NULL, 0, NULL, &columns[i].field_len);
    ret = SQLColAttribute(hStmt, columns[i].index, SQL_DESC_CONCISE_TYPE,
                          NULL, 0, NULL, &columns[i].type);

    columns[i].type_name = new unsigned char[(MAX_FIELD_SIZE)];
    columns[i].type_name[0] = '\0';

    ret = SQLColAttribute(hStmt, columns[i].index, SQL_DESC_TYPE_NAME,
                          columns[i].type_name, (SQLSMALLINT)(MAX_FIELD_SIZE),
                          &typebuflen, NULL);
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
 */
Napi::Value ODBC::GetColumnValue(Napi::Env env, SQLHSTMT hStmt, Column column,
                                  uint16_t *buffer, size_t bufferLength)
{
  if (column.isFileBound) {
    return env.Null();
  }

  SQLLEN len = 0;
  SQLLEN maxBuffSize = 0;
  unsigned short terCharLen = 1;
  SQLRETURN ret = SQL_SUCCESS;
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
  case SQL_BINARY:
  case SQL_VARBINARY:
  case SQL_LONGVARBINARY:
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
    ret = SQLGetData(hStmt, column.index, SQL_C_SLONG, &value, sizeof(value), &len);

    if (len == SQL_NULL_DATA) return env.Null();
    return Napi::Number::New(env, (double)value);
  }
  break;

  case SQL_NUMERIC:
  case SQL_DECIMAL:
  case SQL_DECFLOAT:
  case SQL_FLOAT:
  case SQL_REAL:
  case SQL_DOUBLE:
  {
    double value;
    ret = SQLGetData(hStmt, column.index, SQL_C_DOUBLE, &value, sizeof(value), &len);

    if (len == SQL_NULL_DATA) return env.Null();
    return Napi::Number::New(env, value);
  }
  break;

  case SQL_DATETIME:
  case SQL_TIMESTAMP:
  {
    SQL_TIMESTAMP_STRUCT odbcTime;
#ifdef _WIN32
    struct tm timeInfo = {};
    ret = SQLGetData(hStmt, column.index, SQL_C_CHAR, &odbcTime, sizeof(odbcTime), &len);
#else
#if defined(_AIX) || defined(__MVS__)
    struct tm timeInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
    struct tm timeInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
    ret = SQLGetData(hStmt, column.index, SQL_C_TYPE_TIMESTAMP, &odbcTime, sizeof(odbcTime), &len);
#endif

    if (len == SQL_NULL_DATA) return env.Null();

    timeInfo.tm_year = odbcTime.year - 1900;
    timeInfo.tm_mon = odbcTime.month - 1;
    timeInfo.tm_mday = odbcTime.day;
    timeInfo.tm_hour = odbcTime.hour;
    timeInfo.tm_min = odbcTime.minute;
    timeInfo.tm_sec = odbcTime.second;
    timeInfo.tm_isdst = -1;

#ifdef TIMEGM
    return Napi::Date::New(env, (double(timegm(&timeInfo)) * 1000) + (odbcTime.fraction / 1000000));
#else
    return Napi::Date::New(env, (double(mktime(&timeInfo)) * 1000) + (odbcTime.fraction / 1000000));
#endif
  }
  break;

  case SQL_BIT:
  {
    char bit[4] = {'\0'};
    ret = SQLGetData(hStmt, column.index, SQL_C_CHAR, &bit, 4, &len);
    if (len == SQL_NULL_DATA) return env.Null();
    return Napi::Boolean::New(env, (*bit == '0') ? false : true);
  }
  break;

  case SQL_TYPE_TIME:
  case SQL_BIGINT:
    if (column.type == SQL_BIGINT) ctype = SQL_C_CHAR;
    /* fall through */
  case SQL_DBCLOB:
    if (column.type == SQL_DBCLOB) ctype = SQL_C_CHAR;
    /* fall through */
  default:
  {
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
        Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, hStmt, errmsg).ToString()).ThrowAsJavaScriptException();
        return env.Undefined();
      }
    }
    maxBuffSize = bufferSize;
    memset(buffer, '\0', bufferSize);
    ret = SQLGetData(hStmt, column.index, ctype, (char *)buffer, maxBuffSize, &len);

    if (column.getData) { secondGetData = 1; }
    else if (ret == SQL_SUCCESS_WITH_INFO && len > 0)
    {
      if (len + terCharLen > 0x3fffffff)
        newbufflen = 0x3fffffff;
      else
        newbufflen = len + terCharLen;

      tmp_out_ptr = (uint16_t *)malloc(newbufflen);
      if (tmp_out_ptr == NULL)
      {
        ret = (SQLRETURN)-3;
        newbufflen = bufferSize;
      }
      else
      {
        memcpy(tmp_out_ptr, (char *)buffer, bufferLength);
        free((uint16_t *)buffer);
        buffer = tmp_out_ptr;
        maxBuffSize = newbufflen - bufferLength;
        len = 0;
        ret = SQLGetData(hStmt, column.index, ctype, (char *)buffer + bufferLength, maxBuffSize, &len);
        ret = SQL_SUCCESS;
        len = newbufflen;
        secondGetData = 1;
      }
    }
    if (len >= 0) newbufflen = len;

    if (len == SQL_NULL_DATA)
    {
      FREE(buffer);
      return env.Null();
    }
    else if (SQL_SUCCEEDED(ret) || secondGetData)
    {
      if (ctype == SQL_C_BINARY)
      {
        // Return binary data as buffer - transfer ownership to Node
        Napi::Buffer<char> buf = Napi::Buffer<char>::Copy(env, (char *)buffer, newbufflen);
        FREE(buffer);
        return buf;
      }
      else
      {
#ifdef UNICODE
        Napi::Value str = Napi::String::New(env, (char16_t *)buffer);
#else
        Napi::Value str = Napi::String::New(env, (char *)buffer);
#endif
        FREE(buffer);
        return str;
      }
    }
    else
    {
      FREE(buffer);
      if (ret == SQL_INVALID_HANDLE)
      {
        unsigned long long hstmt_value = HandleToLogValue(hStmt);
#ifdef __MVS__
        __fprintf_a(stdout, "Invalid Handle: SQLGetData return code = %i, stmt handle = 0x%llx"
                ", columnType = %i, index = %i\n",
              ret, hstmt_value, (int)column.type, column.index);
#else
        fprintf(stdout, "Invalid Handle: SQLGetData return code = %i, stmt handle = 0x%llx"
            ", columnType = %i, index = %i\n",
          ret, hstmt_value, (int)column.type, column.index);
#endif
        assert(ret != SQL_INVALID_HANDLE);
      }
      Napi::Error::New(env, ODBC::GetSQLError(env, SQL_HANDLE_STMT, hStmt, errmsg).ToString()).ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }
  }
}

/*
 * GetOutputParameter
 */
Napi::Value ODBC::GetOutputParameter(Napi::Env env, Parameter *prm)
{
  bool sqlTypeBinary = false;

  switch (prm->type)
  {
  case SQL_BLOB:
  case SQL_BINARY:
  case SQL_VARBINARY:
  case SQL_LONGVARBINARY:
    sqlTypeBinary = true;
    break;
  }

  if (((int)prm->length == SQL_NULL_DATA) || !prm->buffer)
  {
    return env.Null();
  }

  switch (prm->type)
  {
  case SQL_INTEGER:
  case SQL_SMALLINT:
  case SQL_TINYINT:
  case SQL_BOOLEAN:
  case SQL_NUMERIC:
  case SQL_BIGINT:
    if (prm->length == sizeof(int))
      return Napi::Number::New(env, *(int *)prm->buffer);
    else if (prm->length == sizeof(short))
      return Napi::Number::New(env, *(short *)prm->buffer);
    else if (prm->length == sizeof(long))
      return Napi::Number::New(env, (double)*(long *)prm->buffer);
    else if (prm->length == sizeof(long long))
      return Napi::Number::New(env, (double)*(long long *)prm->buffer);
    else
      return Napi::Number::New(env, *(char *)prm->buffer);
    break;

  case SQL_DECIMAL:
  case SQL_FLOAT:
  case SQL_DECFLOAT:
  case SQL_REAL:
  case SQL_DOUBLE:
    return Napi::Number::New(env, *(double *)prm->buffer);
    break;

  case SQL_BIT:
    return Napi::Boolean::New(env, (*((char *)prm->buffer) == '0') ? false : true);
    break;

  default:
    if (prm->isBuffer)
    {
      return Napi::Buffer<char>::Copy(env, (char *)prm->buffer, prm->length);
    }
    else if (sqlTypeBinary || prm->c_type == SQL_C_BINARY)
    {
      prm->isBuffer = true;
      return Napi::Buffer<char>::Copy(env, (char *)prm->buffer, prm->length);
    }
    else
    {
#ifdef UNICODE
      return Napi::String::New(env, (char16_t *)prm->buffer);
#else
      return Napi::String::New(env, (char *)prm->buffer);
#endif
    }
  }
}

/*
 * GetRecordTuple
 */
Napi::Object ODBC::GetRecordTuple(Napi::Env env, SQLHSTMT hStmt, Column *columns,
                                   short *colCount, uint16_t *buffer, size_t bufferLength)
{
  Napi::Object tuple = Napi::Object::New(env);

  for (int i = 0; i < *colCount; i++)
  {
#ifdef UNICODE
    tuple.Set(Napi::String::New(env, (char16_t *)columns[i].name),
              GetColumnValue(env, hStmt, columns[i], buffer, bufferLength));
#else
    tuple.Set(Napi::String::New(env, (const char *)columns[i].name),
              GetColumnValue(env, hStmt, columns[i], buffer, bufferLength));
#endif
  }

  return tuple;
}

/*
 * GetRecordArray
 */
Napi::Value ODBC::GetRecordArray(Napi::Env env, SQLHSTMT hStmt, Column *columns,
                                  short *colCount, uint16_t *buffer, size_t bufferLength)
{
  Napi::Array array = Napi::Array::New(env);

  for (int i = 0; i < *colCount; i++)
  {
    array.Set(i, GetColumnValue(env, hStmt, columns[i], buffer, bufferLength));
  }

  return array;
}

/*
 * GetParametersFromArray
 */
Parameter *ODBC::GetParametersFromArray(Napi::Env env, Napi::Array values, int *paramCount)
{
  DEBUG_PRINTF("ODBC::GetParametersFromArray\n");
  *paramCount = values.Length();

  Parameter *params = (Parameter *)malloc(*paramCount * sizeof(Parameter));
  if (*paramCount != 0 && !params)
  {
    Napi::Error::New(env, "Could not allocate enough memory for params").ThrowAsJavaScriptException();
    return params;
  }
  memset(params, '\0', *paramCount * sizeof(Parameter));

  for (int i = 0; i < *paramCount; i++)
  {
    Napi::Value value = values.Get(i);
    params[i].paramtype = SQL_PARAM_INPUT;
    params[i].size = 0;
    params[i].length = SQL_NULL_DATA;
    params[i].buffer_length = 0;
    params[i].decimals = 0;
    params[i].isBuffer = false;

    if (value.IsArray())
    {
      Napi::Array paramArray = value.As<Napi::Array>();
      int arrlen = paramArray.Length();
      if (arrlen < 4)
      {
        Napi::Error::New(env, "Wrong param format!").ThrowAsJavaScriptException();
        return params;
      }

      Napi::Value val = paramArray.Get((uint32_t)0);
      if (val.IsNumber()) params[i].paramtype = val.As<Napi::Number>().Int32Value();

      val = paramArray.Get((uint32_t)1);
      if (val.IsNumber()) params[i].c_type = val.As<Napi::Number>().Int32Value();
      else params[i].c_type = SQL_C_CHAR;

      val = paramArray.Get((uint32_t)2);
      if (val.IsNumber()) params[i].type = val.As<Napi::Number>().Int32Value();
      else params[i].type = SQL_CHAR;

      if (arrlen == 5)
      {
        val = paramArray.Get((uint32_t)4);
        if (val.IsNumber()) params[i].buffer_length = val.As<Napi::Number>().Int32Value();
      }

      val = paramArray.Get((uint32_t)3);
      if (params[i].paramtype == DATA_AT_EXEC_PARAM && val.IsArray())
      {
        GetDataAtExecParam(env, val, &params[i], i + 1);
      }
      else if (val.IsNull() || val.IsUndefined())
      {
        GetNullParam(&params[i], i + 1);
      }
      else if (val.IsNumber() && !val.IsNull() && params[i].type != 3)
      {
        // Check if integer
        double dval = val.As<Napi::Number>().DoubleValue();
        int32_t ival = val.As<Napi::Number>().Int32Value();
        if (dval == (double)ival && params[i].type != 3)
          GetInt32Param(env, val, &params[i], i + 1);
        else
          GetNumberParam(env, val, &params[i], i + 1);
      }
      else if (val.IsBoolean())
      {
        GetBoolParam(env, val, &params[i], i + 1);
      }
      else if (val.IsArray())
      {
        GetArrayParam(env, val, &params[i], i + 1);
      }
      else if (val.IsBuffer())
      {
        GetBufferParam(val, &params[i], i + 1);
      }
      else
      {
        GetStringParam(env, val, &params[i], i + 1);
      }
    }
    else if (value.IsBuffer())
    {
      GetBufferParam(value, &params[i], i + 1);
    }
    else if (value.IsString())
    {
      GetStringParam(env, value, &params[i], i + 1);
    }
    else if (value.IsNull() || value.IsUndefined())
    {
      GetNullParam(&params[i], i + 1);
    }
    else if (value.IsNumber())
    {
      double dval = value.As<Napi::Number>().DoubleValue();
      int32_t ival = value.As<Napi::Number>().Int32Value();
      if (dval == (double)ival)
        GetInt32Param(env, value, &params[i], i + 1);
      else
        GetNumberParam(env, value, &params[i], i + 1);
    }
    else if (value.IsBoolean())
    {
      GetBoolParam(env, value, &params[i], i + 1);
    }
  }
  return params;
}

void ODBC::GetStringParam(Napi::Env env, Napi::Value value, Parameter *param, int num)
{
  int length = 0;
  int bufflen = 0;
  int terCharLen = 1;
  bool isBinary = (param->c_type == SQL_C_BINARY);

  // Determine string length using encoding that matches NAN behavior:
  // SQL_C_BINARY -> Latin-1 (WriteOneByte), UNICODE -> UTF-16 (Write), else -> UTF-8
  if (isBinary) {
    size_t latin1_len = 0;
    napi_get_value_string_latin1(env, value, NULL, 0, &latin1_len);
    length = (int)latin1_len;
  }
#ifdef UNICODE
  else {
    size_t utf16_len = 0;
    napi_get_value_string_utf16(env, value, NULL, 0, &utf16_len);
    length = (int)utf16_len;
  }
#else
  else {
    std::string utf8str = value.ToString().Utf8Value();
    length = (int)utf8str.length();
  }
#endif

  bufflen = length;
  param->length = length;
  if (!param->c_type || (param->c_type == SQL_CHAR))
    param->c_type = SQL_C_TCHAR;
#ifdef UNICODE
  param->length = SQL_NTS;
  if (!param->type || (param->type == SQL_CHAR))
    param->type = (length >= 8000) ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
  if (!isBinary)
    bufflen = (length + 1) * sizeof(uint16_t);
  terCharLen = 2;
#else
  if (!param->type || (param->type == SQL_CHAR))
    param->type = (length >= 8000) ? SQL_LONGVARCHAR : SQL_VARCHAR;
  if (!isBinary)
  {
    std::string utf8str = value.ToString().Utf8Value();
    bufflen = (int)utf8str.length() + 1;
    param->length = bufflen - 1;
  }
#endif
  if (isBinary) { bufflen = length + 1; }
  if (bufflen <= param->buffer_length &&
      ((param->paramtype % 2 == 0) || (param->arraySize > 0)))
  {
    bufflen = param->buffer_length + terCharLen;
  }
  param->buffer_length = bufflen;
  param->size = bufflen;

  param->buffer = malloc(param->buffer_length);
  if (!param->buffer) return;
  memset(param->buffer, 0, param->buffer_length);

  // Copy string data using correct encoding (matching NAN behavior)
  if (param->paramtype == FILE_PARAM) {
    // FILE_PARAM: use UTF-8 (matches NAN's WriteUtf8)
    std::string utf8str = value.ToString().Utf8Value();
    memcpy(param->buffer, utf8str.c_str(), utf8str.length());
  } else if (isBinary) {
    // SQL_C_BINARY: use Latin-1 (matches NAN's WriteOneByte)
    size_t copied = 0;
    napi_get_value_string_latin1(env, value, (char *)param->buffer, param->buffer_length, &copied);
  } else {
#ifdef UNICODE
    // UNICODE text: use UTF-16 (matches NAN's String::Write)
    size_t copied = 0;
    napi_get_value_string_utf16(env, value, (char16_t *)param->buffer, length + 1, &copied);
#else
    // Non-UNICODE text: use UTF-8 (matches NAN's WriteUtf8)
    std::string utf8str = value.ToString().Utf8Value();
    memcpy(param->buffer, utf8str.c_str(), utf8str.length());
#endif
  }

#ifdef UNICODE
  ((char *)param->buffer)[param->buffer_length - 2] = '\0';
#endif
  ((char *)param->buffer)[param->buffer_length - 1] = '\0';

  if (param->paramtype == FILE_PARAM)
  {
    param->decimals = param->buffer_length;
    param->fileOption = SQL_FILE_READ;
    param->fileIndicator = 0;
  }
}

void ODBC::GetBufferParam(Napi::Value value, Parameter *param, int num)
{
  Napi::Buffer<char> buffer = value.As<Napi::Buffer<char>>();
  param->buffer = (char *)buffer.Data();
  param->length = buffer.Length();
  int bufflen = param->length;
  param->isBuffer = true;

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
  param->length = SQL_NTS;
  if (!param->type || (param->type == SQL_CHAR))
    param->type = (param->length >= 8000) ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
  if (param->c_type != SQL_C_BINARY) param->size /= 2;
#else
  if (!param->type || (param->type == SQL_CHAR))
    param->type = (param->length >= 8000) ? SQL_LONGVARCHAR : SQL_VARCHAR;
#endif

  if (param->paramtype == FILE_PARAM)
  {
    param->decimals = param->buffer_length;
    param->fileOption = SQL_FILE_READ;
    param->fileIndicator = 0;
  }
}

void ODBC::GetDataAtExecParam(Napi::Env env, Napi::Value value, Parameter *param, int num)
{
  Napi::Array chunkArray = value.As<Napi::Array>();
  int numChunks = chunkArray.Length();

  if (numChunks == 0)
  {
    Napi::Error::New(env, "DATA_AT_EXEC: Data array must not be empty").ThrowAsJavaScriptException();
    return;
  }

  param->chunks = (void **)calloc(numChunks, sizeof(void *));
  param->chunkLens = (SQLLEN *)calloc(numChunks, sizeof(SQLLEN));
  if (!param->chunks || !param->chunkLens)
  {
    Napi::Error::New(env, "Failed to allocate chunk arrays for DATA_AT_EXEC").ThrowAsJavaScriptException();
    return;
  }
  param->chunkCount = numChunks;

  SQLLEN totalSize = 0;
  for (int j = 0; j < numChunks; j++)
  {
    Napi::Value chunk = chunkArray.Get(j);
    if (!chunk.IsBuffer())
    {
      Napi::Error::New(env, "DATA_AT_EXEC: each element in Data array must be a Buffer").ThrowAsJavaScriptException();
      return;
    }
    Napi::Buffer<char> buf = chunk.As<Napi::Buffer<char>>();
    param->chunks[j] = buf.Data();
    param->chunkLens[j] = (SQLLEN)buf.Length();
    totalSize += param->chunkLens[j];
  }

  if (param->buffer_length > 0)
    param->size = param->buffer_length;
  else
    param->size = totalSize;

  param->buffer = NULL;
  param->buffer_length = 0;
  param->length = SQL_LEN_DATA_AT_EXEC((SQLLEN)param->size);
  param->isBuffer = true;
}

void ODBC::GetNullParam(Parameter *param, int num)
{
  param->c_type = param->c_type ? param->c_type : SQL_C_DEFAULT;
  param->type = param->type ? param->type : SQL_VARCHAR;
  param->length = SQL_NULL_DATA;

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
}

void ODBC::GetInt32Param(Napi::Env env, Napi::Value value, Parameter *param, int num)
{
  int64_t *number = new int64_t(value.As<Napi::Number>().Int32Value());
  param->c_type = SQL_C_SBIGINT;
  if (!param->type || (param->type == 1)) param->type = SQL_BIGINT;
  param->buffer = number;
  param->length = sizeof(number);
}

void ODBC::GetNumberParam(Napi::Env env, Napi::Value value, Parameter *param, int num)
{
  double *number = new double(value.As<Napi::Number>().DoubleValue());
  std::string string = value.ToString().Utf8Value();
  int length = (int)string.length();
  char *ptr = strchr((char *)string.c_str(), '.');

  if (!param->c_type || (param->c_type == SQL_C_CHAR)) param->c_type = SQL_C_DOUBLE;
  if (!param->type || (param->type == SQL_CHAR)) param->type = SQL_DECIMAL;
  param->buffer = number;
  param->buffer_length = sizeof(double);
  param->length = param->buffer_length;
  param->size = sizeof(double);
  if (ptr)
  {
    param->decimals = length - (ptr - string.c_str()) - 1;
    if (param->type == SQL_DECIMAL) param->size = length - 1;
  }
  else
  {
    param->decimals = 0;
    if (param->type == SQL_DECIMAL) param->size = length;
  }
}

void ODBC::GetBoolParam(Napi::Env env, Napi::Value value, Parameter *param, int num)
{
  bool boolean = value.As<Napi::Boolean>().Value();
  int64_t *number = new int64_t(boolean ? 1 : 0);
  param->c_type = SQL_C_SBIGINT;
  if (!param->type || (param->type == 1)) param->type = SQL_BIGINT;
  param->buffer = number;
  param->length = sizeof(number);
}

void ODBC::GetArrayParam(Napi::Env env, Napi::Value value, Parameter *param, int num)
{
  Napi::Array valueArray = value.As<Napi::Array>();
  int arrlen = valueArray.Length();

  param->arraySize = arrlen;
  if (arrlen <= 0)
  {
    param->buffer = new int[0];
    return;
  }

  param->strLenArray = new SQLINTEGER[arrlen];

  for (int j = 0; j < arrlen; j++)
  {
    Napi::Value val = valueArray.Get(j);
    if (val.IsNull() || val.IsUndefined())
    {
      param->strLenArray[j] = SQL_NULL_DATA;
      if (j == arrlen - 1)
      {
        GetNullParam(param, num);
        param->buffer = new int[arrlen];
      }
      continue;
    }
    else if (val.IsNumber())
    {
      double dval = val.As<Napi::Number>().DoubleValue();
      int32_t ival = val.As<Napi::Number>().Int32Value();
      if (dval == (double)ival && param->type != 3)
      {
        int64_t *number = new int64_t[arrlen];
        for (int i = j; i < arrlen; i++)
        {
          if (i != j)
          {
            val = valueArray.Get(i);
            if (val.IsNull() || val.IsUndefined()) { param->strLenArray[i] = SQL_NULL_DATA; continue; }
          }
          number[i] = val.As<Napi::Number>().Int64Value();
          param->strLenArray[i] = sizeof(int64_t);
        }
        param->c_type = SQL_C_SBIGINT;
        if (!param->type || (param->type == 1)) param->type = SQL_BIGINT;
        param->buffer = number;
      }
      else
      {
        double *number = new double[arrlen];
        int decimals = 0;
        for (int i = j; i < arrlen; i++)
        {
          if (i != j)
          {
            val = valueArray.Get(i);
            if (val.IsNull() || val.IsUndefined()) { param->strLenArray[i] = SQL_NULL_DATA; continue; }
          }
          GetNumberParam(env, val, param, num);
          number[i] = *(double *)param->buffer;
          param->strLenArray[i] = param->length;
          delete (double *)param->buffer;
          param->buffer = NULL;
          decimals = (decimals < param->decimals) ? param->decimals : decimals;
        }
        param->decimals = decimals;
        param->buffer = number;
      }
    }
    else if (val.IsBoolean())
    {
      int64_t *boolean = new int64_t[arrlen];
      for (int i = j; i < arrlen; i++)
      {
        if (i != j)
        {
          val = valueArray.Get(i);
          if (val.IsNull() || val.IsUndefined()) { param->strLenArray[i] = SQL_NULL_DATA; continue; }
        }
        GetBoolParam(env, val, param, num);
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

      SQLPOINTER *elemBufs = new SQLPOINTER[arrlen]();
      SQLULEN *elemLens = new SQLULEN[arrlen]();
      SQLLEN *elemDataLens = new SQLLEN[arrlen]();
      bool *elemIsBuf = new bool[arrlen]();

      for (int i = j; i < arrlen; i++)
      {
        if (i != j)
        {
          val = valueArray.Get(i);
          if (val.IsNull() || val.IsUndefined()) { param->strLenArray[i] = SQL_NULL_DATA; continue; }
        }
        if (val.IsBuffer())
        {
          GetBufferParam(val, param, num);
        }
        else
        {
          GetStringParam(env, val, param, num);
#ifdef UNICODE
          param->buffer_length -= 2;
#else
          param->buffer_length -= 1;
#endif
        }
        elemBufs[i] = param->buffer;
        elemLens[i] = param->buffer_length;
        elemDataLens[i] = param->length;
        elemIsBuf[i] = param->isBuffer;
        param->buffer = NULL;

        if ((SQLULEN)param->buffer_length > cbValueMax)
          cbValueMax = (SQLULEN)param->buffer_length;
      }

      bufflen = (SQLULEN)arrlen * cbValueMax;
      buff = (SQLPOINTER *)malloc(bufflen);
      memset(buff, 0, bufflen);

      for (int i = j; i < arrlen; i++)
      {
        if (elemBufs[i] == NULL) continue;
        SQLULEN copyLen = cbValueMax;
        if (copyLen > (SQLULEN)elemDataLens[i]) copyLen = (SQLULEN)elemDataLens[i];
        memcpy((char *)buff + (SQLULEN)i * cbValueMax, elemBufs[i], copyLen);
        param->strLenArray[i] = (SQLINTEGER)copyLen;
        if (!elemIsBuf[i]) FREE(elemBufs[i]);
      }

      delete[] elemBufs;
      delete[] elemLens;
      delete[] elemDataLens;
      delete[] elemIsBuf;

      param->buffer = buff;
      param->buffer_length = cbValueMax;
      param->size = cbValueMax;
    }
    break;
  }
}

SQLRETURN ODBC::BindParameters(SQLHSTMT hSTMT, Parameter params[], int count)
{
  SQLRETURN ret = SQL_SUCCESS;
  Parameter prm;

  for (int i = 0; i < count; i++)
  {
    prm = params[i];

    if (prm.paramtype == FILE_PARAM)
    {
      ret = SQLBindFileToParam(hSTMT, i + 1, prm.type, (SQLCHAR *)prm.buffer,
                               NULL, &params[i].fileOption, prm.decimals, &params[i].fileIndicator);
    }
    else if (prm.paramtype == DATA_AT_EXEC_PARAM)
    {
      ret = SQLBindParameter(hSTMT, i + 1, SQL_PARAM_INPUT, prm.c_type, prm.type,
                             prm.size, 0, (SQLPOINTER)(intptr_t)(i + 1), 0, &params[i].length);
    }
    else if (prm.arraySize > 0)
    {
      ret = SQLBindParameter(hSTMT, i + 1, prm.paramtype, prm.c_type, prm.type,
                             prm.size, prm.decimals, prm.buffer, prm.buffer_length, prm.strLenArray);
    }
    else
    {
      ret = SQLBindParameter(hSTMT, i + 1, prm.paramtype, prm.c_type, prm.type,
                             prm.size, prm.decimals, prm.buffer, prm.buffer_length, &params[i].length);
    }

    if (ret == SQL_ERROR) break;
  }
  return ret;
}

SQLRETURN ODBC::PutDataLoop(SQLHSTMT hSTMT, Parameter params[], int count)
{
  SQLRETURN ret;
  SQLPOINTER token;

  DEBUG_PRINTF("ODBC::PutDataLoop - Entry\n");

  while ((ret = SQLParamData(hSTMT, &token)) == SQL_NEED_DATA)
  {
    int paramIdx = (int)(intptr_t)token - 1;

    if (paramIdx < 0 || paramIdx >= count ||
        params[paramIdx].chunks == NULL || params[paramIdx].chunkCount == 0)
    {
      return SQL_ERROR;
    }

    for (int j = 0; j < params[paramIdx].chunkCount; j++)
    {
      ret = SQLPutData(hSTMT, params[paramIdx].chunks[j], params[paramIdx].chunkLens[j]);
      if (!SQL_SUCCEEDED(ret)) return ret;
    }
  }

  DEBUG_PRINTF("ODBC::PutDataLoop - Exit, ret=%d\n", ret);
  return ret;
}

/*
 * CallbackSQLError
 */
Napi::Value ODBC::CallbackSQLError(Napi::Env env, SQLSMALLINT handleType,
                                    SQLHANDLE handle, Napi::FunctionReference *cb)
{
  return CallbackSQLError(env, handleType, handle, (char *)"[node-ibm_db] SQL_ERROR", cb);
}

Napi::Value ODBC::CallbackSQLError(Napi::Env env, SQLSMALLINT handleType,
                                    SQLHANDLE handle, char *message,
                                    Napi::FunctionReference *cb)
{
  Napi::Value objError = ODBC::GetSQLError(env, handleType, handle, message);
  cb->Call({objError});
  return env.Undefined();
}

/*
 * GetSQLError
 */
Napi::Value ODBC::GetSQLError(Napi::Env env, SQLSMALLINT handleType, SQLHANDLE handle)
{
  return GetSQLError(env, handleType, handle, (char *)"[ibm_db] SQL_ERROR");
}

Napi::Value ODBC::GetSQLError(Napi::Env env, SQLSMALLINT handleType,
                               SQLHANDLE handle, char *message)
{
  DEBUG_PRINTF("ODBC::GetSQLError : handleType=%i, handle=0x%llx\n", handleType, HandleToLogValue(handle));

  Napi::Object objError = Napi::Object::New(env);

  SQLINTEGER i = 0;
  SQLINTEGER native;
  SQLSMALLINT len;
  SQLRETURN ret;
  char errorSQLState[14];
  char errorMessage[SQL_MAX_MESSAGE_LENGTH];

  do
  {
    ret = SQLGetDiagRec(handleType, handle, i + 1,
                        (SQLTCHAR *)errorSQLState, &native,
                        (SQLTCHAR *)errorMessage, sizeof(errorMessage), &len);

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
        objError.Set("error", Napi::String::New(env, message));
        objError.Set("sqlcode", Napi::Number::New(env, (double)native));
#ifdef UNICODE
        objError.Set("message", Napi::String::New(env, (char16_t *)errorMessage));
        objError.Set("sqlstate", Napi::String::New(env, (char16_t *)errorSQLState));
#else
        objError.Set("message", Napi::String::New(env, errorMessage));
        objError.Set("sqlstate", Napi::String::New(env, errorSQLState));
#endif
      }
      if (native == -1) break;
    }
    else
    {
      break;
    }
    i++;
  } while (ret != SQL_NO_DATA);

  return objError;
}

/*
 * GetAllRecordsSync
 */
Napi::Array ODBC::GetAllRecordsSync(Napi::Env env, SQLHENV hENV, SQLHDBC hDBC,
                                     SQLHSTMT hSTMT, uint16_t *buffer, size_t bufferLength)
{
  DEBUG_PRINTF("ODBC::GetAllRecordsSync\n");

  int count = 0;
  short colCount = 0;
  Column *columns = GetColumns(hSTMT, &colCount);
  Napi::Array rows = Napi::Array::New(env);

  while (true)
  {
    SQLRETURN ret = SQLFetch(hSTMT);

    if (ret == SQL_ERROR)
    {
      // error - break and return what we have
      break;
    }
    if (ret == SQL_NO_DATA) break;

    rows.Set(count, ODBC::GetRecordTuple(env, hSTMT, columns, &colCount, buffer, bufferLength));
    count++;
  }
  ODBC::FreeColumns(columns, &colCount);
  return rows;
}

// Cleanup hook called when the Node.js environment is tearing down.
// Sets a global flag so that C++ destructors skip ODBC driver calls
// (SQLFreeHandle, SQLDisconnect) which would segfault on AIX if the
// ODBC driver shared library has already been unloaded. (#439, #1045)
#ifdef _AIX
// On AIX, GCC's _GLOBAL__FD_node calls __cxa_finalize(NULL) during exit()
// which crashes with SIGILL (jump to 0x0) because libdb2's __cxa_atexit
// handlers become invalid. We install a SIGILL handler during shutdown
// to catch this crash and exit cleanly instead. (#439, #1045)
static void AIX_SigillHandler(int sig)
{
  _exit(0);
}

static void EnvironmentCleanupHook(void* /*arg*/)
{
  g_shuttingDown = true;

  // Install SIGILL handler to catch __cxa_finalize crash during exit.
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = AIX_SigillHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGILL, &sa, NULL);
}
#else
static void EnvironmentCleanupHook(void* /*arg*/)
{
  g_shuttingDown = true;
}
#endif

// atexit handler: belt-and-suspenders to ensure g_shuttingDown is set
// before __cxa_finalize runs static destructors during process exit.
static void AtExitCleanupHandler()
{
  g_shuttingDown = true;
}

// Module initialization
Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
  ODBC::Init(env, exports);
  ODBCResult::Init(env, exports);
  ODBCConnection::Init(env, exports);
  ODBCStatement::Init(env, exports);

  // On AIX, pass the napi_env so the cleanup hook can read process.exitCode.
  napi_add_env_cleanup_hook(env, EnvironmentCleanupHook, nullptr);
  atexit(AtExitCleanupHandler);

  return exports;
}

NODE_API_MODULE(odbc_bindings, InitAll)

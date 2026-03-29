/*
  Copyright (c) 2013, Dan VerWeire <dverweire@gmail.com>
  Copyright (c) 2010, Lee Smith <notwink@gmail.com>

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

#ifndef _SRC_ODBC_H
#define _SRC_ODBC_H

#include <napi.h>
#include <uv.h>
#include <wchar.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <sqlcli1.h>

#define MAX_FIELD_SIZE 1024
#define MAX_VALUE_SIZE 1022

// Property attributes matching NAN behavior: writable + configurable
#define NAPI_METHOD_ATTR static_cast<napi_property_attributes>(napi_writable | napi_configurable)

#define MODE_COLLECT_AND_CALLBACK 1
#define MODE_CALLBACK_FOR_EACH 2
#define FETCH_ARRAY 3
#define FETCH_OBJECT 4
#define FETCH_NODATA 0
#define SQL_DESTROY SQL_DROP

// Workaround(zOS): Db2 supplied headers do not define SQL_SUCCEEDED
#ifndef SQL_SUCCEEDED
#define SQL_SUCCEEDED(rc) (((rc) & (~1)) == 0)
#endif

#ifndef FREE
#define FREE(x) do { if(x) { free(x); x = NULL; } } while(0)
#endif

// Free Bind Parameters
#define FREE_PARAMS(params, count)                              \
  Parameter prm;                                                \
  if (params != NULL)                                           \
  {                                                             \
    for (int i = 0; i < count; i++)                             \
    {                                                           \
      if (prm = params[i], prm.buffer != NULL && !prm.isBuffer) \
      {                                                         \
        switch (prm.c_type)                                     \
        {                                                       \
        case SQL_C_SBIGINT:                                     \
        case SQL_C_LONG:                                        \
          delete (int64_t *)prm.buffer;                         \
          break;                                                \
        case SQL_C_DOUBLE:                                      \
          delete (double *)prm.buffer;                          \
          break;                                                \
        case SQL_C_BIT:                                         \
          delete (bool *)prm.buffer;                            \
          break;                                                \
        case SQL_C_CHAR:                                        \
        case SQL_C_WCHAR:                                       \
        default:                                                \
          free(prm.buffer);                                     \
          prm.buffer = NULL;                                    \
          break;                                                \
        }                                                       \
      }                                                         \
      if (prm.arraySize > 0)                                    \
      {                                                         \
        free(prm.strLenArray);                                  \
        prm.strLenArray = NULL;                                 \
        prm.arraySize = 0;                                      \
      }                                                         \
      if (prm.chunkCount > 0)                                   \
      {                                                         \
        free(prm.chunks);                                       \
        prm.chunks = NULL;                                      \
        free(prm.chunkLens);                                    \
        prm.chunkLens = NULL;                                   \
        prm.chunkCount = 0;                                     \
      }                                                         \
    }                                                           \
    free(params);                                               \
  }                                                             \
  params = NULL;                                                \
  count = 0;

// Define sqltypes which are not defined in SDSNC.H/SQLCLI on z/OS
#ifndef SQL_BOOLEAN
#define SQL_BOOLEAN 16
#define SQL_ROW 19
#endif

// Workaround(zOS): Guard data-at-execution macros
#ifndef SQL_NEED_DATA
#define SQL_NEED_DATA 99
#endif
#ifndef SQL_LEN_DATA_AT_EXEC_OFFSET
#define SQL_LEN_DATA_AT_EXEC_OFFSET (-100)
#endif
#ifndef SQL_LEN_DATA_AT_EXEC
#define SQL_LEN_DATA_AT_EXEC(length) (-(length) + SQL_LEN_DATA_AT_EXEC_OFFSET)
#endif

// two macros ensures that any macro used will be expanded
// before being stringified
#define LINESTRING(x) #x
#define LINENO(x) LINESTRING(x)

typedef struct
{
  unsigned char *name;
  unsigned char *type_name;
  unsigned int name_len;
  SQLSMALLINT max_display_len;
  SQLSMALLINT scale;
  SQLSMALLINT precision;
  SQLSMALLINT field_len;
  SQLLEN type;
  SQLUSMALLINT index;
  bool getData;
  bool isFileBound;
} Column;

typedef struct
{
  SQLSMALLINT paramtype;
  SQLSMALLINT c_type;
  SQLSMALLINT type;
  SQLULEN size;
  SQLSMALLINT decimals;
  SQLPOINTER buffer;
  SQLLEN buffer_length;
  SQLLEN length;
  SQLUINTEGER fileOption;
  SQLINTEGER fileIndicator;
  int arraySize;
  SQLINTEGER *strLenArray;
  bool isBuffer;
  void **chunks;
  SQLLEN *chunkLens;
  int chunkCount;
} Parameter;

// Propagate exceptions from UV callbacks (NAPI equivalent of NAN's FatalException)
static inline void PropagateCallbackException(Napi::Env env) {
  if (env.IsExceptionPending()) {
    Napi::Error err = env.GetAndClearPendingException();
    napi_fatal_exception(env, err.Value());
  }
}

// Flag to indicate the Node.js environment is shutting down.
// When true, destructors must not call ODBC driver functions
// (SQLFreeHandle, SQLDisconnect, etc.) because the ODBC driver
// shared library may already be unloaded, causing a segfault
// on platforms like AIX. (See issues #439 and #1045.)
extern bool g_shuttingDown;

class ODBC : public Napi::ObjectWrap<ODBC>
{
public:
  static Napi::FunctionReference constructor;
  static uv_mutex_t g_odbcMutex;
  static uv_async_t g_async;

  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  ODBC(const Napi::CallbackInfo &info);
  ~ODBC();

  static Column *GetColumns(SQLHSTMT hStmt, short *colCount);
  static void FreeColumns(Column *&columns, short *colCount);
  static Napi::Value GetColumnValue(Napi::Env env, SQLHSTMT hStmt, Column column, uint16_t *buffer, size_t bufferLength);
  static Napi::Value GetOutputParameter(Napi::Env env, Parameter *prm);
  static Napi::Object GetRecordTuple(Napi::Env env, SQLHSTMT hStmt, Column *columns, short *colCount, uint16_t *buffer, size_t bufferLength);
  static Napi::Value GetRecordArray(Napi::Env env, SQLHSTMT hStmt, Column *columns, short *colCount, uint16_t *buffer, size_t bufferLength);
  static Napi::Value CallbackSQLError(Napi::Env env, SQLSMALLINT handleType, SQLHANDLE handle, Napi::FunctionReference *cb);
  static Napi::Value CallbackSQLError(Napi::Env env, SQLSMALLINT handleType, SQLHANDLE handle, char *message, Napi::FunctionReference *cb);
  static Napi::Value GetSQLError(Napi::Env env, SQLSMALLINT handleType, SQLHANDLE handle);
  static Napi::Value GetSQLError(Napi::Env env, SQLSMALLINT handleType, SQLHANDLE handle, char *message);
  static Napi::Array GetAllRecordsSync(Napi::Env env, SQLHENV hENV, SQLHDBC hDBC, SQLHSTMT hSTMT, uint16_t *buffer, size_t bufferLength);
  static Parameter *GetParametersFromArray(Napi::Env env, Napi::Array values, int *paramCount);
  static SQLRETURN BindParameters(SQLHSTMT hSTMT, Parameter params[], int count);
  static SQLRETURN PutDataLoop(SQLHSTMT hSTMT, Parameter params[], int count);

  void Free();

  // Instance methods exposed to JS
  Napi::Value CreateConnection(const Napi::CallbackInfo &info);
  Napi::Value CreateConnectionSync(const Napi::CallbackInfo &info);

protected:
  SQLHENV m_hEnv;

  static void GetStringParam(Napi::Env env, Napi::Value value, Parameter *param, int num);
  static void GetNullParam(Parameter *param, int num);
  static void GetInt32Param(Napi::Env env, Napi::Value value, Parameter *param, int num);
  static void GetNumberParam(Napi::Env env, Napi::Value value, Parameter *param, int num);
  static void GetBoolParam(Napi::Env env, Napi::Value value, Parameter *param, int num);
  static void GetArrayParam(Napi::Env env, Napi::Value value, Parameter *param, int num);
  static void GetBufferParam(Napi::Value value, Parameter *param, int num);
  static void GetDataAtExecParam(Napi::Env env, Napi::Value value, Parameter *param, int num);

  // UV async callbacks
  static void UV_CreateConnection(uv_work_t *work_req);
  static void UV_AfterCreateConnection(uv_work_t *work_req, int status);
};

struct create_connection_work_data
{
  napi_env env;
  Napi::FunctionReference *cb;
  ODBC *dbo;
  SQLHDBC hDBC;
  int result;
};

struct open_request
{
  Napi::FunctionReference *cb;
  ODBC *dbo;
  int result;
  char connection[1];
};

struct close_request
{
  Napi::FunctionReference *cb;
  ODBC *dbo;
  int result;
};

struct query_request
{
  Napi::FunctionReference *cb;
  ODBC *dbo;
  SQLHSTMT hSTMT;
  int affectedRows;
  char *sql;
  char *catalog;
  char *schema;
  char *table;
  char *type;
  char *column;
  Parameter *params;
  int paramCount;
  int result;
};

#ifdef UNICODE
#define SQL_T(x) (L##x)
#define UNICHAR uint16_t
#else
#define SQL_T(x) (x)
#define UNICHAR char
#endif

#ifdef DEBUG
#define DEBUG_PRINTF(...) fprintf(stdout, __VA_ARGS__)
#ifdef UNICODE
#define DEBUG_TPRINTF(...) fwprintf(stdout, __VA_ARGS__)
#else
#define DEBUG_TPRINTF(...) fprintf(stdout, __VA_ARGS__)
#endif
#else
#define DEBUG_PRINTF(...) (void)0
#define DEBUG_TPRINTF(...) (void)0
#endif

#define REQ_ARGS(N)                                                              \
  if (info.Length() < (N))                                                       \
  {                                                                              \
    Napi::TypeError::New(env, "Expected " #N " arguments").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                      \
  }

#define REQ_STR_ARG(I, VAR)                                                                 \
  if (info.Length() <= (I) || !info[I].IsString())                                          \
  {                                                                                         \
    Napi::TypeError::New(env, "Argument " #I " must be a string").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                                 \
  }                                                                                         \
  std::string VAR = info[I].As<Napi::String>().Utf8Value();

#define REQ_WSTR_ARG(I, VAR)                                                                \
  if (info.Length() <= (I) || !info[I].IsString())                                          \
  {                                                                                         \
    Napi::TypeError::New(env, "Argument " #I " must be a string").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                                 \
  }                                                                                         \
  std::string VAR = info[I].As<Napi::String>().Utf8Value();

#define REQ_STRO_ARG(I, VAR)                                                                \
  if (info.Length() <= (I) || !info[I].IsString())                                          \
  {                                                                                         \
    Napi::TypeError::New(env, "Argument " #I " must be a string").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                                 \
  }                                                                                         \
  std::string VAR = info[I].As<Napi::String>().Utf8Value();

#define REQ_STR_OR_NULL_ARG(I, VAR)                                                                       \
  if (info.Length() <= (I) || (!info[I].IsString() && !info[I].IsNull()))                                 \
  {                                                                                                       \
    Napi::TypeError::New(env, "Argument " #I " must be a string or null").ThrowAsJavaScriptException();   \
    return env.Undefined();                                                                               \
  }                                                                                                       \
  std::string VAR = info[I].IsNull() ? std::string("null") : info[I].As<Napi::String>().Utf8Value();

#define REQ_STRO_OR_NULL_ARG(I, VAR)                                                                      \
  if (info.Length() <= (I) || (!info[I].IsString() && !info[I].IsNull()))                                 \
  {                                                                                                       \
    Napi::TypeError::New(env, "Argument " #I " must be a string or null").ThrowAsJavaScriptException();   \
    return env.Undefined();                                                                               \
  }                                                                                                       \
  std::string VAR = info[I].IsNull() ? std::string("null") : info[I].As<Napi::String>().Utf8Value();

#define REQ_FUN_ARG(I, VAR)                                                                   \
  if (info.Length() <= (I) || !info[I].IsFunction())                                          \
  {                                                                                           \
    Napi::TypeError::New(env, "Argument " #I " must be a function").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                                   \
  }                                                                                           \
  Napi::Function VAR = info[I].As<Napi::Function>();

#define REQ_BOOL_ARG(I, VAR)                                                                  \
  if (info.Length() <= (I) || !info[I].IsBoolean())                                           \
  {                                                                                           \
    Napi::TypeError::New(env, "Argument " #I " must be a boolean").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                                   \
  }                                                                                           \
  bool VAR = info[I].As<Napi::Boolean>().Value();

#define REQ_EXT_ARG(I, VAR)                                                                \
  if (info.Length() <= (I) || !info[I].IsExternal())                                       \
  {                                                                                        \
    Napi::TypeError::New(env, "Argument " #I " invalid").ThrowAsJavaScriptException();     \
    return env.Undefined();                                                                \
  }                                                                                        \
  Napi::External<void> VAR = info[I].As<Napi::External<void>>();

#define REQ_UINT_ARG(I, VAR)                                                                 \
  if (info.Length() <= (I) || !info[I].IsNumber())                                           \
  {                                                                                          \
    Napi::TypeError::New(env, "Argument " #I " invalid").ThrowAsJavaScriptException();       \
    return env.Undefined();                                                                  \
  }                                                                                          \
  SQLUINTEGER VAR = info[I].As<Napi::Number>().Uint32Value();

#define REQ_INT_ARG(I, VAR)                                                                  \
  if (info.Length() <= (I) || !info[I].IsNumber())                                           \
  {                                                                                          \
    Napi::TypeError::New(env, "Argument " #I " invalid").ThrowAsJavaScriptException();       \
    return env.Undefined();                                                                  \
  }                                                                                          \
  SQLINTEGER VAR = info[I].As<Napi::Number>().Int32Value();

#define OPT_INT_ARG(I, VAR, DEFAULT)                                                             \
  SQLUSMALLINT VAR;                                                                              \
  if (info.Length() <= (I))                                                                      \
  {                                                                                              \
    VAR = (DEFAULT);                                                                             \
  }                                                                                              \
  else if (info[I].IsNumber())                                                                   \
  {                                                                                              \
    VAR = info[I].As<Napi::Number>().Int32Value();                                               \
  }                                                                                              \
  else                                                                                           \
  {                                                                                              \
    Napi::TypeError::New(env, "Argument " #I " must be an integer").ThrowAsJavaScriptException();\
    return env.Undefined();                                                                      \
  }

// Check memory allocated successfully or not (for methods returning Napi::Value)
#define MEMCHECK(buffer)                                                            \
  if (!buffer)                                                                      \
  {                                                                                 \
    Napi::Error::New(env, "Could not allocate enough memory in ibm_db "             \
                          "file " __FILE__ ":" LINENO(__LINE__)).ThrowAsJavaScriptException(); \
    return env.Undefined();                                                         \
  }

// Macro to get c++ string from std::string (JS string via Utf8Value)
#ifdef UNICODE
#define GETCPPSTR(to, from, len)                             \
  if (len > 0 && from != "null")                             \
  {                                                          \
    to = (uint16_t *)malloc((len + 1) * sizeof(uint16_t));   \
    MEMCHECK(to);                                            \
    memcpy(to, from.c_str(), len);                           \
    ((uint16_t *)to)[len] = '\0';                            \
  }                                                          \
  else                                                       \
  {                                                          \
    len = 0;                                                 \
  }
#else
#define GETCPPSTR(to, from, len)     \
  if (len > 0 && from != "null")     \
  {                                  \
    to = (char *)malloc(len + 1);    \
    MEMCHECK(to);                    \
    memcpy(to, from.c_str(), len);   \
    ((char *)to)[len] = '\0';        \
  }                                  \
  else                               \
  {                                  \
    len = 0;                         \
  }
#endif

// Check memory allocated (goto exit variant)
#define MEMCHECK2(buffer, errmsg)                          \
  if (!buffer)                                             \
  {                                                        \
    errmsg = "Could not allocate enough memory in ibm_db " \
             "file " __FILE__ ":" LINENO(__LINE__);        \
    goto exit;                                             \
  }

// Macro to get c++ string from std::string (goto exit variant)
#ifdef UNICODE
#define GETCPPSTR2(to, from, len, errmsg)                    \
  if (len > 0 && from != "null")                             \
  {                                                          \
    to = (uint16_t *)malloc((len + 1) * sizeof(uint16_t));   \
    if (to)                                                  \
    {                                                        \
      memcpy(to, from.c_str(), len);                         \
      ((uint16_t *)to)[len] = '\0';                          \
    }                                                        \
    else                                                     \
    {                                                        \
      errmsg = "Could not allocate enough memory in ibm_db " \
               "file " __FILE__ ":" LINENO(__LINE__);        \
      goto exit;                                             \
    }                                                        \
  }                                                          \
  else                                                       \
  {                                                          \
    len = 0;                                                 \
  }
#else
#define GETCPPSTR2(to, from, len, errmsg)                    \
  if (len > 0 && from != "null")                             \
  {                                                          \
    to = (char *)malloc(len + 1);                            \
    if (to)                                                  \
    {                                                        \
      memcpy(to, from.c_str(), len);                         \
      ((char *)to)[len] = '\0';                              \
    }                                                        \
    else                                                     \
    {                                                        \
      errmsg = "Could not allocate enough memory in ibm_db " \
               "file " __FILE__ ":" LINENO(__LINE__);        \
      goto exit;                                             \
    }                                                        \
  }                                                          \
  else                                                       \
  {                                                          \
    len = 0;                                                 \
  }
#endif

#endif

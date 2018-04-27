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

#include <v8.h>
#include <node.h>
#include <nan.h>
#include <wchar.h>

#include <stdlib.h>
#include <sqlcli1.h>

using namespace v8;
using namespace node;

#define MAX_FIELD_SIZE 1024
#define MAX_VALUE_SIZE 1022

#define MODE_COLLECT_AND_CALLBACK 1
#define MODE_CALLBACK_FOR_EACH 2
#define FETCH_ARRAY 3
#define FETCH_OBJECT 4
#define SQL_DESTROY 9999

// Workaround(zOS): Db2 supplied headers do not define SQL_SUCCEEDED
#ifndef SQL_SUCCEEDED
#define SQL_SUCCEEDED(rc) (((rc)&(~1))==0)
#endif

// Free Bind Parameters 
#define FREE_PARAMS( params, count )                                 \
    Parameter prm;                                                   \
    if(params != NULL ) {                                            \
      for (int i = 0; i < count; i++) {                              \
        if (prm = params[i], prm.buffer != NULL) {                   \
          switch (prm.c_type) {                                      \
            case SQL_C_LONG:    delete (int64_t *)prm.buffer; break; \
            case SQL_C_DOUBLE:  delete (double  *)prm.buffer; break; \
            case SQL_C_BIT:     delete (bool    *)prm.buffer; break; \
            case SQL_C_CHAR:                                         \
            case SQL_C_WCHAR:                                        \
            default:     free(prm.buffer); prm.buffer = NULL; break; \
          }                                                          \
        }                                                            \
      }                                                              \
      free(params);                                                  \
    }                                                                \
    params = NULL;                                                   \
    count = 0;

// two macros ensures that any macro used will be expanded 
// before being stringified. #x gives string value of x.
#define LINESTRING(x) #x
#define LINENO(x) LINESTRING(x)
// Check memory allocated successfully or not.
#define MEMCHECK( buffer )                                           \
  if (!buffer) {                                                     \
    Nan::LowMemoryNotification();                                    \
    Nan::ThrowError( "Could not allocate enough memory in ibm_db "   \
                     "file " __FILE__ ":" LINENO(__LINE__) ".");     \
    return;                                                          \
  }

typedef struct {
  unsigned char *name;
  unsigned char *type_name;
  unsigned int name_len;
  SQLSMALLINT max_display_len;
  SQLSMALLINT scale;
  SQLSMALLINT precision;
  SQLSMALLINT field_len;
  SQLLEN type;
  SQLUSMALLINT index;
} Column;

typedef struct {
  SQLSMALLINT  paramtype;
  SQLSMALLINT  c_type;
  SQLSMALLINT  type;
  SQLULEN      size;
  SQLSMALLINT  decimals;
  SQLPOINTER   buffer;
  SQLLEN       buffer_length;    
  SQLLEN       length;
  SQLUINTEGER  fileOption;    // For BindFileToParam
  SQLINTEGER   fileIndicator; // For BindFileToParam
} Parameter;

class ODBC : public Nan::ObjectWrap {
  public:
    static Nan::Persistent<Function> constructor;
    static uv_mutex_t g_odbcMutex;
    static uv_async_t g_async;
    
    static void Init(v8::Handle<Object> exports);
    static Column* GetColumns(SQLHSTMT hStmt, short* colCount);
    static void FreeColumns(Column* columns, short* colCount);
    static Handle<Value> GetColumnValue(SQLHSTMT hStmt, Column column, uint16_t* buffer, int bufferLength);
    static Handle<Value> GetOutputParameter(Parameter prm);
    static Local<Object> GetRecordTuple (SQLHSTMT hStmt, Column* columns, short* colCount, uint16_t* buffer, int bufferLength);
    static Local<Value> GetRecordArray (SQLHSTMT hStmt, Column* columns, short* colCount, uint16_t* buffer, int bufferLength);
    static Handle<Value> CallbackSQLError(SQLSMALLINT handleType, SQLHANDLE handle, Nan::Callback* cb);
    static Local<Value> CallbackSQLError (SQLSMALLINT handleType, SQLHANDLE handle, char* message, Nan::Callback* cb);
    static Local<Value> GetSQLError (SQLSMALLINT handleType, SQLHANDLE handle);
    static Local<Value> GetSQLError (SQLSMALLINT handleType, SQLHANDLE handle, char* message);
    static Local<Array>  GetAllRecordsSync (SQLHENV hENV, SQLHDBC hDBC, SQLHSTMT hSTMT, uint16_t* buffer, int bufferLength);
    static Parameter* GetParametersFromArray (Local<Array> values, int* paramCount);
    static SQLRETURN  BindParameters(SQLHSTMT hSTMT, Parameter params[], int count);
    
    void Free();
    
  protected:
    ODBC() {}

    ~ODBC();

    static void GetStringParam(Local<Value> value, Parameter * param, int num);
    static void GetNullParam(Parameter * param, int num);
    static void GetInt32Param(Local<Value> value, Parameter * param, int num);
    static void GetNumberParam(Local<Value> value, Parameter * param, int num);
    static void GetBoolParam(Local<Value> value, Parameter * param, int num);

    static NAN_METHOD(New);

    //async methods
    static NAN_METHOD(CreateConnection);
    static void UV_CreateConnection(uv_work_t* work_req);
    static void UV_AfterCreateConnection(uv_work_t* work_req, int status);
    
    static void WatcherCallback(uv_async_t* w, int revents);
    
    //sync methods
    static NAN_METHOD(CreateConnectionSync);
    
    ODBC *self(void) { return this; }

  protected:
    SQLHENV m_hEnv;
};

struct create_connection_work_data {
  Nan::Callback* cb;
  ODBC *dbo;
  SQLHDBC hDBC;
  int result;
};

struct open_request {
  Nan::Persistent<Function> cb;
  ODBC *dbo;
  int result;
  char connection[1];
};

struct close_request {
  Nan::Persistent<Function> cb;
  ODBC *dbo;
  int result;
};

struct query_request {
  Nan::Persistent<Function> cb;
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
  int  paramCount;
  int result;
};

#ifdef UNICODE
    #define SQL_T(x) (L##x)
#else
    #define SQL_T(x) (x)
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

#define REQ_ARGS(N)                                                     \
  if (info.Length() < (N))                                              \
    return Nan::ThrowTypeError("Expected " #N "arguments");

//Require String Argument; Save String as Utf8
#define REQ_STR_ARG(I, VAR)                                             \
  if (info.Length() <= (I) || !info[I]->IsString())                     \
    return Nan::ThrowTypeError("Argument " #I " must be a string");       \
  String::Utf8Value VAR(info[I]->ToString());

//Require String Argument; Save String as Wide String (UCS2)
#define REQ_WSTR_ARG(I, VAR)                                             \
  if (info.Length() <= (I) || !info[I]->IsString())                     \
    return Nan::ThrowTypeError("Argument " #I " must be a string");       \
  String::Value VAR(info[I]->ToString());

//Require String Argument; Save String as Object
#define REQ_STRO_ARG(I, VAR)                                             \
  if (info.Length() <= (I) || !info[I]->IsString())                     \
    return Nan::ThrowTypeError("Argument " #I " must be a string");       \
  Local<String> VAR(info[I]->ToString());

//Require String or Null Argument; Save String as Utf8
#define REQ_STR_OR_NULL_ARG(I, VAR)                                             \
  if ( info.Length() <= (I) || (!info[I]->IsString() && !info[I]->IsNull()) )   \
    return Nan::ThrowTypeError("Argument " #I " must be a string or null");       \
  String::Utf8Value VAR(info[I]->ToString());

//Require String or Null Argument; Save String as Wide String (UCS2)
#define REQ_WSTR_OR_NULL_ARG(I, VAR)                                              \
  if ( info.Length() <= (I) || (!info[I]->IsString() && !info[I]->IsNull()) )     \
    return Nan::ThrowTypeError("Argument " #I " must be a string or null");         \
  String::Value VAR(info[I]->ToString());

//Require String or Null Argument; save String as String Object
#define REQ_STRO_OR_NULL_ARG(I, VAR)                                              \
  if ( info.Length() <= (I) || (!info[I]->IsString() && !info[I]->IsNull()) ) {   \
    Nan::ThrowTypeError("Argument " #I " must be a string or null");                \
    return;                                                         \
  }                                                                               \
  Local<String> VAR(info[I]->ToString());

#define REQ_FUN_ARG(I, VAR)                                             \
  if (info.Length() <= (I) || !info[I]->IsFunction())                   \
    return Nan::ThrowTypeError("Argument " #I " must be a function");     \
  Local<Function> VAR = Local<Function>::Cast(info[I]);

#define REQ_BOOL_ARG(I, VAR)                                            \
  if (info.Length() <= (I) || !info[I]->IsBoolean())                    \
    return Nan::ThrowTypeError("Argument " #I " must be a boolean");      \
  Local<Boolean> VAR = (info[I]->ToBoolean());
  
#define REQ_EXT_ARG(I, VAR)                                             \
  if (info.Length() <= (I) || !info[I]->IsExternal())                   \
    return Nan::ThrowTypeError("Argument " #I " invalid");                \
  Local<External> VAR = Local<External>::Cast(info[I]);

#define REQ_INT_ARG(I, VAR)                                             \
  if (info.Length() <= (I) || !info[I]->IsInt32())                      \
    return Nan::ThrowTypeError("Argument " #I " invalid");              \
  SQLUSMALLINT VAR = (info[I]->Int32Value());

#define OPT_INT_ARG(I, VAR, DEFAULT)                                    \
  SQLUSMALLINT VAR;                                                     \
  if (info.Length() <= (I)) {                                           \
    VAR = (DEFAULT);                                                    \
  } else if (info[I]->IsInt32()) {                                      \
    VAR = info[I]->Int32Value();                                        \
  } else {                                                              \
    return Nan::ThrowTypeError("Argument " #I " must be an integer");     \
  }


// From node v10 NODE_DEFINE_CONSTANT
#define NODE_ODBC_DEFINE_CONSTANT(constructor_template, constant)       \
  (constructor_template)->Set(Nan::New<String>(#constant).ToLocalChecked(),\
                Nan::New<Number>(constant),                               \
                static_cast<v8::PropertyAttribute>(v8::ReadOnly|v8::DontDelete))

#endif

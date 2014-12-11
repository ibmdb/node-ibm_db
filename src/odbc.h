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
#include <wchar.h>

#include <stdlib.h>
#ifdef dynodbc
#include "dynodbc.h"
#else
#include <sqlcli1.h>
#endif

using namespace v8;
using namespace node;

#define MAX_FIELD_SIZE 1024
#define MAX_VALUE_SIZE 1048576

#define MODE_COLLECT_AND_CALLBACK 1
#define MODE_CALLBACK_FOR_EACH 2
#define FETCH_ARRAY 3
#define FETCH_OBJECT 4
#define SQL_DESTROY 9999


typedef struct {
  unsigned char *name;
  unsigned char *type_name;
  unsigned int len;
  SQLLEN type;
  SQLUSMALLINT index;
} Column;

typedef struct {
  SQLSMALLINT  c_type;
  SQLSMALLINT  type;
  SQLLEN       size;
  void        *buffer;
  SQLLEN       buffer_length;    
  SQLLEN       length;
  SQLSMALLINT  decimals;
} Parameter;

class ODBC : public node::ObjectWrap {
  public:
    static Persistent<FunctionTemplate> constructor_template;
    static uv_mutex_t g_odbcMutex;
    static uv_async_t g_async;
    
    static void Init(v8::Handle<Object> target);
    static Column* GetColumns(SQLHSTMT hStmt, short* colCount);
    static void FreeColumns(Column* columns, short* colCount);
    static Handle<Value> GetColumnValue(SQLHSTMT hStmt, Column column, uint16_t* buffer, int bufferLength);
    static Local<Object> GetRecordTuple (SQLHSTMT hStmt, Column* columns, short* colCount, uint16_t* buffer, int bufferLength);
    static Handle<Value> GetRecordArray (SQLHSTMT hStmt, Column* columns, short* colCount, uint16_t* buffer, int bufferLength);
    static Handle<Value> CallbackSQLError (SQLSMALLINT handleType, SQLHANDLE handle, Persistent<Function> cb);
    static Handle<Value> CallbackSQLError (SQLSMALLINT handleType, SQLHANDLE handle, char* message, Persistent<Function> cb);
    static Local<Object> GetSQLError (SQLSMALLINT handleType, SQLHANDLE handle);
    static Local<Object> GetSQLError (SQLSMALLINT handleType, SQLHANDLE handle, char* message);
    static Local<Array>  GetAllRecordsSync (HENV hENV, HDBC hDBC, HSTMT hSTMT, uint16_t* buffer, int bufferLength);
#ifdef dynodbc
    static Handle<Value> LoadODBCLibrary(const Arguments& args);
#endif
    static Parameter* GetParametersFromArray (Local<Array> values, int* paramCount);
    
    void Free();
    
  protected:
    ODBC() {}

    ~ODBC();

    static Handle<Value> New(const Arguments& args);

    //async methods
    static Handle<Value> CreateConnection(const Arguments& args);
    static void UV_CreateConnection(uv_work_t* work_req);
    static void UV_AfterCreateConnection(uv_work_t* work_req, int status);
    
    static void WatcherCallback(uv_async_t* w, int revents);
    
    //sync methods
    static Handle<Value> CreateConnectionSync(const Arguments& args);
    
    ODBC *self(void) { return this; }

  protected:
    HENV m_hEnv;
};

struct create_connection_work_data {
  Persistent<Function> cb;
  ODBC *dbo;
  HDBC hDBC;
  int result;
};

struct open_request {
  Persistent<Function> cb;
  ODBC *dbo;
  int result;
  char connection[1];
};

struct close_request {
  Persistent<Function> cb;
  ODBC *dbo;
  int result;
};

struct query_request {
  Persistent<Function> cb;
  ODBC *dbo;
  HSTMT hSTMT;
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
  if (args.Length() < (N))                                              \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Expected " #N "arguments")));

//Require String Argument; Save String as Utf8
#define REQ_STR_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsString())                     \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a string"))); \
  String::Utf8Value VAR(args[I]->ToString());

//Require String Argument; Save String as Wide String (UCS2)
#define REQ_WSTR_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsString())                     \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a string"))); \
  String::Value VAR(args[I]->ToString());

//Require String Argument; Save String as Object
#define REQ_STRO_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsString())                     \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a string"))); \
  Local<String> VAR(args[I]->ToString());

//Require String or Null Argument; Save String as Utf8
#define REQ_STR_OR_NULL_ARG(I, VAR)                                             \
  if ( args.Length() <= (I) || (!args[I]->IsString() && !args[I]->IsNull()) )                     \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a string or null"))); \
  String::Utf8Value VAR(args[I]->ToString());

//Require String or Null Argument; Save String as Wide String (UCS2)
#define REQ_WSTR_OR_NULL_ARG(I, VAR)                                             \
  if ( args.Length() <= (I) || (!args[I]->IsString() && !args[I]->IsNull()) )                     \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a string or null"))); \
  String::Value VAR(args[I]->ToString());

//Require String or Null Argument; save String as String Object
#define REQ_STRO_OR_NULL_ARG(I, VAR)                                             \
  if ( args.Length() <= (I) || (!args[I]->IsString() && !args[I]->IsNull()) )                     \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a string or null"))); \
  Local<String> VAR(args[I]->ToString());

#define REQ_FUN_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsFunction())                   \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a function"))); \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_BOOL_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsBoolean())                   \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a boolean"))); \
  Local<Boolean> VAR = (args[I]->ToBoolean());
  
#define REQ_EXT_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsExternal())                   \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " invalid"))); \
  Local<External> VAR = Local<External>::Cast(args[I]);

#define OPT_INT_ARG(I, VAR, DEFAULT)                                    \
  int VAR;                                                              \
  if (args.Length() <= (I)) {                                           \
    VAR = (DEFAULT);                                                    \
  } else if (args[I]->IsInt32()) {                                      \
    VAR = args[I]->Int32Value();                                        \
  } else {                                                              \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be an integer"))); \
  }


#endif

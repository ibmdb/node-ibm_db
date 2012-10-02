/*
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

#ifndef DATABASE_H
#define DATABASE_H

#include <v8.h>
#include <node.h>

#include <stdlib.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

using namespace v8;
using namespace node;

class Database : public node::ObjectWrap {
 public:
  static Persistent<FunctionTemplate> constructor_template;
  static void Init(v8::Handle<Object> target);
  static pthread_mutex_t g_odbcMutex;
  static uv_async_t g_async;

 protected:
 Database() { }

  ~Database() {
  }

  static Handle<Value> New(const Arguments& args);

  static void UV_AfterOpen(uv_work_t* req);
  static void UV_Open(uv_work_t* req);
  static Handle<Value> Open(const Arguments& args);

  static void UV_AfterClose(uv_work_t* req);
  static void UV_Close(uv_work_t* req);
  static Handle<Value> Close(const Arguments& args);

  static void UV_AfterQuery(uv_work_t* req);
  static void UV_Query(uv_work_t* req);
  static Handle<Value> Query(const Arguments& args);

  static void UV_Tables(uv_work_t* req);
  static Handle<Value> Tables(const Arguments& args);

  static void UV_Columns(uv_work_t* req);
  static Handle<Value> Columns(const Arguments& args);
  
  static void WatcherCallback(uv_async_t *w, int revents);
  
  Database *self(void) { return this; }
  void printError(const char *fn, SQLHANDLE handle, SQLSMALLINT type);

 protected:
  HENV m_hEnv;
  HDBC m_hDBC;
  HSTMT m_hStmt;
  SQLUSMALLINT canHaveMoreResults;
};

enum ExecMode
  {
    EXEC_EMPTY = 0,
    EXEC_LAST_INSERT_ID = 1,
    EXEC_AFFECTED_ROWS = 2
  };

struct open_request {
  Persistent<Function> cb;
  Database *dbo;
  int result;
  char connection[1];
};

struct close_request {
  Persistent<Function> cb;
  Database *dbo;
  int result;
};

typedef struct {
  unsigned char *name;
  unsigned int len;
  SQLLEN type;
} Column;

typedef struct {
    SQLSMALLINT  c_type;
    SQLSMALLINT  type;
    SQLLEN       size;
    void        *buffer;
    SQLLEN       buffer_length;    
    SQLLEN       length;
} Parameter;

struct query_request {
  Persistent<Function> cb;
  Database *dbo;
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

#define REQ_ARGS(N)                                                     \
  if (args.Length() < (N))                                              \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Expected " #N "arguments")));

#define REQ_STR_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsString())                     \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a string"))); \
  String::Utf8Value VAR(args[I]->ToString());

#define REQ_STR_OR_NULL_ARG(I, VAR)                                             \
  if ( args.Length() <= (I) || (!args[I]->IsString() && !args[I]->IsNull()) )                     \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a string or null"))); \
  String::Utf8Value VAR(args[I]->ToString());

#define REQ_FUN_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsFunction())                   \
    return ThrowException(Exception::TypeError(                         \
                                               String::New("Argument " #I " must be a function"))); \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

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

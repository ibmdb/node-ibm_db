/*
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
#include <node_events.h>

#include "Database.h"

#define MAX_FIELD_SIZE 1024

using namespace v8;
using namespace node;

typedef struct {
  unsigned char *name;
  unsigned int len;
} ColumnLabel;


void Database::Init(v8::Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);

  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->Inherit(EventEmitter::constructor_template);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("Database"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "open", Open);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "close", Close);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "dispatchQuery", Query);

  target->Set(v8::String::NewSymbol("Database"),
              constructor_template->GetFunction());
}

Handle<Value> Database::New(const Arguments& args) {
  HandleScope scope;
  Database* dbo = new Database();
  dbo->Wrap(args.This());
  return args.This();
}

int Database::EIO_AfterOpen(eio_req *req) {
  ev_unref(EV_DEFAULT_UC);
  HandleScope scope;
  struct open_request *open_req = (struct open_request *)(req->data);

  Local<Value> argv[1];
  bool err = false;
  if (req->result) {
    err = true;
    argv[0] = Exception::Error(String::New("Error opening database"));
  }

  TryCatch try_catch;

  open_req->dbo->Unref();
  open_req->cb->Call(Context::GetCurrent()->Global(), err ? 1 : 0, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  open_req->dbo->Emit(String::New("ready"), 0, NULL);
  open_req->cb.Dispose();

  free(open_req);
  scope.Close(Undefined());
  return 0;
}

int Database::EIO_Open(eio_req *req) {
  struct open_request *open_req = (struct open_request *)(req->data);
  Database *self = open_req->dbo->self();

  int ret = SQLAllocEnv( &self->m_hEnv );
  if( ret == SQL_SUCCESS ) {
    ret = SQLAllocConnect( self->m_hEnv,&self->m_hDBC );
    if( ret == SQL_SUCCESS ) {
      SQLSetConnectOption( self->m_hDBC,SQL_LOGIN_TIMEOUT,5 );
      char connstr[1024];
      ret = SQLDriverConnect(self->m_hDBC,NULL,(SQLCHAR*)open_req->connection,strlen(open_req->connection),(SQLCHAR*)connstr,1024,NULL,SQL_DRIVER_NOPROMPT);

      if( ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO )
        {
          ret = SQLAllocStmt( self->m_hDBC,&self->m_hStmt );
          if (ret != SQL_SUCCESS) printf("not connected\n");
        }
      else
        {
          self->printError("SQLDriverConnect", self->m_hDBC, SQL_HANDLE_DBC);
        }
    }
  }
  req->result = ret;
  return 0;
}

Handle<Value> Database::Open(const Arguments& args) {
  HandleScope scope;

  REQ_STR_ARG(0, connection);
  REQ_FUN_ARG(1, cb);

  Database* dbo = ObjectWrap::Unwrap<Database>(args.This());

  struct open_request *open_req = (struct open_request *)
    calloc(1, sizeof(struct open_request) + connection.length());

  if (!open_req) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(
                                           String::New("Could not allocate enough memory")));
  }

  strcpy(open_req->connection, *connection);
  open_req->cb = Persistent<Function>::New(cb);
  open_req->dbo = dbo;

  eio_custom(EIO_Open, EIO_PRI_DEFAULT, EIO_AfterOpen, open_req);

  ev_ref(EV_DEFAULT_UC);
  dbo->Ref();
  scope.Close(Undefined());
  return Undefined();
}

int Database::EIO_AfterClose(eio_req *req) {
  ev_unref(EV_DEFAULT_UC);

  HandleScope scope;

  struct close_request *close_req = (struct close_request *)(req->data);

  Local<Value> argv[1];
  bool err = false;
  if (req->result) {
    err = true;
    argv[0] = Exception::Error(String::New("Error closing database"));
  }

  TryCatch try_catch;

  close_req->dbo->Unref();
  close_req->cb->Call(Context::GetCurrent()->Global(), err ? 1 : 0, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  close_req->cb.Dispose();

  free(close_req);
  scope.Close(Undefined());
  return 0;
}

int Database::EIO_Close(eio_req *req) {
  struct close_request *close_req = (struct close_request *)(req->data);
  Database* dbo = close_req->dbo;
  SQLDisconnect(dbo->m_hDBC);
  SQLFreeHandle(SQL_HANDLE_ENV, dbo->m_hEnv);
  SQLFreeHandle(SQL_HANDLE_DBC, dbo->m_hDBC);
  return 0;
}

Handle<Value> Database::Close(const Arguments& args) {
  HandleScope scope;

  REQ_FUN_ARG(0, cb);

  Database* dbo = ObjectWrap::Unwrap<Database>(args.This());

  struct close_request *close_req = (struct close_request *)
    calloc(1, sizeof(struct close_request));

  if (!close_req) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(
                                           String::New("Could not allocate enough memory")));
  }

  close_req->cb = Persistent<Function>::New(cb);
  close_req->dbo = dbo;

  eio_custom(EIO_Close, EIO_PRI_DEFAULT, EIO_AfterClose, close_req);

  ev_ref(EV_DEFAULT_UC);
  dbo->Ref();
  scope.Close(Undefined());
  return Undefined();
}

int Database::EIO_AfterQuery(eio_req *req) {
  ev_unref(EV_DEFAULT_UC);
  struct query_request *prep_req = (struct query_request *)(req->data);
  HandleScope scope;

  // get column data
  short colcount;
  SQLSMALLINT buflen;
  SQLRETURN ret;
  SQLNumResultCols(prep_req->dbo->m_hStmt, &colcount);

  ColumnLabel *labels = new ColumnLabel[colcount];

  // retrieve and store column names to build the row object
  for(int i = 0; i < colcount; i++)
    {
      labels[i].name = new unsigned char[MAX_FIELD_SIZE];
      memset(labels[i].name, 0, MAX_FIELD_SIZE);
      ret = SQLColAttribute(prep_req->dbo->m_hStmt, (SQLUSMALLINT)i+1, SQL_DESC_LABEL, labels[i].name, (SQLSMALLINT)MAX_FIELD_SIZE, (SQLSMALLINT *)&buflen, NULL);
      labels[i].len = buflen;
    }

  // i dont think odbc will tell how many rows are returned, loop until out...
  Local<Array> rows = Array::New();
  int count = 0;
  while(true)
    {
      Local<Object> tuple = Object::New();
      ret = SQLFetch(prep_req->dbo->m_hStmt);
      if(ret) break; // error :|

      for(int i = 0; i < colcount; i++)
        {
          int len;

	  // SQLGetData can supposedly return multiple chunks, need to do this to retrieve large fields

          char buf[MAX_FIELD_SIZE];
          memset(buf,0,MAX_FIELD_SIZE);
          int ret = SQLGetData(prep_req->dbo->m_hStmt, i+1, SQL_CHAR, (char *) buf, MAX_FIELD_SIZE-1, (SQLINTEGER *) &len);

          if(ret == SQL_NULL_DATA || len < 0)
            {
              tuple->Set(String::New((const char *)labels[i].name), Null());
            }
          else
            {
              tuple->Set(String::New((const char *)labels[i].name), String::New(buf));
            }
        }
      rows->Set(Integer::New(count), tuple);
      count++;
    }

  for(int i = 0; i < colcount; i++)
    {
      delete [] labels[i].name;
    }
  delete [] labels;

  TryCatch try_catch;

  prep_req->dbo->Unref();

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }

  SQLFreeStmt(prep_req->dbo->m_hStmt,NULL);
  SQLAllocStmt(prep_req->dbo->m_hDBC,&prep_req->dbo->m_hStmt );


  Local<Value> args[2];
  args[0] = Local<Value>::New(Null());
  args[1] = rows;

  prep_req->dbo->Emit(String::New("result"), 2, args);

  free(prep_req);
  scope.Close(Undefined());
  return 0;
}

int Database::EIO_Query(eio_req *req) {
  struct query_request *prep_req = (struct query_request *)(req->data);


  if(prep_req->dbo->m_hStmt)
    {
      SQLFreeStmt(prep_req->dbo->m_hStmt,NULL);
      SQLAllocStmt(prep_req->dbo->m_hDBC,&prep_req->dbo->m_hStmt );
    }

  SQLRETURN ret = SQLExecDirect( prep_req->dbo->m_hStmt,(SQLCHAR *)prep_req->sql, strlen(prep_req->sql) );
  if(ret != 0)
    {
      char buf[512];
      char sqlstate[128];
      SQLError(prep_req->dbo->m_hEnv, prep_req->dbo->m_hDBC, prep_req->dbo->m_hStmt,(SQLCHAR *)sqlstate,NULL,(SQLCHAR *)buf, sizeof(buf), NULL);
    }

  req->result = ret;

  return 0;
}

Handle<Value> Database::Query(const Arguments& args) {
  HandleScope scope;

  REQ_STR_ARG(0, sql);
  //REQ_FUN_ARG(1, cb);

  Database* dbo = ObjectWrap::Unwrap<Database>(args.This());

  struct query_request *prep_req = (struct query_request *)
    calloc(1, sizeof(struct query_request) + sql.length());

  if (!prep_req) {
    V8::LowMemoryNotification();
    return ThrowException(Exception::Error(
                                           String::New("Could not allocate enough memory")));
  }

  strcpy(prep_req->sql, *sql);

  prep_req->dbo = dbo;

  eio_custom(EIO_Query, EIO_PRI_DEFAULT, EIO_AfterQuery, prep_req);

  ev_ref(EV_DEFAULT_UC);
  dbo->Ref();
  scope.Close(Undefined());
  return Undefined();
}

void Database::printError(const char *fn, SQLHANDLE handle, SQLSMALLINT type)
{
  SQLINTEGER i = 0;
  SQLINTEGER native;
  SQLCHAR state[ 7 ];
  SQLCHAR text[256];
  SQLSMALLINT len;
  SQLRETURN ret;

  fprintf(stderr,
          "\n"
          "The driver reported the following diagnostics whilst running "
          "%s\n\n",
          fn);

  do
    {
      ret = SQLGetDiagRec(type, handle, ++i, state, &native, text,
                          sizeof(text), &len );
      if (SQL_SUCCEEDED(ret))
        printf("%s:%ld:%ld:%s\n", state, i, native, text);
    }
  while( ret == SQL_SUCCESS );
}


Persistent<FunctionTemplate> Database::constructor_template;

extern "C" void init (v8::Handle<Object> target) {
  Database::Init(target);
}

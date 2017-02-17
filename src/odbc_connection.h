/*
  Copyright (c) 2013, Dan VerWeire<dverweire@gmail.com>
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

#ifndef _SRC_ODBC_CONNECTION_H
#define _SRC_ODBC_CONNECTION_H

#include <nan.h>

#define DEFAULT_CONNECTION_TIMEOUT 30

class ODBCConnection : public Nan::ObjectWrap {
  public:
   static Nan::Persistent<String> OPTION_SQL;
   static Nan::Persistent<String> OPTION_PARAMS;
   static Nan::Persistent<String> OPTION_NORESULTS;
   static Nan::Persistent<Function> constructor;
   
   static void Init(v8::Handle<Object> exports);
   
   void Free();
   
  protected:
    ODBCConnection() {};
    
    explicit ODBCConnection(SQLHENV hENV, SQLHDBC hDBC): 
      Nan::ObjectWrap(),
      m_hENV(hENV),
      m_hDBC(hDBC) {};
     
    ~ODBCConnection();

    //constructor
    static NAN_METHOD(New);

    //Property Getter/Setters
    static NAN_GETTER(ConnectedGetter);
    static NAN_GETTER(ConnectTimeoutGetter);
    static NAN_SETTER(ConnectTimeoutSetter);
    static NAN_GETTER(SystemNamingGetter);
    static NAN_SETTER(SystemNamingSetter);

    //async methods
    static NAN_METHOD(BeginTransaction);
    static void UV_BeginTransaction(uv_work_t* work_req);
    static void UV_AfterBeginTransaction(uv_work_t* work_req, int status);
    
    static NAN_METHOD(EndTransaction);
    static void UV_EndTransaction(uv_work_t* work_req);
    static void UV_AfterEndTransaction(uv_work_t* work_req, int status);
    
    static NAN_METHOD(Open);
    static void UV_Open(uv_work_t* work_req);
    static void UV_AfterOpen(uv_work_t* work_req, int status);
    static void SetConnectionAttributes( ODBCConnection* conn );

    static NAN_METHOD(Close);
    static void UV_Close(uv_work_t* work_req);
    static void UV_AfterClose(uv_work_t* work_req, int status);

    static NAN_METHOD(CreateStatement);
    static void UV_CreateStatement(uv_work_t* work_req);
    static void UV_AfterCreateStatement(uv_work_t* work_req, int status);

    static NAN_METHOD(Query);
    static void UV_Query(uv_work_t* req);
    static void UV_AfterQuery(uv_work_t* req, int status);

    static NAN_METHOD(Columns);
    static void UV_Columns(uv_work_t* req);
    
    static NAN_METHOD(Tables);
    static void UV_Tables(uv_work_t* req);
    
    //sync methods
    static NAN_METHOD(CloseSync);
    static NAN_METHOD(CreateStatementSync);
    static NAN_METHOD(OpenSync);
    static NAN_METHOD(QuerySync);
    static NAN_METHOD(CallSync);
    static NAN_METHOD(BeginTransactionSync);
    static NAN_METHOD(EndTransactionSync);
    static NAN_METHOD(SetIsolationLevel);
    
    struct Fetch_Request {
      Nan::Callback* callback;
      ODBCConnection *objResult;
      SQLRETURN result;
    };
    
    ODBCConnection *self(void) { return this; }

  protected:
    SQLHENV m_hENV;
    SQLHDBC m_hDBC;
    SQLUSMALLINT canHaveMoreResults;
    bool systemNaming;  // For i5/OS SQL_ATTR_DBC_SYS_NAMING
    bool connected;
    int statements;
    SQLUINTEGER connectTimeout; // For SQL_ATTR_LOGIN_TIMEOUT
};

struct create_statement_work_data {
  Nan::Callback* cb;
  ODBCConnection *conn;
  SQLHSTMT hSTMT;
  int result;
};

struct query_work_data {
  Nan::Callback* cb;
  ODBCConnection *conn;
  SQLHSTMT hSTMT;
  
  Parameter *params;
  int paramCount;
  int completionType;
  bool noResultObject;
  
  void *sql;
  void *catalog;
  void *schema;
  void *table;
  void *type;
  void *column;
  
  int sqlLen;
  int sqlSize;
  
  int result;
};

struct open_connection_work_data {
  Nan::Callback* cb;
  ODBCConnection *conn;
  int result;
  int connectionLength;
  void* connection;
};

struct close_connection_work_data {
  Nan::Callback* cb;
  ODBCConnection *conn;
  int result;
};

#endif

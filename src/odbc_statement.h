/*
  Copyright (c) 2013, Dan VerWeire<dverweire@gmail.com>

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

#ifndef _SRC_ODBC_STATEMENT_H
#define _SRC_ODBC_STATEMENT_H

#include <nan.h>

class ODBCStatement : public Nan::ObjectWrap {
  public:
   static Nan::Persistent<Function> constructor;
   static void Init(v8::Handle<Object> exports);
   
   void Free();
   
  protected:
    ODBCStatement() {};
    
    explicit ODBCStatement(SQLHENV hENV, SQLHDBC hDBC, SQLHSTMT hSTMT): 
      Nan::ObjectWrap(),
      m_hENV(hENV),
      m_hDBC(hDBC),
      m_hSTMT(hSTMT) {};
     
    ~ODBCStatement();

    //constructor
    static NAN_METHOD(New);

    //async methods
    static NAN_METHOD(Execute);
    static void UV_Execute(uv_work_t* work_req);
    static void UV_AfterExecute(uv_work_t* work_req, int status);

    static NAN_METHOD(ExecuteDirect);
    static void UV_ExecuteDirect(uv_work_t* work_req);
    static void UV_AfterExecuteDirect(uv_work_t* work_req, int status);

    static NAN_METHOD(ExecuteNonQuery);
    static void UV_ExecuteNonQuery(uv_work_t* work_req);
    static void UV_AfterExecuteNonQuery(uv_work_t* work_req, int status);
    
    static NAN_METHOD(Prepare);
    static void UV_Prepare(uv_work_t* work_req);
    static void UV_AfterPrepare(uv_work_t* work_req, int status);
    
    static NAN_METHOD(Bind);
    static void UV_Bind(uv_work_t* work_req);
    static void UV_AfterBind(uv_work_t* work_req, int status);
    
    //sync methods
    static NAN_METHOD(CloseSync);
    static NAN_METHOD(ExecuteSync);
    static NAN_METHOD(ExecuteDirectSync);
    static NAN_METHOD(ExecuteNonQuerySync);
    static NAN_METHOD(PrepareSync);
    static NAN_METHOD(BindSync);
    
    struct Fetch_Request {
      Nan::Callback* callback;
      ODBCStatement *objResult;
      SQLRETURN result;
    };
    
    ODBCStatement *self(void) { return this; }

  protected:
    SQLHENV m_hENV;
    SQLHDBC m_hDBC;
    SQLHSTMT m_hSTMT;
    
    Parameter *params;
    int paramCount;
    
    uint16_t *buffer;
    int bufferLength;
    Column *columns;
    short colCount;
};

struct execute_direct_work_data {
  Nan::Callback* cb;
  ODBCStatement *stmt;
  int result;
  void *sql;
  int sqlLen;
};

struct execute_work_data {
  Nan::Callback* cb;
  ODBCStatement *stmt;
  int result;
};

struct prepare_work_data {
  Nan::Callback* cb;
  ODBCStatement *stmt;
  int result;
  void *sql;
  int sqlLen;
};

struct bind_work_data {
  Nan::Callback* cb;
  ODBCStatement *stmt;
  int result;
};

#endif

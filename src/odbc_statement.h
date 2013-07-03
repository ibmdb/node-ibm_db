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

class ODBCStatement : public node::ObjectWrap {
  public:
   static Persistent<FunctionTemplate> constructor_template;
   static void Init(v8::Handle<Object> target);
   
   void Free();
   
  protected:
    ODBCStatement() {};
    
    explicit ODBCStatement(HENV hENV, HDBC hDBC, HSTMT hSTMT): 
      ObjectWrap(),
      m_hENV(hENV),
      m_hDBC(hDBC),
      m_hSTMT(hSTMT) {};
     
    ~ODBCStatement();

    //constructor
    static Handle<Value> New(const Arguments& args);

    //async methods
    static Handle<Value> Execute(const Arguments& args);
    static void UV_Execute(uv_work_t* work_req);
    static void UV_AfterExecute(uv_work_t* work_req, int status);

    static Handle<Value> ExecuteDirect(const Arguments& args);
    static void UV_ExecuteDirect(uv_work_t* work_req);
    static void UV_AfterExecuteDirect(uv_work_t* work_req, int status);

    static Handle<Value> ExecuteNonQuery(const Arguments& args);
    static void UV_ExecuteNonQuery(uv_work_t* work_req);
    static void UV_AfterExecuteNonQuery(uv_work_t* work_req, int status);
    
    static Handle<Value> Prepare(const Arguments& args);
    static void UV_Prepare(uv_work_t* work_req);
    static void UV_AfterPrepare(uv_work_t* work_req, int status);
    
    static Handle<Value> Bind(const Arguments& args);
    static void UV_Bind(uv_work_t* work_req);
    static void UV_AfterBind(uv_work_t* work_req, int status);
    
    //sync methods
    static Handle<Value> CloseSync(const Arguments& args);
    static Handle<Value> ExecuteSync(const Arguments& args);
    static Handle<Value> ExecuteDirectSync(const Arguments& args);
    static Handle<Value> ExecuteNonQuerySync(const Arguments& args);
    static Handle<Value> PrepareSync(const Arguments& args);
    static Handle<Value> BindSync(const Arguments& args);
    
    struct Fetch_Request {
      Persistent<Function> callback;
      ODBCStatement *objResult;
      SQLRETURN result;
    };
    
    ODBCStatement *self(void) { return this; }

  protected:
    HENV m_hENV;
    HDBC m_hDBC;
    HSTMT m_hSTMT;
    
    Parameter *params;
    int paramCount;
    
    uint16_t *buffer;
    int bufferLength;
    Column *columns;
    short colCount;
};

struct execute_direct_work_data {
  Persistent<Function> cb;
  ODBCStatement *stmt;
  int result;
  void *sql;
  int sqlLen;
};

struct execute_work_data {
  Persistent<Function> cb;
  ODBCStatement *stmt;
  int result;
};

struct prepare_work_data {
  Persistent<Function> cb;
  ODBCStatement *stmt;
  int result;
  void *sql;
  int sqlLen;
};

struct bind_work_data {
  Persistent<Function> cb;
  ODBCStatement *stmt;
  int result;
};

#endif

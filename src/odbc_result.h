/*
  Copyright (c) 2012, Dan VerWeire<dverweire@gmail.com>
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

#ifndef _SRC_ODBC_RESULT_H
#define _SRC_ODBC_RESULT_H

class ODBCResult : public node::ObjectWrap {
  public:
   static Persistent<FunctionTemplate> constructor_template;
   static void Init(v8::Handle<Object> target);
   
   void Free();
   
  protected:
    ODBCResult() {};
    
    explicit ODBCResult(HENV hENV, HDBC hDBC, HSTMT hSTMT): 
      ObjectWrap(),
      m_hENV(hENV),
      m_hDBC(hDBC),
      m_hSTMT(hSTMT) {};
     
    ~ODBCResult();

    //constructor
    static Handle<Value> New(const Arguments& args);

    //async methods
    static Handle<Value> Fetch(const Arguments& args);
    static void UV_Fetch(uv_work_t* work_req);
    static void UV_AfterFetch(uv_work_t* work_req);

    static Handle<Value> FetchAll(const Arguments& args);
    static void UV_FetchAll(uv_work_t* work_req);
    static void UV_AfterFetchAll(uv_work_t* work_req);
    
    //sync methods
    static Handle<Value> Close(const Arguments& args);
    static Handle<Value> MoreResults(const Arguments& args);
    
    struct Fetch_Request {
      Persistent<Function> callback;
      ODBCResult *objResult;
      SQLRETURN result;
    };
    
    ODBCResult *self(void) { return this; }

  protected:
    HENV m_hENV;
    HDBC m_hDBC;
    HSTMT m_hSTMT;
    uint16_t *buffer;
    int bufferLength;
    Column *columns;
    short colCount;
};

#endif

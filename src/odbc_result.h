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

#ifndef _SRC_ODBC_RESULT_H
#define _SRC_ODBC_RESULT_H

class ODBCResult : public node::ObjectWrap {
  public:
   static Persistent<String> OPTION_FETCH_MODE;
   static Persistent<FunctionTemplate> constructor_template;
   static void Init(v8::Handle<Object> target);
   
   void Free();
   
  protected:
    ODBCResult() {};
    
    explicit ODBCResult(HENV hENV, HDBC hDBC, HSTMT hSTMT, bool canFreeHandle): 
      ObjectWrap(),
      m_hENV(hENV),
      m_hDBC(hDBC),
      m_hSTMT(hSTMT),
      m_canFreeHandle(canFreeHandle) {};
     
    ~ODBCResult();

    //constructor
    static Handle<Value> New(const Arguments& args);

    //async methods
    static Handle<Value> Fetch(const Arguments& args);
    static void UV_Fetch(uv_work_t* work_req);
    static void UV_AfterFetch(uv_work_t* work_req, int status);

    static Handle<Value> FetchAll(const Arguments& args);
    static void UV_FetchAll(uv_work_t* work_req);
    static void UV_AfterFetchAll(uv_work_t* work_req, int status);
    
    //sync methods
    static Handle<Value> CloseSync(const Arguments& args);
    static Handle<Value> MoreResultsSync(const Arguments& args);
    static Handle<Value> FetchSync(const Arguments& args);
    static Handle<Value> FetchAllSync(const Arguments& args);
    static Handle<Value> GetColumnNamesSync(const Arguments& args);
    
    //property getter/setters
    static Handle<Value> FetchModeGetter(Local<String> property, const AccessorInfo &info);
    static void FetchModeSetter(Local<String> property, Local<Value> value, const AccessorInfo &info);
    
    struct fetch_work_data {
      Persistent<Function> cb;
      ODBCResult *objResult;
      SQLRETURN result;
      
      int fetchMode;
      int count;
      int errorCount;
      Persistent<Array> rows;
      Persistent<Object> objError;
    };
    
    ODBCResult *self(void) { return this; }

  protected:
    HENV m_hENV;
    HDBC m_hDBC;
    HSTMT m_hSTMT;
    bool m_canFreeHandle;
    int m_fetchMode;
    
    uint16_t *buffer;
    int bufferLength;
    Column *columns;
    short colCount;
};



#endif

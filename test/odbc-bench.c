/*
  Copyright (c) 2012, Dan VerWeire<dverweire@gmail.com>
  Copyright (c) 2011, Lee Smith<notwink@gmail.com>
  
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
#include <time.h>
#include <stdlib.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <sys/timeb.h>

#define MAX_FIELD_SIZE 1024
#define MAX_VALUE_SIZE 1048576



int main() {
  HENV m_hEnv;
  HDBC m_hDBC;
  HSTMT m_hStmt;
  SQLRETURN ret;
  SQLUSMALLINT canHaveMoreResults;
  //SQLCHAR outstr[1024];
  //SQLSMALLINT outstrlen;
  
  if( SQL_SUCCEEDED(SQLAllocEnv( &m_hEnv )) ) {
    
    if( SQL_SUCCEEDED(SQLAllocHandle( SQL_HANDLE_DBC, m_hEnv, &m_hDBC )) ) {
      SQLSetConnectOption( m_hDBC, SQL_LOGIN_TIMEOUT,5 );

      ret = SQLDriverConnect(
          m_hDBC,
          NULL,
          "DRIVER={MySQL};SERVER=localhost;USER=test;PASSWORD=;DATABASE=test;",
          SQL_NTS,
          NULL,//outstr,
          0,//sizeof(outstr),
          NULL,//&outstrlen,
          SQL_DRIVER_NOPROMPT
      );

      if( SQL_SUCCEEDED(ret) ) {
        int iterations = 10000;
        int i = 0;
        struct timeb start;
        
        ftime(&start);
        
        for (i =0 ; i <= iterations; i ++) {
          SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, &m_hStmt);
          
          SQLExecDirect(m_hStmt, "select 1 + 1 as test;", SQL_NTS);
          
          while ( SQL_SUCCEEDED(SQLFetch(m_hStmt) )) {
            //printf("sql query succeeded\n");
          }
          
          SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
        }
        
        struct timeb stop;
        ftime(&stop);
        
        double elapsed = ((stop.time * 1000 + stop.millitm) - (start.time * 1000 + start.millitm));
        
        printf("%d queries issued in %f seconds, %f/sec\n", iterations, (double) elapsed / 1000, iterations/((double) elapsed / 1000));
      }
      else {
        printf("here3\n");
        printError("SQLDriverConnect", m_hDBC, SQL_HANDLE_DBC);
      }
    }
    else {
      printError("SQLAllocHandle - dbc", m_hEnv, SQL_HANDLE_ENV);
    }
  }
  else {
    printError("SQLAllocHandle - env", m_hEnv, SQL_HANDLE_ENV);
  }
  
  //SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
  //SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
  
  return 0;
}

void printError(const char *fn, SQLHANDLE handle, SQLSMALLINT type)
{
  SQLINTEGER i = 0;
  SQLINTEGER native;
  SQLCHAR state[ 7 ];
  SQLCHAR text[256];
  SQLSMALLINT len;
  SQLRETURN ret;

  printf(
    "\n"
    "The driver reported the following diagnostics whilst running "
    "%s\n\n",
    fn
  );

  do {
    ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len );
    if (SQL_SUCCEEDED(ret))
      printf("%s:%ld:%ld:%s\n", state, (long int) i, (long int) native, text);
  }
  while( ret == SQL_SUCCESS );
}

#ifdef dynodbc

#include "dynodbc.h"
#include <stdio.h>

#ifdef _WIN32
  #include <windows.h>
#elif defined(__GNUC__) // GNU compiler
  #include <dlfcn.h>
#else
#error define your copiler
#endif

#include<string>
/*
#define RTLD_LAZY   1
#define RTLD_NOW    2
#define RTLD_GLOBAL 4
*/

void* LoadSharedLibrary(char *pcDllname, int iMode = 2)
{
  std::string sDllName = pcDllname;
#ifdef _WIN32
  sDllName += ".dll";
  return (void*)LoadLibraryA(pcDllname);
#elif defined(__GNUC__) // GNU compiler
  sDllName += ".so";
  void* handle = dlopen(sDllName.c_str(),iMode);
  
  if (!handle) {
    printf("node-odbc: error loading ODBC library: %s\n", dlerror());
  }
  
  return handle;
#endif
}

void* GetFunction(void *Lib, char *Fnname)
{
#if defined(_MSC_VER) // Microsoft compiler
  return (void*)GetProcAddress((HINSTANCE)Lib,Fnname);
#elif defined(__GNUC__) // GNU compiler
  void * tmp = dlsym(Lib, Fnname);
  if (!tmp) {
    printf("node-odbc: error loading function: %s\n", Fnname);
  }
  return tmp;
#endif
}

bool FreeSharedLibrary(void *hDLL)
{
#if defined(_MSC_VER) // Microsoft compiler
  return (FreeLibrary((HINSTANCE)hDLL)!=0);
#elif defined(__GNUC__) // GNU compiler
  return dlclose(hDLL);
#endif
}

pfnSQLGetData           pSQLGetData;
pfnSQLGetFunctions      pSQLGetFunctions;
pfnSQLAllocConnect      pSQLAllocConnect;
pfnSQLAllocEnv          pSQLAllocEnv;
pfnSQLAllocStmt         pSQLAllocStmt;
pfnSQLBindCol           pSQLBindCol;
pfnSQLCancel            pSQLCancel;
pfnSQLColAttributes     pSQLColAttributes;
pfnSQLConnect           pSQLConnect;
pfnSQLDescribeCol       pSQLDescribeCol;
pfnSQLDisconnect        pSQLDisconnect;
pfnSQLError             pSQLError;
pfnSQLExecDirect        pSQLExecDirect;
pfnSQLExecute           pSQLExecute;
pfnSQLFetch             pSQLFetch;
pfnSQLGetDiagRec        pSQLGetDiagRec;
pfnSQLGetDiagField      pSQLGetDiagField;
pfnSQLFreeHandle        pSQLFreeHandle;
pfnSQLFetchScroll       pSQLFetchScroll;
pfnSQLColAttribute      pSQLColAttribute;
pfnSQLSetConnectAttr    pSQLSetConnectAttr;
pfnSQLDriverConnect     pSQLDriverConnect;
pfnSQLAllocHandle       pSQLAllocHandle;
pfnSQLRowCount          pSQLRowCount;
pfnSQLNumResultCols     pSQLNumResultCols;
pfnSQLEndTran           pSQLEndTran;
pfnSQLTables            pSQLTables;
pfnSQLColumns           pSQLColumns;
pfnSQLBindParameter     pSQLBindParameter;
pfnSQLPrimaryKeys       pSQLPrimaryKeys;
pfnSQLSetEnvAttr        pSQLSetEnvAttr  ;
pfnSQLFreeConnect       pSQLFreeConnect;
pfnSQLFreeEnv           pSQLFreeEnv;
pfnSQLFreeStmt          pSQLFreeStmt;                      
pfnSQLGetCursorName     pSQLGetCursorName;
pfnSQLPrepare           pSQLPrepare;
pfnSQLSetCursorName     pSQLSetCursorName;
pfnSQLTransact          pSQLTransact;
pfnSQLSetConnectOption  pSQLSetConnectOption;
pfnSQLDrivers           pSQLDrivers;
pfnSQLDataSources       pSQLDataSources;
pfnSQLGetInfo           pSQLGetInfo;
pfnSQLMoreResults       pSQLMoreResults;

//#define LOAD_ENTRY( hMod, Name ) (p##Name = (pfn##Name) GetProcAddress( (hMod), #Name ))
#define LOAD_ENTRY( hMod, Name ) (p##Name = (pfn##Name) GetFunction( (hMod), #Name ))

static BOOL  s_fODBCLoaded = false;

BOOL DynLoadODBC( char* odbcModuleName )
{
#ifdef _WIN32
    HMODULE hMod;
#elif defined(__GNUC__) // GNU compiler
    void* hMod;
#endif

    if ( s_fODBCLoaded )
      return true;

 //   if ( (hMod = (HMODULE) LoadLibrary( odbcModuleName ))) {
#ifdef _WIN32
  if ( (hMod = (HMODULE) LoadSharedLibrary( odbcModuleName ))) {
#elif defined(__GNUC__) // GNU compiler
  if ( (hMod = (void *) LoadSharedLibrary( odbcModuleName ))) {
#endif

//#if (ODBCVER < 0x0300)
  if (LOAD_ENTRY( hMod, SQLGetData   )  )
  if (LOAD_ENTRY( hMod, SQLGetFunctions   )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLAllocConnect   )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLAllocEnv       )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLAllocStmt      )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLColAttributes  )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLError          )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLFreeConnect    )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLFreeEnv        )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLTransact       )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLSetConnectOption )  )
/*
 * NOTE: This is commented out because it wouldn't be used
 * in a direct-to-driver situation and we currently never
 * call SQLDrivers. But if we ever do we may need to have
 * some type of flag to determine if we should try to load
 * this function if the user is not doing a direct-to-driver
 * and is specifying a specific libodbc library.
 */
//Unused-> if (LOAD_ENTRY( hMod, SQLDrivers        )  )

  //Unused-> if (LOAD_ENTRY( hMod, SQLDataSources    )  )
//#endif
  //Unused-> if (LOAD_ENTRY( hMod, SQLBindCol        )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLCancel         )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLConnect       )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLDescribeCol    )  )
  if (LOAD_ENTRY( hMod, SQLDisconnect     )  )
  if (LOAD_ENTRY( hMod, SQLExecDirect     )  )
  if (LOAD_ENTRY( hMod, SQLExecute        )  )
  if (LOAD_ENTRY( hMod, SQLFetch          )  )
  if (LOAD_ENTRY( hMod, SQLGetDiagRec     )  )
  if (LOAD_ENTRY( hMod, SQLGetDiagField   )  )
  if (LOAD_ENTRY( hMod, SQLFreeHandle     )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLFetchScroll    )  )
  if (LOAD_ENTRY( hMod, SQLColAttribute   )  )
  if (LOAD_ENTRY( hMod, SQLSetConnectAttr )  )
  if (LOAD_ENTRY( hMod, SQLDriverConnect  )  )
  if (LOAD_ENTRY( hMod, SQLAllocHandle    )  )
  if (LOAD_ENTRY( hMod, SQLRowCount       )  )
  if (LOAD_ENTRY( hMod, SQLNumResultCols  )  )
  if (LOAD_ENTRY( hMod, SQLEndTran        )  )
  if (LOAD_ENTRY( hMod, SQLTables         )  )
  if (LOAD_ENTRY( hMod, SQLColumns        )  )
  if (LOAD_ENTRY( hMod, SQLBindParameter  )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLPrimaryKeys)  )
  if (LOAD_ENTRY( hMod, SQLSetEnvAttr     )  )
  if (LOAD_ENTRY( hMod, SQLFreeStmt       )  )
  if (LOAD_ENTRY( hMod, SQLPrepare        )  )
  //Unused-> if (LOAD_ENTRY( hMod, SQLGetInfo        )  )
  if (LOAD_ENTRY( hMod, SQLBindParameter  )  )
  if (LOAD_ENTRY( hMod, SQLMoreResults    )
          ) {

          s_fODBCLoaded = true;
      }
  }

  return (s_fODBCLoaded);
}
#endif

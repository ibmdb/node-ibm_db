#ifndef _SRC_DYNODBC_H_
#define _SRC_DYNODBC_H_

#ifdef dynodbc

#ifdef _WIN32
#include <windows.h>
#endif
#include <sql.h>
#include <sqlext.h>

typedef RETCODE (SQL_API * pfnSQLGetData)(
  SQLHSTMT       StatementHandle,
  SQLUSMALLINT   Col_or_Param_Num,
  SQLSMALLINT    TargetType,
  SQLPOINTER     TargetValuePtr,
  SQLLEN         BufferLength,
  SQLLEN *       StrLen_or_IndPtr);

typedef RETCODE  (SQL_API * pfnSQLGetFunctions)(
  SQLHDBC           ConnectionHandle,
  SQLUSMALLINT      FunctionId,
  SQLUSMALLINT *    SupportedPtr);

typedef RETCODE (SQL_API * pfnSQLAllocConnect)(
  SQLHENV        henv,
  SQLHDBC   FAR *phdbc);

typedef RETCODE (SQL_API * pfnSQLAllocEnv)(
  SQLHENV   FAR *phenv);

typedef RETCODE (SQL_API * pfnSQLAllocStmt)(
  SQLHDBC        hdbc,
  SQLHSTMT  FAR *phstmt);

typedef RETCODE (SQL_API * pfnSQLBindCol)(
  SQLHSTMT       hstmt,
  UWORD       icol,
  SWORD       fCType,
  PTR         rgbValue,
  SDWORD      cbValueMax,
  SDWORD FAR *pcbValue);

typedef RETCODE (SQL_API * pfnSQLCancel)(
  SQLHSTMT       hstmt);

typedef RETCODE (SQL_API * pfnSQLColAttributes)(
  SQLHSTMT       hstmt,
  UWORD       icol,
  UWORD       fDescType,
  PTR         rgbDesc,
  SWORD       cbDescMax,
  SWORD  FAR *pcbDesc,
  SDWORD FAR *pfDesc);


typedef RETCODE (SQL_API * pfnSQLColAttribute)(
  SQLHSTMT StatementHandle,
  SQLUSMALLINT ColumnNumber, SQLUSMALLINT FieldIdentifier,
  SQLPOINTER CharacterAttribute, SQLSMALLINT BufferLength,
  SQLSMALLINT *StringLength, SQLPOINTER NumericAttribute);

typedef RETCODE (SQL_API * pfnSQLSetConnectAttr)(
  SQLHDBC ConnectionHandle,
  SQLINTEGER Attribute, SQLPOINTER Value,
  SQLINTEGER StringLength);

typedef RETCODE (SQL_API * pfnSQLDriverConnect)(    
  SQLHDBC            hdbc,
  SQLHWND            hwnd,
  SQLTCHAR           *szConnStrIn,
  SQLSMALLINT        cbConnStrIn,
  SQLTCHAR           *szConnStrOut,
  SQLSMALLINT        cbConnStrOutMax,
  SQLSMALLINT       *pcbConnStrOut,
  SQLUSMALLINT       fDriverCompletion);

typedef RETCODE (SQL_API * pfnSQLAllocHandle)(    
  SQLSMALLINT HandleType,
  SQLHANDLE InputHandle, SQLHANDLE *OutputHandle);

typedef RETCODE (SQL_API * pfnSQLRowCount)(
  SQLHSTMT StatementHandle, 
  SQLLEN *RowCount);

typedef RETCODE (SQL_API * pfnSQLNumResultCols)(
  SQLHSTMT StatementHandle,
  SQLSMALLINT *ColumnCount);

typedef RETCODE (SQL_API * pfnSQLEndTran)(
  SQLSMALLINT HandleType, SQLHANDLE Handle,
  SQLSMALLINT CompletionType);

typedef RETCODE (SQL_API * pfnSQLExecDirect)(
  SQLHSTMT StatementHandle,
  SQLTCHAR *StatementText, SQLINTEGER TextLength);
  

typedef RETCODE (SQL_API * pfnSQLTables)(
  SQLHSTMT StatementHandle,
  SQLTCHAR *CatalogName, SQLSMALLINT NameLength1,
  SQLTCHAR *SchemaName, SQLSMALLINT NameLength2,
  SQLTCHAR *TableName, SQLSMALLINT NameLength3,
  SQLTCHAR *TableType, SQLSMALLINT NameLength4);

typedef RETCODE (SQL_API * pfnSQLColumns)(
  SQLHSTMT StatementHandle,
  SQLTCHAR *CatalogName, SQLSMALLINT NameLength1,
  SQLTCHAR *SchemaName, SQLSMALLINT NameLength2,
  SQLTCHAR *TableName, SQLSMALLINT NameLength3,
  SQLTCHAR *ColumnName, SQLSMALLINT NameLength4);

typedef RETCODE (SQL_API * pfnSQLBindParameter)(    
  SQLHSTMT           hstmt,
  SQLUSMALLINT       ipar,
  SQLSMALLINT        fParamType,
  SQLSMALLINT        fCType,
  SQLSMALLINT        fSqlType,
  SQLUINTEGER        cbColDef,
  SQLSMALLINT        ibScale,
  SQLPOINTER         rgbValue,
  SQLINTEGER         cbValueMax,
  SQLLEN        *pcbValue);

typedef RETCODE (SQL_API * pfnSQLPrimaryKeys)(
  SQLHSTMT           hstmt,
  SQLTCHAR           *szCatalogName,
  SQLSMALLINT        cbCatalogName,
  SQLTCHAR           *szSchemaName,
  SQLSMALLINT        cbSchemaName,
  SQLTCHAR           *szTableName,
  SQLSMALLINT        cbTableName);

typedef RETCODE (SQL_API * pfnSQLSetEnvAttr)(
  SQLHENV EnvironmentHandle,
  SQLINTEGER Attribute, SQLPOINTER Value,
  SQLINTEGER StringLength);


typedef RETCODE (SQL_API * pfnSQLConnect)(
  SQLHDBC        hdbc,
  UCHAR  FAR *szDSN,
  SWORD       cbDSN,
  UCHAR  FAR *szUID,
  SWORD       cbUID,
  UCHAR  FAR *szAuthStr,
  SWORD       cbAuthStr);

typedef RETCODE (SQL_API * pfnSQLDescribeCol)(
  SQLHSTMT       hstmt,
  UWORD       icol,
  UCHAR  FAR *szColName,
  SWORD       cbColNameMax,
  SWORD  FAR *pcbColName,
  SWORD  FAR *pfSqlType,
  UDWORD FAR *pcbColDef,
  SWORD  FAR *pibScale,
  SWORD  FAR *pfNullable);

typedef RETCODE (SQL_API * pfnSQLDisconnect)(
  SQLHDBC        hdbc);

typedef RETCODE (SQL_API * pfnSQLError)(
  SQLHENV        henv,
  SQLHDBC        hdbc,
  SQLHSTMT       hstmt,
  UCHAR  FAR *szSqlState,
  SDWORD FAR *pfNativeError,
  UCHAR  FAR *szErrorMsg,
  SWORD       cbErrorMsgMax,
  SWORD  FAR *pcbErrorMsg);

/*typedef RETCODE (SQL_API * pfnSQLExecDirect)(
  HSTMT       hstmt,
  UCHAR  FAR *szSqlStr,
  SDWORD      cbSqlStr);
*/
typedef RETCODE (SQL_API * pfnSQLExecute)(
  SQLHSTMT       hstmt);

typedef RETCODE (SQL_API * pfnSQLFetch)(
  SQLHSTMT       hstmt);

typedef RETCODE (SQL_API * pfnSQLGetDiagRec)(
  SQLSMALLINT HandleType, SQLHANDLE Handle,
  SQLSMALLINT RecNumber, SQLTCHAR *Sqlstate,
  SQLINTEGER *NativeError, SQLTCHAR *MessageText,
  SQLSMALLINT BufferLength, SQLSMALLINT *TextLength);

typedef RETCODE (SQL_API * pfnSQLGetDiagField)(
  SQLSMALLINT HandleType, SQLHANDLE Handle,
  SQLSMALLINT RecNumber, SQLSMALLINT DiagIdentifier,
  SQLPOINTER DiagInfoPtr, SQLSMALLINT BufferLength,
  SQLSMALLINT *StringLengthPtr);

typedef RETCODE (SQL_API * pfnSQLFreeHandle)(
  SQLSMALLINT HandleType, SQLHANDLE Handle);

typedef RETCODE (SQL_API * pfnSQLFetchScroll)(
  SQLHSTMT StatementHandle,
  SQLSMALLINT FetchOrientation, SQLINTEGER FetchOffset);

typedef RETCODE (SQL_API * pfnSQLColAttribute)(
  SQLHSTMT StatementHandle,
  SQLUSMALLINT ColumnNumber, SQLUSMALLINT FieldIdentifier,
  SQLPOINTER CharacterAttribute, SQLSMALLINT BufferLength,
  SQLSMALLINT *StringLength, SQLPOINTER NumericAttribute);


typedef RETCODE (SQL_API * pfnSQLFreeConnect)(
  SQLHDBC        hdbc);

typedef RETCODE (SQL_API * pfnSQLFreeEnv)(
  SQLHENV        henv);

typedef RETCODE (SQL_API * pfnSQLFreeStmt)(
  SQLHSTMT       hstmt,
  UWORD       fOption);

typedef RETCODE (SQL_API * pfnSQLGetCursorName)(
  SQLHSTMT       hstmt,
  UCHAR  FAR *szCursor,
  SWORD       cbCursorMax,
  SWORD  FAR *pcbCursor);

typedef RETCODE (SQL_API * pfnSQLNumResultCols)(
  SQLHSTMT       hstmt,
  SWORD  FAR *pccol);

typedef RETCODE (SQL_API * pfnSQLPrepare)(
  SQLHSTMT    StatementHandle,
  SQLTCHAR   *StatementText,
  SQLINTEGER  TextLength);
//  HSTMT       hstmt,
//  UCHAR  FAR *szSqlStr,
//  SDWORD      cbSqlStr);

typedef RETCODE (SQL_API * pfnSQLRowCount)(
  SQLHSTMT       hstmt,
  SQLLEN FAR *pcrow);

typedef RETCODE (SQL_API * pfnSQLSetCursorName)(
  SQLHSTMT       hstmt,
  UCHAR  FAR *szCursor,
  SWORD       cbCursor);

typedef RETCODE (SQL_API * pfnSQLTransact)(
  SQLHENV        henv,
  SQLHDBC        hdbc,
  UWORD       fType);

typedef RETCODE (SQL_API * pfnSQLSetConnectOption)(
  SQLHDBC        hdbc,
  UWORD       fOption,
  UDWORD      vParam);

typedef RETCODE (SQL_API * pfnSQLDrivers)(
  SQLHENV        henv,
  UWORD       fDirection,
  UCHAR FAR  *szDriverDesc,
  SWORD       cbDriverDescMax,
  SWORD FAR  *pcbDriverDesc,
  UCHAR FAR  *szDriverAttributes,
  SWORD       cbDrvrAttrMax,
  SWORD  FAR *pcbDrvrAttr);

// typedef RETCODE (SQL_API * pfnSQLBindParameter)(
//   HSTMT       hstmt,
//   UWORD       ipar,
//   SWORD       fParamType,
//   SWORD       fCType,
//   SWORD       fSqlType,
//   UDWORD      cbColDef,
//   SWORD       ibScale,
//   PTR         rgbValue,
//   SDWORD      cbValueMax,
//   SDWORD FAR *pcbValue);

typedef RETCODE (SQL_API * pfnSQLDataSources)(
  SQLHENV        henv,
  UWORD       fDirection,
  UCHAR  FAR *szDSN,
  SWORD       cbDSNMax,
  SWORD  FAR *pcbDSN,
  UCHAR  FAR *szDescription,
  SWORD       cbDescriptionMax,
  SWORD  FAR *pcbDescription);

typedef RETCODE (SQL_API * pfnSQLGetInfo)(
  SQLHDBC        hdbc,
  UWORD       fInfoType,
  PTR         rgbInfoValue,
  SWORD       cbInfoValueMax,
  SWORD  FAR *pcbInfoValue);

typedef RETCODE (SQL_API * pfnSQLMoreResults)(
  SQLHSTMT       hstmt);

extern pfnSQLGetData            pSQLGetData;
extern pfnSQLGetFunctions       pSQLGetFunctions;
extern pfnSQLAllocConnect       pSQLAllocConnect;
extern pfnSQLAllocEnv           pSQLAllocEnv;
extern pfnSQLAllocStmt          pSQLAllocStmt;
extern pfnSQLBindCol            pSQLBindCol;
extern pfnSQLCancel             pSQLCancel;
extern pfnSQLColAttributes      pSQLColAttributes;
extern pfnSQLConnect            pSQLConnect;
extern pfnSQLDescribeCol        pSQLDescribeCol;
extern pfnSQLDisconnect         pSQLDisconnect;
extern pfnSQLError              pSQLError;
extern pfnSQLExecDirect         pSQLExecDirect;
extern pfnSQLExecute            pSQLExecute;
extern pfnSQLFetch              pSQLFetch;
extern pfnSQLGetDiagRec         pSQLGetDiagRec;
extern pfnSQLGetDiagField       pSQLGetDiagField;
extern pfnSQLFreeHandle         pSQLFreeHandle;
extern pfnSQLFetchScroll        pSQLFetchScroll;
extern pfnSQLFetchScroll        pSQLFetchScroll;
extern pfnSQLColAttribute       pSQLColAttribute; 
extern pfnSQLSetConnectAttr     pSQLSetConnectAttr;
extern pfnSQLDriverConnect      pSQLDriverConnect;
extern pfnSQLAllocHandle        pSQLAllocHandle;
extern pfnSQLRowCount           pSQLRowCount;
extern pfnSQLNumResultCols      pSQLNumResultCols;
extern pfnSQLEndTran            pSQLEndTran;
//extern pfnSQLExecDirect         pSQLExecDirect;
extern pfnSQLTables             pSQLTables;
extern pfnSQLColumns            pSQLColumns;
// extern pfnSQLBindParameter      pSQLBindParameter;
extern pfnSQLPrimaryKeys        pSQLPrimaryKeys;
extern pfnSQLSetEnvAttr         pSQLSetEnvAttr;
extern pfnSQLFreeConnect        pSQLFreeConnect;
extern pfnSQLFreeEnv            pSQLFreeEnv;
extern pfnSQLFreeStmt           pSQLFreeStmt;
extern pfnSQLGetCursorName      pSQLGetCursorName;
extern pfnSQLNumResultCols      pSQLNumResultCols;
extern pfnSQLPrepare            pSQLPrepare;
extern pfnSQLRowCount           pSQLRowCount;
extern pfnSQLSetCursorName      pSQLSetCursorName;
extern pfnSQLTransact           pSQLTransact;
extern pfnSQLSetConnectOption   pSQLSetConnectOption;
extern pfnSQLDrivers            pSQLDrivers;
extern pfnSQLDataSources        pSQLDataSources;
extern pfnSQLBindParameter      pSQLBindParameter;
extern pfnSQLGetInfo            pSQLGetInfo;
extern pfnSQLMoreResults        pSQLMoreResults;

BOOL DynLoadODBC( char* odbcModuleName );

#define SQLAllocEnv pSQLAllocEnv
#define SQLAllocConnect pSQLAllocConnect
#define SQLSetConnectOption pSQLSetConnectOption
#define SQLAllocStmt pSQLAllocStmt
#define SQLGetFunctions pSQLGetFunctions
#define SQLError pSQLError
#define SQLGetData pSQLGetData
#define SQLMoreResults pSQLMoreResults
#define SQLPrepare pSQLPrepare
#define SQLExecute pSQLExecute
#define SQLGetDiagRec pSQLGetDiagRec
#define SQLGetDiagField pSQLGetDiagField
#define SQLFreeHandle pSQLFreeHandle
#define SQLFreeStmt pSQLFreeStmt
#define SQLFetchScroll pSQLFetchScroll
#define SQLFetch pSQLFetch
#define SQLBindCol pSQLBindCol
#define SQLColAttribute pSQLColAttribute
#define SQLGetInfo pSQLGetInfo
#define SQLDriverConnect pSQLDriverConnect
#define SQLAllocHandle pSQLAllocHandle
#define SQLDisconnect pSQLDisconnect
#define SQLRowCount pSQLRowCount
#define SQLNumResultCols pSQLNumResultCols
#define SQLSetConnectAttr pSQLSetConnectAttr
#define SQLEndTran pSQLEndTran
#define SQLExecDirect pSQLExecDirect
#define SQLTables pSQLTables
#define SQLColumns pSQLColumns
#define SQLBindParameter pSQLBindParameter
#define SQLPrimaryKeys pSQLPrimaryKeys
#define SQLSetEnvAttr pSQLSetEnvAttr
#endif
#endif // _SRC_DYNODBC_H_

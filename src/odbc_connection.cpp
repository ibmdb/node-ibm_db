/*
  Copyright (c) 2013, Dan VerWeire <dverweire@gmail.com>
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
#include <node_version.h>
#include <time.h>
#include <uv.h>

#include "odbc.h"
#include "odbc_connection.h"
#include "odbc_result.h"
#include "odbc_statement.h"

using namespace v8;
using namespace node;

Nan::Persistent<Function> ODBCConnection::constructor;
Nan::Persistent<String> ODBCConnection::OPTION_SQL;
Nan::Persistent<String> ODBCConnection::OPTION_PARAMS;
Nan::Persistent<String> ODBCConnection::OPTION_NORESULTS;
Nan::Persistent<String> ODBCConnection::OPTION_ARRAYSIZE;

NAN_MODULE_INIT(ODBCConnection::Init)
{
  DEBUG_PRINTF("ODBCConnection::Init\n");
  Nan::HandleScope scope;

  OPTION_SQL.Reset(Nan::New<String>("sql").ToLocalChecked());
  OPTION_PARAMS.Reset(Nan::New<String>("params").ToLocalChecked());
  OPTION_NORESULTS.Reset(Nan::New<String>("noResults").ToLocalChecked());
  OPTION_ARRAYSIZE.Reset(Nan::New<String>("ArraySize").ToLocalChecked());

  Local<FunctionTemplate> constructor_template = Nan::New<FunctionTemplate>(New);

  // Constructor Template
  constructor_template->SetClassName(Nan::New("ODBCConnection").ToLocalChecked());

  // Reserve space for one Handle<Value>
  Local<ObjectTemplate> instance_template = constructor_template->InstanceTemplate();
  instance_template->SetInternalFieldCount(1);

  // Properties
  // Nan::SetAccessor(instance_template, Nan::New("mode").ToLocalChecked(), ModeGetter, ModeSetter);
  Nan::SetAccessor(instance_template, Nan::New("connected").ToLocalChecked(), ConnectedGetter);
  Nan::SetAccessor(instance_template, Nan::New("connectTimeout").ToLocalChecked(), ConnectTimeoutGetter, ConnectTimeoutSetter);
  Nan::SetAccessor(instance_template, Nan::New("systemNaming").ToLocalChecked(), SystemNamingGetter, SystemNamingSetter);

  // Prototype Methods
  Nan::SetPrototypeMethod(constructor_template, "open", Open);
  Nan::SetPrototypeMethod(constructor_template, "openSync", OpenSync);
  Nan::SetPrototypeMethod(constructor_template, "close", Close);
  Nan::SetPrototypeMethod(constructor_template, "closeSync", CloseSync);

  Nan::SetPrototypeMethod(constructor_template, "createDbSync", CreateDbSync);
  Nan::SetPrototypeMethod(constructor_template, "dropDbSync", DropDbSync);

  Nan::SetPrototypeMethod(constructor_template, "createStatement", CreateStatement);
  Nan::SetPrototypeMethod(constructor_template, "createStatementSync", CreateStatementSync);
  Nan::SetPrototypeMethod(constructor_template, "query", Query);
  Nan::SetPrototypeMethod(constructor_template, "querySync", QuerySync);

  Nan::SetPrototypeMethod(constructor_template, "beginTransaction", BeginTransaction);
  Nan::SetPrototypeMethod(constructor_template, "beginTransactionSync", BeginTransactionSync);
  Nan::SetPrototypeMethod(constructor_template, "endTransaction", EndTransaction);
  Nan::SetPrototypeMethod(constructor_template, "endTransactionSync", EndTransactionSync);

  Nan::SetPrototypeMethod(constructor_template, "setIsolationLevel", SetIsolationLevel);
  Nan::SetPrototypeMethod(constructor_template, "getInfo", GetInfo);
  Nan::SetPrototypeMethod(constructor_template, "getInfoSync", GetInfoSync);
  Nan::SetPrototypeMethod(constructor_template, "getTypeInfo", GetTypeInfo);
  Nan::SetPrototypeMethod(constructor_template, "getTypeInfoSync", GetTypeInfoSync);
  Nan::SetPrototypeMethod(constructor_template, "getFunctions", GetFunctions);
  Nan::SetPrototypeMethod(constructor_template, "getFunctionsSync", GetFunctionsSync);
  Nan::SetPrototypeMethod(constructor_template, "setAttr", SetAttr);
  Nan::SetPrototypeMethod(constructor_template, "setAttrSync", SetAttrSync);

  Nan::SetPrototypeMethod(constructor_template, "columns", Columns);
  Nan::SetPrototypeMethod(constructor_template, "tables", Tables);

  // Attach the Database Constructor to the target object
  constructor.Reset(Nan::GetFunction(constructor_template).ToLocalChecked());
  Nan::Set(target, Nan::New("ODBCConnection").ToLocalChecked(),
           Nan::GetFunction(constructor_template).ToLocalChecked());
}

ODBCConnection::~ODBCConnection()
{
  DEBUG_PRINTF("ODBCConnection::~ODBCConnection\n");
  this->Free();
}

void ODBCConnection::Free()
{
  DEBUG_PRINTF("ODBCConnection::Free m_hDBC = %i \n", m_hDBC);
  if (m_hDBC)
  {
    SQLDisconnect(m_hDBC);
    SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
    m_hDBC = (SQLHDBC)NULL;
  }
}

/*
 * New
 */

NAN_METHOD(ODBCConnection::New)
{
  DEBUG_PRINTF("ODBCConnection::New - Entry\n");
  Nan::HandleScope scope;

  REQ_EXT_ARG(0, js_henv);
  REQ_EXT_ARG(1, js_hdbc);

  SQLHENV hENV = (SQLHENV)((intptr_t)js_henv->Value());
  SQLHDBC hDBC = (SQLHDBC)((intptr_t)js_hdbc->Value());

  ODBCConnection *conn = new ODBCConnection(hENV, hDBC);

  conn->Wrap(info.Holder());

  // set default connectTimeout to 30 seconds
  conn->connectTimeout = DEFAULT_CONNECTION_TIMEOUT;

  conn->systemNaming = false;

  info.GetReturnValue().Set(info.Holder());
  DEBUG_PRINTF("ODBCConnection::New - Exit\n");
}

NAN_GETTER(ODBCConnection::ConnectedGetter)
{
  Nan::HandleScope scope;

  ODBCConnection *obj = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  info.GetReturnValue().Set(obj->connected ? Nan::True() : Nan::False());
}

NAN_GETTER(ODBCConnection::ConnectTimeoutGetter)
{
  Nan::HandleScope scope;

  ODBCConnection *obj = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  info.GetReturnValue().Set(Nan::New<Number>(obj->connectTimeout));
}

NAN_SETTER(ODBCConnection::ConnectTimeoutSetter)
{
  Nan::HandleScope scope;

  ODBCConnection *obj = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  if (value->IsNumber())
  {
    obj->connectTimeout = Nan::To<int32_t>(value).FromJust();
  }
}

NAN_GETTER(ODBCConnection::SystemNamingGetter)
{
  Nan::HandleScope scope;

  ODBCConnection *obj = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  info.GetReturnValue().Set(obj->systemNaming ? Nan::True() : Nan::False());
}

NAN_SETTER(ODBCConnection::SystemNamingSetter)
{
  Nan::HandleScope scope;

  ODBCConnection *obj = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  obj->systemNaming = Nan::To<bool>(value).FromJust();
}

/*
 * Open
 *
 */

NAN_METHOD(ODBCConnection::Open)
{
  DEBUG_PRINTF("ODBCConnection::Open - Entry\n");
  Nan::HandleScope scope;
  int len = 0;
  const char *errmsg = NULL;

  REQ_STRO_ARG(0, connection);
  REQ_FUN_ARG(1, cb);

  // get reference to the connection object
  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  // create a uv work request
  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  // allocate our worker data
  open_connection_work_data *data = (open_connection_work_data *)
      calloc(1, sizeof(open_connection_work_data));
  MEMCHECK2(data, errmsg);

  len = connection.length();
  data->connectionLength = len + 1;

  // copy the connection string to the work data
  GETCPPSTR2(data->connection, connection, len, errmsg);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;

  work_req->data = data;

  // queue the work
  uv_queue_work(uv_default_loop(),
                work_req,
                UV_Open,
                (uv_after_work_cb)UV_AfterOpen);

  conn->Ref();

exit:
  DEBUG_PRINTF("ODBCConnection::Open - Exit\n");
  if (errmsg)
  {
    free(work_req);
    free(data);
    return Nan::ThrowError(errmsg);
  }

  info.GetReturnValue().Set(info.Holder());
}

void ODBCConnection::UV_Open(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_Open - Entry\n");
  open_connection_work_data *data = (open_connection_work_data *)(req->data);

  ODBCConnection *self = data->conn->self();

  SetConnectionAttributes(self);

  // Attempt to connect
  // NOTE: SQLDriverConnect requires the thread to be locked
  int ret = SQLDriverConnect(
      self->m_hDBC,                 // ConnectionHandle
      NULL,                         // WindowHandle
      (SQLTCHAR *)data->connection, // InConnectionString
      data->connectionLength,       // StringLength1
      NULL,                         // OutConnectionString
      0,                            // BufferLength - in characters
      NULL,                         // StringLength2Ptr
      SQL_DRIVER_NOPROMPT);         // DriverCompletion

  if (SQL_SUCCEEDED(ret))
  {
    SQLHSTMT hStmt;

    // allocate a temporary statment
    ret = SQLAllocHandle(SQL_HANDLE_STMT, self->m_hDBC, &hStmt);

    // try to determine if the driver can handle
    // multiple recordsets
    ret = SQLGetFunctions(
        self->m_hDBC,
        SQL_API_SQLMORERESULTS,
        &self->canHaveMoreResults);

    if (!SQL_SUCCEEDED(ret))
    {
      self->canHaveMoreResults = 0;
    }

    // free the handle
    ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    hStmt = (SQLHSTMT)NULL;
  }

  data->result = ret;
  DEBUG_PRINTF("ODBCConnection::UV_Open - Exit\n");
}

void ODBCConnection::UV_AfterOpen(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterOpen - Entry\n");
  Nan::HandleScope scope;

  open_connection_work_data *data = (open_connection_work_data *)(req->data);

  Local<Value> argv[1];

  bool err = false;

  if (data->result)
  {
    err = true;

    Local<Value> objError = ODBC::GetSQLError(SQL_HANDLE_DBC, data->conn->self()->m_hDBC);

    argv[0] = objError;
  }

  if (!err)
  {
    data->conn->self()->connected = true;
  }

  Nan::TryCatch try_catch;

  data->conn->Unref();
  data->cb->Call(err ? 1 : 0, argv);

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  delete data->cb;

  FREE(data->connection);
  FREE(data);
  FREE(req);
  DEBUG_PRINTF("ODBCConnection::UV_AfterOpen - Exit\n");
}

void ODBCConnection::SetConnectionAttributes(ODBCConnection *conn)
{
  SQLRETURN rc = SQL_SUCCESS;
  SQLUINTEGER timeOut = conn->connectTimeout;
  DEBUG_PRINTF("ODBCConnection::SetConnectionAttributes - timeOut = %i, systemNaming = %i\n",
               timeOut, conn->systemNaming);

  if (timeOut > 32767)
  {
    timeOut = DEFAULT_CONNECTION_TIMEOUT;
    DEBUG_PRINTF("ODBCConnection::SetConnectionAttributes - Invalid connection timeout value changed to default.");
  }
  if (timeOut > 0)
  {
    rc = SQLSetConnectAttr(conn->m_hDBC,                  // ConnectionHandle
                           SQL_ATTR_LOGIN_TIMEOUT,        // Attribute
                           (SQLPOINTER)(intptr_t)timeOut, // ValuePtr
                           sizeof(timeOut));              // StringLength
    if (rc != SQL_SUCCESS)
    {
      // We should not disallow connection if rc is not success, though it would never happen.
      // So, ignore any rc and just log here the value for debug build.
      DEBUG_PRINTF("ODBCConnection::SetConnectionAttributes - rc for connectTimeout = %i\n", rc);
    }
  }

// Workaround(zOS): Currently ODBC driver for z/OS doesn't support SQL_ATTR_DBC_SYS_NAMING
// used for connection to DB2 for i.  Temporarily disabling this for now.
#if !defined(__MVS__)
  if (conn->systemNaming)
  {
    rc = SQLSetConnectAttr(conn->m_hDBC,
                           SQL_ATTR_DBC_SYS_NAMING,
                           (SQLPOINTER)SQL_TRUE,
                           SQL_IS_INTEGER);
    if (rc != SQL_SUCCESS)
    {
      DEBUG_PRINTF("ODBCConnection::SetConnectionAttributes - rc for systemNaming = %i\n", rc);
    }
  }
#endif
}

/*
 * OpenSync
 */

NAN_METHOD(ODBCConnection::OpenSync)
{
  DEBUG_PRINTF("ODBCConnection::OpenSync - Entry\n");
  Nan::HandleScope scope;

  REQ_STRO_ARG(0, connection);
  int connectionLength = connection.length();

  // get reference to the connection object
  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  Local<Value> objError;
  SQLRETURN ret;
  bool err = false;
  void *connectionString = NULL;

  if (connectionLength <= 0)
  {
    return Nan::ThrowTypeError("Connection String must be a non-empty string");
  }
  GETCPPSTR(connectionString, connection, connectionLength);

  SetConnectionAttributes(conn);

  // Attempt to connect
  // NOTE: SQLDriverConnect requires the thread to be locked
  ret = SQLDriverConnect(
      conn->m_hDBC,                 // ConnectionHandle
      NULL,                         // WindowHandle
      (SQLTCHAR *)connectionString, // InConnectionString
      connectionLength + 1,         // StringLength1
      NULL,                         // OutConnectionString
      0,                            // BufferLength - in characters
      NULL,                         // StringLength2Ptr
      SQL_DRIVER_NOPROMPT);         // DriverCompletion

  if (!SQL_SUCCEEDED(ret))
  {
    err = true;

    objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->self()->m_hDBC);
  }
  else
  {
    SQLHSTMT hStmt;

    // allocate a temporary statment
    ret = SQLAllocHandle(SQL_HANDLE_STMT, conn->m_hDBC, &hStmt);

    // try to determine if the driver can handle
    // multiple recordsets
    ret = SQLGetFunctions(
        conn->m_hDBC,
        SQL_API_SQLMORERESULTS,
        &conn->canHaveMoreResults);

    if (!SQL_SUCCEEDED(ret))
    {
      conn->canHaveMoreResults = 0;
    }

    // free the handle
    ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    hStmt = (SQLHSTMT)NULL;

    conn->self()->connected = true;

    // only uv_ref if the connection was successful
    /*#if NODE_VERSION_AT_LEAST(0, 7, 9)
      uv_ref((uv_handle_t *)&ODBC::g_async);
    #else
      uv_ref(uv_default_loop());
    #endif*/
  }

  FREE(connectionString);
  DEBUG_PRINTF("ODBCConnection::OpenSync - Exit\n");

  if (err)
  {
    return Nan::ThrowError(objError);
  }
  else
  {
    info.GetReturnValue().Set(Nan::True());
  }
}

/*
 * Close
 *
 */

NAN_METHOD(ODBCConnection::Close)
{
  DEBUG_PRINTF("ODBCConnection::Close\n");
  Nan::HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  close_connection_work_data *data = (close_connection_work_data *)(calloc(1, sizeof(close_connection_work_data)));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;

  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_Close,
      (uv_after_work_cb)UV_AfterClose);

  conn->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCConnection::UV_Close(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_Close\n");
  close_connection_work_data *data = (close_connection_work_data *)(req->data);
  ODBCConnection *conn = data->conn;

  // TODO: check to see if there are any open statements
  // on this connection

  conn->Free();
  conn->connected = false;
  data->result = 0;
}

void ODBCConnection::UV_AfterClose(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterClose\n");
  Nan::HandleScope scope;

  close_connection_work_data *data = (close_connection_work_data *)(req->data);

  ODBCConnection *conn = data->conn;

  Local<Value> argv[1];
  bool err = false;

  if (data->result)
  {
    err = true;
    argv[0] = Exception::Error(Nan::New("Error closing database").ToLocalChecked());
  }
  else
  {
    conn->connected = false;

    // only unref if the connection was closed
    // #if NODE_VERSION_AT_LEAST(0, 7, 9)
    //     uv_unref((uv_handle_t *)&ODBC::g_async);
    // #else
    //     uv_unref(uv_default_loop());
    // #endif
  }

  Nan::TryCatch try_catch;

  data->conn->Unref();
  data->cb->Call(err ? 1 : 0, argv);

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  delete data->cb;

  free(data);
  free(req);
}

/*
 * CloseSync
 */

NAN_METHOD(ODBCConnection::CloseSync)
{
  DEBUG_PRINTF("ODBCConnection::CloseSync - Entry\n");
  Nan::HandleScope scope;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  // TODO: check to see if there are any open statements
  // on this connection

  conn->Free();

  conn->connected = false;

#if NODE_VERSION_AT_LEAST(0, 7, 9)
  uv_unref((uv_handle_t *)&ODBC::g_async);
#else
  uv_unref(uv_default_loop());
#endif

  DEBUG_PRINTF("ODBCConnection::CloseSync - Exit\n");
  info.GetReturnValue().Set(Nan::True());
}

/*  */
/*
 * CreateDbSync -- Creates a Database
 *
 * ===Description
 * Creates a database with the specified name. Returns true if operation successful else false
 * "SQLCreateDb"
 *
 * This function is not supported for Db2 for z/OS servers.
 *
 * ===Parameters
 *
 * connection handle
 *     A valid database connection with parameter ATTACH=true specified.
 *     ('DRIVER={IBM DB2 ODBC DRIVER};ATTACH=true;HOSTNAME=myhost;PORT=1234;PROTOCOL=TCPIP;UID=user;PWD=secret;)
 *
 * dbName
 *     Name of the database that is to be created.
 *
 * codeSet
 *      Database code set information.
 *      Note: If the value of the codeSet argument is not specified,
 *      the database is created in the Unicode code page for DB2 data servers and in the UTF-8 code page for IDS data servers.
 *
 * mode
 *      Database logging mode.
 *      Note: This value is applicable only to "IDS data servers".
 *
 * ===Return Values
 *
 * Returns TRUE on success or FALSE on failure.
 */

NAN_METHOD(ODBCConnection::CreateDbSync)
{
  DEBUG_PRINTF("ODBCConnection::CreateDbSync - Entry\n");
  Nan::HandleScope scope;

#ifdef __MVS__
  // createDbSync API is not supported on z/OS:
  // Databases are implemented/used in Db2 for z/OS differently than
  // and Db2 for LUW.  A database in z/OS is simply a logical collection
  // of table/index spaces that you create using the "CREATE DATABASE"
  // SQL statement, while a database in LUW is conceptually equivalent
  // to a subsystem in z/OS.  Connecting to a Db2 on z/OS subsystem
  // entails a "database" is created already.  As such, this API is
  // not applicable for Db2 on z/OS servers.
  info.GetReturnValue().Set(Nan::False());
#else
  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  SQLRETURN ret;
  Local<Value> objError;
  bool err = false;
  const char *errmsg = NULL;

  int dbNameLength = 0;
  int codeSetLength = 0;
  int modeLength = 0;
  void *databaseNameString = NULL;
  void *codeSetString = NULL;
  void *modeString = NULL;

  // dbName is required, codeSet and mode are optional and can be NULL.
  REQ_ARGS(3);
  REQ_STRO_ARG(0, dbName);
  dbNameLength = dbName.length();
  if (dbNameLength <= 0)
  {
    return Nan::ThrowTypeError("ODBCConnection::CreateDbSync(): Database name must be a String.");
  }

  REQ_STRO_OR_NULL_ARG(1, codeSet);
  codeSetLength = codeSet.length();
  REQ_STRO_OR_NULL_ARG(2, mode);
  modeLength = mode.length();

  GETCPPSTR2(databaseNameString, dbName, dbNameLength, errmsg);
  if (!(info[1]->IsNull()))
  {
    GETCPPSTR2(codeSetString, codeSet, codeSetLength, errmsg);
  }
  if (!(info[2]->IsNull()))
  {
    GETCPPSTR2(modeString, mode, modeLength, errmsg);
  }

  ret = SQLCreateDb(conn->m_hDBC,
                    (SQLTCHAR *)databaseNameString,
                    dbNameLength + 1,
                    (SQLTCHAR *)codeSetString,
                    codeSetLength + 1,
                    (SQLTCHAR *)modeString,
                    modeLength + 1);

  if (!SQL_SUCCEEDED(ret))
  {
    err = true;
    objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->self()->m_hDBC);
  }
  else
  {
    /* disconnect from the database */
    ret = SQLDisconnect(conn->m_hDBC);

    if (!SQL_SUCCEEDED(ret))
    {
      err = true;
      objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->self()->m_hDBC);
    }

    // free the handle
    ret = SQLFreeHandle(SQL_HANDLE_DBC, conn->m_hDBC);
    if (!SQL_SUCCEEDED(ret))
    {
      err = true;
      objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->self()->m_hDBC);
    }
  }

exit:
  FREE(databaseNameString);
  FREE(codeSetString);
  FREE(modeString);
  DEBUG_PRINTF("ODBCConnection::CreateDbSync - Exit\n");

  if (errmsg)
  {
    return Nan::ThrowError(errmsg);
  }
  else if (err)
  {
    return Nan::ThrowError(objError);
  }
  else
  {
    info.GetReturnValue().Set(Nan::True());
  }
#endif // __MVS__
}

/*  */
/*
 * DropDbSync -- Drop a Database
 *
 * ===Description
 * Drops a database with the specified name. Returns true if operation successful else false
 * "SQLDropDb"
 *
 * This function is not supported for Db2 for z/OS servers.
 *
 * ===Parameters
 *
 * Connection handle.
 *
 * dbName
 *     Name of the database that is to be dropped.
 *
 * ===Return Values
 *
 * Returns TRUE on success or FALSE on failure.
 */

NAN_METHOD(ODBCConnection::DropDbSync)
{
  DEBUG_PRINTF("ODBCConnection::DropDbSync - Entry\n");
  Nan::HandleScope scope;

#ifdef __MVS__
  // createDbSync API is not supported on z/OS:
  // Databases are implemented/used in Db2 for z/OS differently than
  // and Db2 for LUW.  A database in z/OS is simply a logical collection
  // of table/index spaces that you create using the "CREATE DATABASE"
  // SQL statement, while a database in LUW is conceptually equivalent
  // to a subsystem in z/OS.  Connecting to a Db2 on z/OS subsystem
  // entails a "database" is created already.  As such, this API is
  // not applicable for Db2 on z/OS servers.
  info.GetReturnValue().Set(Nan::False());
#else
  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  SQLRETURN ret;
  Local<Value> objError;
  bool err = false;
  void *databaseNameString = NULL;
  int dbNameLength = 0;

  REQ_STRO_ARG(0, dbName);
  dbNameLength = dbName.length();
  if (dbNameLength <= 0)
  {
    return Nan::ThrowTypeError("Database name must be a string.");
  }
  GETCPPSTR(databaseNameString, dbName, dbNameLength);

  ret = SQLDropDb(conn->m_hDBC,
                  (SQLTCHAR *)databaseNameString,
                  dbNameLength + 1);

  if (!SQL_SUCCEEDED(ret))
  {
    err = true;
    objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->self()->m_hDBC);
  }
  else
  {
    /* disconnect from the database */
    ret = SQLDisconnect(conn->m_hDBC);

    if (!SQL_SUCCEEDED(ret))
    {
      err = true;
      objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->self()->m_hDBC);
    }

    // free the handle
    ret = SQLFreeHandle(SQL_HANDLE_DBC, conn->m_hDBC);
    if (!SQL_SUCCEEDED(ret))
    {
      err = true;
      objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->self()->m_hDBC);
    }
  }

  FREE(databaseNameString);
  DEBUG_PRINTF("ODBCConnection::DropDbSync - Exit\n");

  if (err)
  {
    return Nan::ThrowError(objError);
  }
  else
  {
    info.GetReturnValue().Set(Nan::True());
  }
#endif
}

/*
 * CreateStatementSync
 *
 */

NAN_METHOD(ODBCConnection::CreateStatementSync)
{
  DEBUG_PRINTF("ODBCConnection::CreateStatementSync - Entry\n");
  Nan::HandleScope scope;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  SQLHSTMT hSTMT;

  SQLAllocHandle(
      SQL_HANDLE_STMT,
      conn->m_hDBC,
      &hSTMT);

  Local<Value> params[3];
  params[0] = Nan::New<External>((void *)(intptr_t)conn->m_hENV);
  params[1] = Nan::New<External>((void *)(intptr_t)conn->m_hDBC);
  params[2] = Nan::New<External>((void *)(intptr_t)hSTMT);

  Local<Object> js_result(Nan::NewInstance(Nan::New(ODBCStatement::constructor), 3, params).ToLocalChecked());

  DEBUG_PRINTF("ODBCConnection::CreateStatementSync - Exit\n");
  info.GetReturnValue().Set(js_result);
}

/*
 * CreateStatement
 *
 */

NAN_METHOD(ODBCConnection::CreateStatement)
{
  DEBUG_PRINTF("ODBCConnection::CreateStatement - Entry\n");
  Nan::HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  // initialize work request
  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  // initialize our data
  create_statement_work_data *data =
      (create_statement_work_data *)(calloc(1, sizeof(create_statement_work_data)));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;

  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_CreateStatement,
      (uv_after_work_cb)UV_AfterCreateStatement);

  conn->Ref();

  DEBUG_PRINTF("ODBCConnection::CreateStatement - Exit\n");
  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCConnection::UV_CreateStatement(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_CreateStatement - Entry\n");

  // get our work data
  create_statement_work_data *data = (create_statement_work_data *)(req->data);

  DEBUG_PRINTF("ODBCConnection::UV_CreateStatement m_hENV=%X m_hDBC=%X m_hSTMT=%X\n",
               data->conn->m_hENV,
               data->conn->m_hDBC,
               data->hSTMT);

  // allocate a new statment handle
  SQLAllocHandle(SQL_HANDLE_STMT,
                 data->conn->m_hDBC,
                 &data->hSTMT);

  DEBUG_PRINTF("ODBCConnection::UV_CreateStatement - Exit: hENV=%X hDBC=%X hSTMT=%X\n",
               data->conn->m_hENV,
               data->conn->m_hDBC,
               data->hSTMT);
}

void ODBCConnection::UV_AfterCreateStatement(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterCreateStatement - Entry\n");
  Nan::HandleScope scope;

  create_statement_work_data *data = (create_statement_work_data *)(req->data);

  DEBUG_PRINTF("ODBCConnection::UV_AfterCreateStatement m_hENV=%X m_hDBC=%X hSTMT=%X\n",
               data->conn->m_hENV,
               data->conn->m_hDBC,
               data->hSTMT);

  Local<Value> info[3];
  info[0] = Nan::New<External>((void *)(intptr_t)data->conn->m_hENV);
  info[1] = Nan::New<External>((void *)(intptr_t)data->conn->m_hDBC);
  info[2] = Nan::New<External>((void *)(intptr_t)data->hSTMT);

  Local<Object> js_result = Nan::NewInstance(Nan::New(ODBCStatement::constructor), 3, info).ToLocalChecked();

  info[0] = Nan::Null();
  info[1] = js_result;

  Nan::TryCatch try_catch;

  data->cb->Call(2, info);

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  data->conn->Unref();
  delete data->cb;

  free(data);
  free(req);
  DEBUG_PRINTF("ODBCConnection::UV_AfterCreateStatement - Exit\n");
}

/*
 * Query
 */

NAN_METHOD(ODBCConnection::Query)
{
  DEBUG_PRINTF("ODBCConnection::Query - Entry\n");
  Nan::HandleScope scope;

  Local<Function> cb;
  int len = 0;
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;
  query_work_data *data = NULL;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  data = (query_work_data *)calloc(1, sizeof(query_work_data));
  MEMCHECK2(data, errmsg);

  data->sql = NULL;

  // Check arguments for different variations of calling this function
  if (info.Length() == 3)
  {
    // handle Query("sql string", [params], function cb () {});

    if (!info[0]->IsString())
    {
      free(data);
      return Nan::ThrowTypeError("Argument 0 must be an String.");
    }
    else if (!info[1]->IsArray())
    {
      free(data);
      return Nan::ThrowTypeError("Argument 1 must be an Array.");
    }
    else if (!info[2]->IsFunction())
    {
      free(data);
      return Nan::ThrowTypeError("Argument 2 must be a Function.");
    }

    Nan::Utf8String sql(info[0]);
    len = sql.length();
    GETCPPSTR2(data->sql, sql, len, errmsg);

    data->params = ODBC::GetParametersFromArray(
        Local<Array>::Cast(info[1]),
        &data->paramCount);

    cb = Local<Function>::Cast(info[2]);
  }
  else if (info.Length() == 2)
  {
    // handle either Query("sql", cb) or Query({ settings }, cb)

    if (!info[1]->IsFunction())
    {
      free(data);
      return Nan::ThrowTypeError("ODBCConnection::Query(): Argument 1 must be a Function.");
    }

    cb = Local<Function>::Cast(info[1]);

    if (info[0]->IsString())
    {
      // handle Query("sql", function cb () {})
      Nan::Utf8String sql2(info[0]);
      len = sql2.length();
      GETCPPSTR2(data->sql, sql2, len, errmsg);
      data->paramCount = 0;
    }
    else if (info[0]->IsObject())
    {
      // NOTE: going forward this is the way we should expand options
      // rather than adding more arguments to the function signature.
      // specify options on an options object.
      // handle Query({}, function cb () {});

      Local<Object> obj = Nan::To<v8::Object>(info[0]).ToLocalChecked();

      Local<String> optionSqlKey = Nan::New(OPTION_SQL);
      if (Nan::HasOwnProperty(obj, optionSqlKey).IsJust() && Nan::Get(obj, optionSqlKey).ToLocalChecked()->IsString())
      {
        Nan::Utf8String sql3(Nan::Get(obj, optionSqlKey).ToLocalChecked());
        len = sql3.length();
        GETCPPSTR2(data->sql, sql3, len, errmsg);
      }

      Nan::TryCatch try_catch;

      Local<String> optionParamsKey = Nan::New(OPTION_PARAMS);
      if (Nan::HasOwnProperty(obj, optionParamsKey).IsJust() && Nan::Get(obj, optionParamsKey).ToLocalChecked()->IsArray())
      {
        data->params = ODBC::GetParametersFromArray(
            Local<Array>::Cast(Nan::Get(obj, optionParamsKey).ToLocalChecked()),
            &data->paramCount);
      }
      else
      {
        data->paramCount = 0;
      }

      if (try_catch.HasCaught())
      {
        free(data);
        try_catch.ReThrow();
        return;
      }

      Local<String> optionNoResultsKey = Nan::New(OPTION_NORESULTS);
      if (Nan::HasOwnProperty(obj, optionNoResultsKey).IsJust() && Nan::Get(obj, optionNoResultsKey).ToLocalChecked()->IsBoolean())
      {
        data->noResultObject = Nan::To<bool>(Nan::Get(obj, optionNoResultsKey).ToLocalChecked()).FromJust();
      }
      else
      {
        data->noResultObject = false;
      }

      Local<String> optionArraySize = Nan::New(OPTION_ARRAYSIZE);
      if (Nan::HasOwnProperty(obj, optionArraySize).IsJust() && Nan::Get(obj, optionArraySize).ToLocalChecked()->IsInt32())
      {
        data->arraySize = Nan::To<int32_t>(Nan::Get(obj, optionArraySize).ToLocalChecked()).FromJust();
      }
      else
      {
        data->arraySize = 0;
      }
    }
    else
    {
      free(data);
      return Nan::ThrowTypeError("ODBCConnection::Query(): Argument 0 must be a String or an Object.");
    }
  }
  else
  {
    free(data);
    return Nan::ThrowTypeError("ODBCConnection::Query(): Requires either 2 or 3 Arguments. ");
  }
  if (len <= 0)
  {
    free(data);
    return Nan::ThrowTypeError("ODBCConnection::Query(): SQL statement is missing.");
  }
  // Done checking arguments

  data->sqlLen = len;
  data->sqlSize = (len + 1) * sizeof(UNICHAR);

  DEBUG_PRINTF("ODBCConnection::Query: sqlLen=%i, sqlSize=%i,sql=%s, hDBC=%X\n",
               data->sqlLen, data->sqlSize, (char *)data->sql, conn->m_hDBC);

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK2(work_req, errmsg);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_Query,
      (uv_after_work_cb)UV_AfterQuery);

  conn->Ref();

  DEBUG_PRINTF("ODBCConnection::Query - Exit for hDBC=%X\n", data->conn->m_hDBC);

exit:
  if (errmsg)
  {
    if (data)
    {
      free(data->sql);
      free(data);
    }
    free(work_req);
    return Nan::ThrowError(errmsg);
  }
  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCConnection::UV_Query(uv_work_t *req)
{
  query_work_data *data = (query_work_data *)(req->data);
  SQLRETURN ret;
  DEBUG_PRINTF("ODBCConnection::UV_Query - Entry: hDBC=%X\n", data->conn->m_hDBC);

  // allocate a new statment handle
  SQLAllocHandle(SQL_HANDLE_STMT,
                 data->conn->m_hDBC,
                 &data->hSTMT);

  // check to see if should excute a direct or a parameter bound query
  if (!data->paramCount)
  {
    // execute the query directly
    ret = SQLExecDirect(
        data->hSTMT,
        (SQLTCHAR *)data->sql,
        data->sqlLen);
  }
  else
  {
    // prepare statement, bind parameters and execute statement
    ret = SQLPrepare(
        data->hSTMT,
        (SQLTCHAR *)data->sql,
        data->sqlLen);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
    {
      if (data->arraySize > 0)
      { // Its array insert
        ret = SQLSetStmtAttr(data->hSTMT, SQL_ATTR_PARAM_BIND_TYPE,
                             SQL_PARAM_BIND_BY_COLUMN, 0);
        ret = SQLSetStmtAttr(data->hSTMT, SQL_ATTR_PARAMSET_SIZE,
                             (SQLPOINTER)data->arraySize, 0);
      }

      ret = ODBC::BindParameters(data->hSTMT, data->params, data->paramCount);

      if (SQL_SUCCEEDED(ret))
      {
        ret = SQLExecute(data->hSTMT);
      }
    }
  }

  // this will be checked later in UV_AfterQuery
  data->result = ret;
  DEBUG_PRINTF("ODBCConnection::UV_Query - Exit: hDBC=%X\n", data->conn->m_hDBC);
}

void ODBCConnection::UV_AfterQuery(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterQuery - Entry\n");

  Nan::HandleScope scope;

  query_work_data *data = (query_work_data *)(req->data);
  Local<Array> sp_result = Nan::New<Array>();
  int outParamCount = 0; // Non-zero tells its a SP with OUT param

  Nan::TryCatch try_catch;

  DEBUG_PRINTF("ODBCConnection::UV_AfterQuery : data->result=%i, data->noResultObject=%i, stmt=%X\n",
               data->result, data->noResultObject, data->hSTMT);

  // Retrieve values of INOUT and OUTPUT Parameters of Stored Procedure
  if (SQL_SUCCEEDED(data->result))
  {
    for (int i = 0; i < data->paramCount; i++)
    {
      if (data->params[i].paramtype % 2 == 0)
      {
        Nan::Set(sp_result, Nan::New(outParamCount), ODBC::GetOutputParameter(&data->params[i]));
        outParamCount++;
      }
    }
    DEBUG_PRINTF("ODBCConnection::UV_AfterQuery : outParamCount=%i\n", outParamCount);
  }
  if ((data->result >= SQL_SUCCESS) && (data->noResultObject))
  {
    // We have been requested to not create a result object
    // this means we should release the handle now and call back
    // with Nan::True()

    SQLFreeHandle(SQL_HANDLE_STMT, data->hSTMT);
    data->hSTMT = (SQLHSTMT)NULL;

    Local<Value> info[2];
    info[0] = Nan::Null();
    if (outParamCount)
      info[1] = sp_result;
    else
      info[1] = Nan::Null();

    data->cb->Call(2, info);
  }
  else
  {
    Local<Value> info[3];
    Local<Value> js_info[4];
    bool *canFreeHandle = new bool(true);

    js_info[0] = Nan::New<External>((void *)(intptr_t)data->conn->m_hENV);
    js_info[1] = Nan::New<External>((void *)(intptr_t)data->conn->m_hDBC);
    js_info[2] = Nan::New<External>((void *)(intptr_t)data->hSTMT);
    js_info[3] = Nan::New<External>((void *)canFreeHandle);

    // Ingnore SQL_NO_DATA_FOUND warning, fix for issue 573
    if (data->result == SQL_NO_DATA_FOUND)
      data->result = SQL_SUCCESS;

    // Check now to see if there was an error (as there may be further result sets)
    if (data->result != SQL_SUCCESS)
    {
      info[0] = ODBC::GetSQLError(SQL_HANDLE_STMT, data->hSTMT, (char *)"[ibm_db] SQL_ERROR");
      info[1] = Nan::Null();
      SQLFreeHandle(SQL_HANDLE_STMT, data->hSTMT);
      data->hSTMT = (SQLHSTMT)NULL;
    }
    else
    {
      info[0] = Nan::Null();
      Local<Object> js_result = Nan::NewInstance(Nan::New(ODBCResult::constructor), 4, js_info).ToLocalChecked();
      info[1] = js_result;
    }

    if (outParamCount)
      info[2] = sp_result; // Must a CALL stmt
    else
      info[2] = Nan::Null();

    data->cb->Call(3, info);
  }

  data->conn->Unref();

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  delete data->cb;

  if (data->paramCount)
  {
    FREE_PARAMS(data->params, data->paramCount);
  }

  FREE(data->sql);
  FREE(data->catalog);
  FREE(data->schema);
  FREE(data->table);
  FREE(data->type);
  FREE(data->column);
  FREE(data);
  FREE(req);

  // scope.Close(Undefined());
  DEBUG_PRINTF("ODBCConnection::UV_AfterQuery - Exit\n");
}

/*
 * QuerySync
 */

NAN_METHOD(ODBCConnection::QuerySync)
{
  DEBUG_PRINTF("ODBCConnection::QuerySync - Entry\n");
  Nan::HandleScope scope;
  void *sql = NULL;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  Parameter *params = NULL;
  SQLRETURN ret;
  SQLHSTMT hSTMT;
  int paramCount = 0;
  int outParamCount = 0; // Non-zero tells its a SP.
  Local<Array> sp_result = Nan::New<Array>();
  bool noResultObject = false;
  int32_t arraySize = 0;
  int len = 0;

  // Check arguments for different variations of calling this function
  if (info.Length() == 2)
  {
    if (!info[0]->IsString())
    {
      return Nan::ThrowTypeError("ODBCConnection::QuerySync(): Argument 0 must be an String.");
    }
    else if (!info[1]->IsArray())
    {
      return Nan::ThrowTypeError("ODBCConnection::QuerySync(): Argument 1 must be an Array.");
    }

    Nan::Utf8String sql1(info[0]);
    len = sql1.length();
    GETCPPSTR(sql, sql1, len);

    params = ODBC::GetParametersFromArray(
        Local<Array>::Cast(info[1]),
        &paramCount);
  }
  else if (info.Length() == 1)
  {
    // handle either QuerySync("sql") or QuerySync({ settings })

    if (info[0]->IsString())
    {
      // handle Query("sql")
      Nan::Utf8String sql2(info[0]);
      len = sql2.length();
      GETCPPSTR(sql, sql2, len);

      paramCount = 0;
    }
    else if (info[0]->IsObject())
    {
      // NOTE: going forward this is the way we should expand options
      // rather than adding more arguments to the function signature.
      // specify options on an options object.
      // handle Query({}, function cb () {});

      Local<Object> obj = Nan::To<v8::Object>(info[0]).ToLocalChecked();

      Local<String> optionSqlKey = Nan::New<String>(OPTION_SQL);
      if (Nan::HasOwnProperty(obj, optionSqlKey).IsJust() && Nan::Get(obj, optionSqlKey).ToLocalChecked()->IsString())
      {
        Nan::Utf8String sql3(Nan::Get(obj, optionSqlKey).ToLocalChecked());
        len = sql3.length();
        GETCPPSTR(sql, sql3, len);
      }

      Local<String> optionParamsKey = Nan::New(OPTION_PARAMS);
      if (Nan::HasOwnProperty(obj, optionParamsKey).IsJust() && Nan::Get(obj, optionParamsKey).ToLocalChecked()->IsArray())
      {
        params = ODBC::GetParametersFromArray(
            Local<Array>::Cast(Nan::Get(obj, optionParamsKey).ToLocalChecked()),
            &paramCount);
      }
      else
      {
        paramCount = 0;
      }

      Local<String> optionNoResultsKey = Nan::New(OPTION_NORESULTS);
      if (Nan::HasOwnProperty(obj, optionNoResultsKey).IsJust() && Nan::Get(obj, optionNoResultsKey).ToLocalChecked()->IsBoolean())
      {
        noResultObject = Nan::To<bool>(Nan::Get(obj, optionNoResultsKey).ToLocalChecked()).FromJust();
        DEBUG_PRINTF("ODBCConnection::QuerySync - under if noResultObject=%i\n", noResultObject);
      }
      else
      {
        noResultObject = false;
      }

      Local<String> optionArraySize = Nan::New(OPTION_ARRAYSIZE);
      if (Nan::HasOwnProperty(obj, optionArraySize).IsJust() && Nan::Get(obj, optionArraySize).ToLocalChecked()->IsInt32())
      {
        arraySize = Nan::To<int32_t>(Nan::Get(obj, optionArraySize).ToLocalChecked()).FromJust();
        DEBUG_PRINTF("ODBCConnection::QuerySync - under if arraySize =%i\n", arraySize);
      }
      else
      {
        arraySize = 0;
      }
    }
    else
    {
      return Nan::ThrowTypeError("ODBCConnection::QuerySync(): Argument 0 must be a String or an Object.");
    }
  }
  else
  {
    return Nan::ThrowTypeError("ODBCConnection::QuerySync(): Requires either 1 or 2 Arguments.");
  }
  if (len <= 0)
  {
    return Nan::ThrowTypeError("ODBCConnection::QuerySync(): SQL statement is missing.");
  }
  // Done checking arguments

  // allocate a new statment handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT,
                       conn->m_hDBC,
                       &hSTMT);

  DEBUG_PRINTF("ODBCConnection::QuerySync - hSTMT=%X, noResultObject=%i, arraySize = %d\n", hSTMT, noResultObject, arraySize);
  // check to see if should excute a direct or a parameter bound query
  if (!SQL_SUCCEEDED(ret))
  {
    // We'll check again later
  }
  else if (!paramCount)
  {
    // execute the query directly
    ret = SQLExecDirect(
        hSTMT,
        (SQLTCHAR *)sql,
        len);
  }
  else
  {
    // prepare statement, bind parameters and execute statement
    ret = SQLPrepare(
        hSTMT,
        (SQLTCHAR *)sql,
        len);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
    {
      if (arraySize > 0)
      { // Its array insert
        ret = SQLSetStmtAttr(hSTMT, SQL_ATTR_PARAM_BIND_TYPE,
                             SQL_PARAM_BIND_BY_COLUMN, 0);
        ret = SQLSetStmtAttr(hSTMT, SQL_ATTR_PARAMSET_SIZE,
                             (SQLPOINTER)arraySize, 0);
      }
      ret = ODBC::BindParameters(hSTMT, params, paramCount);
      if (SQL_SUCCEEDED(ret))
      {
        ret = SQLExecute(hSTMT);
        if (SQL_SUCCEEDED(ret))
        {
          for (int i = 0; i < paramCount; i++)
          { // For stored Procedure CALL
            if (params[i].paramtype % 2 == 0)
            {
              Nan::Set(sp_result, Nan::New(outParamCount), ODBC::GetOutputParameter(&params[i]));
              outParamCount++;
            }
          }
        }
      }
    }
  }

  FREE(sql);
  FREE_PARAMS(params, paramCount);

  // Ingnore SQL_NO_DATA_FOUND warning, fix for issue 573
  if (ret == SQL_NO_DATA_FOUND)
    ret = SQL_SUCCESS;

  // check to see if there was an error during execution
  if (ret != SQL_SUCCESS)
  {
    // Free stmt handle and then throw error.
    Local<Value> err = ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        hSTMT,
        (char *)"[node-ibm_db] Error in ODBCConnection::QuerySync while executing query.");
    SQLFreeHandle(SQL_HANDLE_STMT, hSTMT);
    hSTMT = (SQLHSTMT)NULL;
    Nan::ThrowError(err);
    DEBUG_PRINTF("ODBCConnection::QuerySync - Exit\n");
    return;
  }
  else if (noResultObject)
  {
    // if there is not result object requested then
    // we must destroy the STMT ourselves.
    SQLFreeHandle(SQL_HANDLE_STMT, hSTMT);
    hSTMT = (SQLHSTMT)NULL;

    if (outParamCount) // Its a CALL stmt with OUT params.
    {                  // Return an array with outparams as second element.
      Local<Array> resultset = Nan::New<Array>();
      Nan::Set(resultset, 0, Nan::Null());
      Nan::Set(resultset, 1, sp_result);
      info.GetReturnValue().Set(resultset);
    }
    else
    {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
  else
  {
    Local<Value> result[4];
    bool *canFreeHandle = new bool(true);

    result[0] = Nan::New<External>((void *)(intptr_t)conn->m_hENV);
    result[1] = Nan::New<External>((void *)(intptr_t)conn->m_hDBC);
    result[2] = Nan::New<External>((void *)(intptr_t)hSTMT);
    result[3] = Nan::New<External>((void *)canFreeHandle);

    Local<Object> js_result = Nan::NewInstance(Nan::New(ODBCResult::constructor), 4, result).ToLocalChecked();

    if (outParamCount) // Its a CALL stmt with OUT params.
    {                  // Return an array with outparams as second element. [result, outparams]
      Local<Array> resultset = Nan::New<Array>();
      Nan::Set(resultset, 0, js_result);
      Nan::Set(resultset, 1, sp_result);
      info.GetReturnValue().Set(resultset);
    }
    else
    {
      info.GetReturnValue().Set(js_result);
    }
  }
  DEBUG_PRINTF("ODBCConnection::QuerySync - Exit\n");
}

/*
 * Tables
 */

NAN_METHOD(ODBCConnection::Tables)
{
  DEBUG_PRINTF("ODBCConnection::Tables - Entry\n");
  Nan::HandleScope scope;
  int len = 0;
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;
  query_work_data *data = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, table);
  REQ_STRO_OR_NULL_ARG(3, type);
  Local<Function> cb = Local<Function>::Cast(info[4]);

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK2(work_req, errmsg);

  data = (query_work_data *)calloc(1, sizeof(query_work_data));
  MEMCHECK2(data, errmsg);

  data->sql = NULL;
  data->catalog = NULL;
  data->schema = NULL;
  data->table = NULL;
  data->type = NULL;
  data->column = NULL;

  len = catalog.length();
  GETCPPSTR2(data->catalog, catalog, len, errmsg);

  len = schema.length();
  GETCPPSTR2(data->schema, schema, len, errmsg);

  len = table.length();
  GETCPPSTR2(data->table, table, len, errmsg);

  len = type.length();
  GETCPPSTR2(data->type, type, len, errmsg);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_Tables,
      (uv_after_work_cb)UV_AfterQuery);

  conn->Ref();

  DEBUG_PRINTF("ODBCConnection::Tables - Exit, catalog=%s, schema=%s, table=%s,type=%s\n", data->catalog, data->schema, data->table, data->type);

exit:
  if (errmsg)
  {
    DEBUG_PRINTF("ODBCConnection::Tables - Error\n");
    if (data)
    {
      FREE(data->catalog);
      FREE(data->schema);
      FREE(data->table);
      FREE(data->type);
      free(data);
    }
    free(work_req);
    return Nan::ThrowError(errmsg);
  }
  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCConnection::UV_Tables(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_Tables - Entry\n");
  query_work_data *data = (query_work_data *)(req->data);

  SQLAllocHandle(SQL_HANDLE_STMT, data->conn->m_hDBC, &data->hSTMT);

  SQLRETURN ret = SQLTables(
      data->hSTMT,
#ifdef __MVS__
      // ODBC on z/OS does not support three-part naming on catalog APIs.
      // For catalog APIs, the catalog qualifier (szTableQualifier) and
      // its length (cbTableQualifier) must be set to NULL and 0, respectively.
      NULL, 0,
#else
      (SQLTCHAR *)data->catalog, SQL_NTS,
#endif
      (SQLTCHAR *)data->schema, SQL_NTS,
      (SQLTCHAR *)data->table, SQL_NTS,
      (SQLTCHAR *)data->type, SQL_NTS);

  // this will be checked later in UV_AfterQuery
  data->result = ret;
  FREE(data->catalog);
  FREE(data->schema);
  FREE(data->table);
  FREE(data->type);
  DEBUG_PRINTF("ODBCConnection::UV_Tables - Exit\n");
}

/*
 * Columns
 */

NAN_METHOD(ODBCConnection::Columns)
{
  DEBUG_PRINTF("ODBCConnection::Columns - Entry\n");
  Nan::HandleScope scope;
  int len = 0;
  const char *errmsg = NULL;
  uv_work_t *work_req = NULL;
  query_work_data *data = NULL;

  REQ_STRO_OR_NULL_ARG(0, catalog);
  REQ_STRO_OR_NULL_ARG(1, schema);
  REQ_STRO_OR_NULL_ARG(2, table);
  REQ_STRO_OR_NULL_ARG(3, column);

  Local<Function> cb = Local<Function>::Cast(info[4]);

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  data = (query_work_data *)calloc(1, sizeof(query_work_data));
  MEMCHECK2(data, errmsg);

  data->sql = NULL;
  data->catalog = NULL;
  data->schema = NULL;
  data->table = NULL;
  data->type = NULL;
  data->column = NULL;

  len = catalog.length();
  GETCPPSTR2(data->catalog, catalog, len, errmsg);

  len = schema.length();
  GETCPPSTR2(data->schema, schema, len, errmsg);

  len = table.length();
  GETCPPSTR2(data->table, table, len, errmsg);

  len = column.length();
  GETCPPSTR2(data->column, column, len, errmsg);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_Columns,
      (uv_after_work_cb)UV_AfterQuery);

  conn->Ref();
  DEBUG_PRINTF("ODBCConnection::Columns - Exit\n");

exit:
  if (errmsg)
  {
    DEBUG_PRINTF("ODBCConnection::Columns - Error\n");
    if (data)
    {
      FREE(data->catalog);
      FREE(data->schema);
      FREE(data->table);
      FREE(data->column);
      free(data);
    }
    free(work_req);
    return Nan::ThrowError(errmsg);
  }
  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCConnection::UV_Columns(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_Columns - Entry\n");
  query_work_data *data = (query_work_data *)(req->data);

  SQLAllocHandle(SQL_HANDLE_STMT, data->conn->m_hDBC, &data->hSTMT);

  SQLRETURN ret = SQLColumns(
      data->hSTMT,
#ifdef __MVS__
      // ODBC on z/OS does not support three-part naming on catalog APIs.
      // For catalog APIs, the catalog qualifier (szTableQualifier) and
      // its length (cbTableQualifier) must be set to NULL and 0, respectively.
      NULL, 0,
#else
      (SQLTCHAR *)data->catalog, SQL_NTS,
#endif
      (SQLTCHAR *)data->schema, SQL_NTS,
      (SQLTCHAR *)data->table, SQL_NTS,
      (SQLTCHAR *)data->column, SQL_NTS);

  // this will be checked later in UV_AfterQuery
  data->result = ret;
  FREE(data->catalog);
  FREE(data->schema);
  FREE(data->table);
  FREE(data->column);
  DEBUG_PRINTF("ODBCConnection::UV_Columns - Exit\n");
}

/*
 * BeginTransactionSync
 *
 */

NAN_METHOD(ODBCConnection::BeginTransactionSync)
{
  DEBUG_PRINTF("ODBCConnection::BeginTransactionSync - Entry\n");
  Nan::HandleScope scope;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  SQLRETURN ret;

  // set the connection manual commits
  ret = SQLSetConnectAttr(
      conn->m_hDBC,
      SQL_ATTR_AUTOCOMMIT,
      (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
      SQL_NTS);

  if (!SQL_SUCCEEDED(ret))
  {
    Local<Value> objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->m_hDBC);

    Nan::ThrowError(objError);

    info.GetReturnValue().Set(Nan::False());
  }

  info.GetReturnValue().Set(Nan::True());
  DEBUG_PRINTF("ODBCConnection::BeginTransactionSync - Exit\n");
}

/*
 * BeginTransaction
 *
 */

NAN_METHOD(ODBCConnection::BeginTransaction)
{
  DEBUG_PRINTF("ODBCConnection::BeginTransaction\n");
  Nan::HandleScope scope;

  REQ_FUN_ARG(0, cb);

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  query_work_data *data =
      (query_work_data *)calloc(1, sizeof(query_work_data));
  if (!data)
    free(work_req); // Below MEMCHECK macro will log error and return;
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_BeginTransaction,
      (uv_after_work_cb)UV_AfterBeginTransaction);

  return;
}

/*
 * UV_BeginTransaction
 *
 */

void ODBCConnection::UV_BeginTransaction(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_BeginTransaction\n");

  query_work_data *data = (query_work_data *)(req->data);

  // set the connection manual commits
  data->result = SQLSetConnectAttr(
      data->conn->self()->m_hDBC,
      SQL_ATTR_AUTOCOMMIT,
      (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
      SQL_NTS);
}

/*
 * UV_AfterBeginTransaction
 *
 */

void ODBCConnection::UV_AfterBeginTransaction(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterBeginTransaction\n");
  Nan::HandleScope scope;

  // TODO: Is this supposed to be of type query_work_data?
  open_connection_work_data *data = (open_connection_work_data *)(req->data);

  Local<Value> argv[1];

  bool err = false;

  if (!SQL_SUCCEEDED(data->result))
  {
    err = true;

    Local<Value> objError = ODBC::GetSQLError(SQL_HANDLE_DBC, data->conn->self()->m_hDBC);

    argv[0] = objError;
  }

  Nan::TryCatch try_catch;

  data->cb->Call(err ? 1 : 0, argv);

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  delete data->cb;

  free(data);
  free(req);
}

/*
 * EndTransactionSync
 *
 */

NAN_METHOD(ODBCConnection::EndTransactionSync)
{
  DEBUG_PRINTF("ODBCConnection::EndTransactionSync - Entry\n");
  Nan::HandleScope scope;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  REQ_BOOL_ARG(0, rollback);

  Local<Value> objError;
  SQLRETURN ret;
  bool error = false;
  SQLSMALLINT completionType = (rollback->Value())
                                   ? SQL_ROLLBACK
                                   : SQL_COMMIT;

  // Call SQLEndTran
  ret = SQLEndTran(
      SQL_HANDLE_DBC,
      conn->m_hDBC,
      completionType);

  // check how the transaction went
  if (!SQL_SUCCEEDED(ret))
  {
    error = true;

    objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->m_hDBC);
  }

  // Reset the connection back to autocommit
  ret = SQLSetConnectAttr(
      conn->m_hDBC,
      SQL_ATTR_AUTOCOMMIT,
      (SQLPOINTER)SQL_AUTOCOMMIT_ON,
      SQL_NTS);

  // check how setting the connection attr went
  // but only process the code if an error has not already
  // occurred. If an error occurred during SQLEndTran,
  // that is the error that we want to throw.
  if (!SQL_SUCCEEDED(ret) && !error)
  {
    // TODO: if this also failed, we really should
    // be restarting the connection or something to deal with this state
    error = true;

    objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->m_hDBC);
  }

  if (error)
  {
    Nan::ThrowError(objError);

    info.GetReturnValue().Set(Nan::False());
  }
  else
  {
    info.GetReturnValue().Set(Nan::True());
  }
  DEBUG_PRINTF("ODBCConnection::EndTransactionSync - Exit\n");
}

/*
 * EndTransaction
 *
 */

NAN_METHOD(ODBCConnection::EndTransaction)
{
  DEBUG_PRINTF("ODBCConnection::EndTransaction\n");
  Nan::HandleScope scope;

  REQ_BOOL_ARG(0, rollback);
  REQ_FUN_ARG(1, cb);

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  query_work_data *data =
      (query_work_data *)calloc(1, sizeof(query_work_data));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  data->completionType = (rollback->Value())
                             ? SQL_ROLLBACK
                             : SQL_COMMIT;
  data->cb = new Nan::Callback(cb);
  data->conn = conn;
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_EndTransaction,
      (uv_after_work_cb)UV_AfterEndTransaction);

  info.GetReturnValue().Set(Nan::Undefined());
}

/*
 * UV_EndTransaction
 *
 */

void ODBCConnection::UV_EndTransaction(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_EndTransaction\n");

  query_work_data *data = (query_work_data *)(req->data);

  bool err = false;

  // Call SQLEndTran
  SQLRETURN ret = SQLEndTran(
      SQL_HANDLE_DBC,
      data->conn->m_hDBC,
      data->completionType);

  data->result = ret;

  if (!SQL_SUCCEEDED(ret))
  {
    err = true;
  }

  // Reset the connection back to autocommit
  ret = SQLSetConnectAttr(
      data->conn->m_hDBC,
      SQL_ATTR_AUTOCOMMIT,
      (SQLPOINTER)SQL_AUTOCOMMIT_ON,
      SQL_NTS);

  if (!SQL_SUCCEEDED(ret) && !err)
  {
    // there was not an earlier error,
    // so we shall pass the return code from
    // this last call.
    data->result = ret;
  }
}

/*
 * UV_AfterEndTransaction
 *
 */

void ODBCConnection::UV_AfterEndTransaction(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterEndTransaction\n");
  Nan::HandleScope scope;

  open_connection_work_data *data = (open_connection_work_data *)(req->data);

  Local<Value> argv[1];

  bool err = false;

  if (!SQL_SUCCEEDED(data->result))
  {
    err = true;

    Local<Value> objError = ODBC::GetSQLError(SQL_HANDLE_DBC, data->conn->self()->m_hDBC);

    argv[0] = objError;
  }

  Nan::TryCatch try_catch;

  data->cb->Call(err ? 1 : 0, argv);

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  delete data->cb;

  free(data);
  free(req);
}

/*
 * SetIsolationLevel
 *
 */

NAN_METHOD(ODBCConnection::SetIsolationLevel)
{
  DEBUG_PRINTF("ODBCConnection::SetIsolationLevel - Entry\n");
  Nan::HandleScope scope;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  Local<Value> objError;
  SQLRETURN ret = SQL_SUCCESS;
  bool error = false;
  SQLUINTEGER isolationLevel = SQL_TXN_READ_COMMITTED;

  if (info.Length() <= 0)
  {
    isolationLevel = SQL_TXN_READ_COMMITTED;
  }
  else if (info[0]->IsInt32())
  {
    isolationLevel = Nan::To<v8::Int32>(info[0]).ToLocalChecked()->Value();
  }
  else
  {
    return Nan::ThrowTypeError("Argument #0 must be an integer.");
  }

  // set the connection manual commits
  ret = SQLSetConnectAttr(
      conn->m_hDBC,
      SQL_ATTR_TXN_ISOLATION,
      (SQLPOINTER)(intptr_t)isolationLevel,
      SQL_NTS);

  DEBUG_PRINTF("ODBCConnection::SetIsolationLevel isolationLevel=%i; ret=%d\n",
               isolationLevel, ret);

  // check how the transaction went
  if (!SQL_SUCCEEDED(ret))
  {
    error = true;

    objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->m_hDBC);
  }

  if (error)
  {
    Nan::ThrowError(objError);

    info.GetReturnValue().Set(Nan::False());
  }
  else
  {
    info.GetReturnValue().Set(Nan::True());
  }
  DEBUG_PRINTF("ODBCConnection::SetIsolationLevel - Exit\n");
}

/*
 * GetInfoSync
 */

NAN_METHOD(ODBCConnection::GetInfoSync)
{
  DEBUG_PRINTF("ODBCConnection::GetInfoSync - Entry\n");
  Nan::HandleScope scope;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  Local<Value> objError;
  SQLRETURN ret = SQL_SUCCESS;
  bool error = false;
  SQLPOINTER rgbInfo = (SQLPOINTER)NULL;
  SQLSMALLINT cbInfo = 0;

  REQ_ARGS(2);
  REQ_INT_ARG(0, infotype);
  REQ_INT_ARG(1, infolen);

  rgbInfo = malloc(infolen);
  MEMCHECK(rgbInfo);
  ret = SQLGetInfo(
      conn->m_hDBC,
      infotype,
      rgbInfo,
      infolen,
      &cbInfo);

  DEBUG_PRINTF("ODBCConnection::GetInfoSync infoType=%i; infoLen=%i, ret=%d\n",
               infotype, infolen, ret);

  if (!SQL_SUCCEEDED(ret))
  {
    error = true;

    objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->m_hDBC);
  }

  if (error)
  {
    Nan::ThrowError(objError);

    info.GetReturnValue().Set(Nan::False());
  }
  else
  {
    Local<Value> value = getInfoValue((SQLUSMALLINT)infotype, rgbInfo);
    info.GetReturnValue().Set(value);
  }
  DEBUG_PRINTF("ODBCConnection::GetInfoSync - Exit\n");
}

/*
 * GetInfo
 */

NAN_METHOD(ODBCConnection::GetInfo)
{
  DEBUG_PRINTF("ODBCConnection::GetInfo - Entry\n");
  Nan::HandleScope scope;

  REQ_ARGS(3);
  REQ_INT_ARG(0, infotype);
  REQ_INT_ARG(1, infolen);
  REQ_FUN_ARG(2, cb);

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  getinfo_work_data *data =
      (getinfo_work_data *)calloc(1, sizeof(getinfo_work_data));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;
  data->infoType = infotype;
  data->buffLen = infolen;

  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_GetInfo,
      (uv_after_work_cb)UV_AfterGetInfo);

  conn->Ref();

  DEBUG_PRINTF("ODBCConnection::GetInfo - Exit\n");
  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCConnection::UV_GetInfo(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_GetInfo - Entry\n");

  // get our work data
  getinfo_work_data *data = (getinfo_work_data *)(req->data);
  SQLSMALLINT valueLen = 0;

  DEBUG_PRINTF("ODBCConnection::UV_GetInfo m_hDBC=%X, infoType=%d, infoLen=%d\n",
               data->conn->m_hDBC, data->infoType, data->buffLen);

  data->buffer = malloc(data->buffLen);
  MEMCHECK(data->buffer);

  data->rc = SQLGetInfo(
      data->conn->m_hDBC,
      data->infoType,
      data->buffer,
      data->buffLen,
      &valueLen);

  data->valueLen = valueLen;
  DEBUG_PRINTF("ODBCConnection::UV_GetInfo - Exit: infoValuePtr=%s, valueLen=%d, rc=%d\n",
               data->buffer, valueLen, data->rc);
}

void ODBCConnection::UV_AfterGetInfo(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterGetInfo - Entry\n");
  Nan::HandleScope scope;
  Local<Value> info[2];

  getinfo_work_data *data = (getinfo_work_data *)(req->data);

  if (data->rc != SQL_SUCCESS)
  {
    info[0] = ODBC::GetSQLError(SQL_HANDLE_DBC, data->conn->m_hDBC, (char *)"[node-ibm_db] SQL_ERROR");
    info[1] = Nan::Null();
  }
  else
  {
    info[0] = Nan::Null();
    info[1] = getInfoValue(data->infoType, data->buffer);
  }

  Nan::TryCatch try_catch;

  data->cb->Call(2, info);

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  delete data->cb;

  free(data->buffer);
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCConnection::UV_AfterGetInfo - Exit\n");
}

Local<Value> getInfoValue(SQLUSMALLINT fInfoType, SQLPOINTER info)
{
  Local<Value> result;

  if (info == NULL)
  {
    return Nan::Null();
  }

  switch ((unsigned int)fInfoType)
  {
  // null terminated string
  case SQL_DATA_SOURCE_NAME:
  case SQL_DRIVER_NAME:
  case SQL_DRIVER_VER:
#if !defined(__MVS__)
  case SQL_DRIVER_BLDLEVEL:
  case SQL_DB2_DRIVER_VER:
#endif
  case SQL_ODBC_VER:
  case SQL_ROW_UPDATES:
  case SQL_SERVER_NAME:
  case SQL_SEARCH_PATTERN_ESCAPE:
  case SQL_DATABASE_NAME:
  case SQL_DBMS_NAME:
  case SQL_XOPEN_CLI_YEAR:
  case SQL_DBMS_VER:
  case SQL_DBMS_FUNCTIONLVL:
  case SQL_ACCESSIBLE_TABLES:
  case SQL_ACCESSIBLE_PROCEDURES:
  case SQL_DATA_SOURCE_READ_ONLY:
  case SQL_EXPRESSIONS_IN_ORDERBY:
  case SQL_IDENTIFIER_QUOTE_CHAR:
  case SQL_MULT_RESULT_SETS:
  case SQL_MULTIPLE_ACTIVE_TXN:
  case SQL_OUTER_JOINS:
  case SQL_SCHEMA_TERM:
  case SQL_PROCEDURE_TERM:
  case SQL_TABLE_TERM:
  case SQL_USER_NAME:
  case SQL_PROCEDURES:
  case SQL_INTEGRITY:
  case SQL_DRIVER_ODBC_VER:
  case SQL_COLUMN_ALIAS:
  case SQL_KEYWORDS:
  case SQL_ORDER_BY_COLUMNS_IN_SELECT:
  case SQL_SPECIAL_CHARACTERS:
  case SQL_MAX_ROW_SIZE_INCLUDES_LONG:
  case SQL_NEED_LONG_DATA_LEN:
  case SQL_LIKE_ESCAPE_CLAUSE:
  case SQL_CATALOG_NAME:
  case SQL_CATALOG_NAME_SEPARATOR:
  case SQL_CATALOG_TERM:
  case SQL_COLLATION_SEQ:
  case SQL_DESCRIBE_PARAMETER:
    result = Nan::New<String>((char *)info).ToLocalChecked();
    break;

  // 16 bit integer
  case SQL_MAX_DRIVER_CONNECTIONS:
  case SQL_ACTIVE_ENVIRONMENTS:
  case SQL_MAX_CONCURRENT_ACTIVITIES:
  case SQL_ODBC_API_CONFORMANCE:
  case SQL_ODBC_SAG_CLI_CONFORMANCE:
  case SQL_ODBC_SQL_CONFORMANCE:
  case SQL_CONCAT_NULL_BEHAVIOR:
  case SQL_CURSOR_COMMIT_BEHAVIOR:
  case SQL_CURSOR_ROLLBACK_BEHAVIOR:
  case SQL_IDENTIFIER_CASE:
  case SQL_MAX_COLUMN_NAME_LEN:
  case SQL_MAX_CURSOR_NAME_LEN:
  case SQL_MAX_SCHEMA_NAME_LEN:
#if !defined(__MVS__)
  case SQL_MAX_MODULE_NAME_LEN:
#endif
  case SQL_MAX_PROCEDURE_NAME_LEN:
  case SQL_MAX_TABLE_NAME_LEN:
  case SQL_TXN_CAPABLE:
  case SQL_CORRELATION_NAME:
  case SQL_NON_NULLABLE_COLUMNS:
  case SQL_FILE_USAGE:
  case SQL_NULL_COLLATION:
  case SQL_GROUP_BY:
  case SQL_QUOTED_IDENTIFIER_CASE:
  case SQL_MAX_COLUMNS_IN_GROUP_BY:
  case SQL_MAX_COLUMNS_IN_INDEX:
  case SQL_MAX_COLUMNS_IN_ORDER_BY:
  case SQL_MAX_COLUMNS_IN_SELECT:
  case SQL_MAX_COLUMNS_IN_TABLE:
  case SQL_MAX_TABLES_IN_SELECT:
  case SQL_MAX_USER_NAME_LEN:
  case SQL_CATALOG_LOCATION:
  case SQL_MAX_CATALOG_NAME_LEN:
  case SQL_MAX_IDENTIFIER_LEN:
    result = Nan::New<Number>(*((SQLSMALLINT *)info));
    break;

  // 32 bit integer
  case SQL_DRIVER_HDBC:
  case SQL_DRIVER_HENV:
  case SQL_DRIVER_HSTMT:
  case SQL_DRIVER_HDESC:
  case SQL_DRIVER_HLIB:
  case SQL_MAX_ROW_SIZE:
  case SQL_ASYNC_MODE:
  case SQL_MAX_STATEMENT_LEN:
  case SQL_MAX_CHAR_LITERAL_LEN:
  case SQL_MAX_INDEX_SIZE:
  case SQL_MAX_BINARY_LITERAL_LEN:
  case SQL_CURSOR_SENSITIVITY:
  case SQL_DEFAULT_TXN_ISOLATION:
  case SQL_MAX_ASYNC_CONCURRENT_STATEMENTS:
  case SQL_ODBC_INTERFACE_CONFORMANCE:
  case SQL_PARAM_ARRAY_ROW_COUNTS:
  case SQL_PARAM_ARRAY_SELECTS:
  case SQL_DTC_TRANSITION_COST:
#if !defined(__MVS__)
  case SQL_DATABASE_CODEPAGE:
  case SQL_APPLICATION_CODEPAGE:
  case SQL_CONNECT_CODEPAGE:
  case SQL_DB2_DRIVER_TYPE:
#endif
    result = Nan::New<Number>(*((SQLINTEGER *)info));
    break;

#if !defined(__MVS__)
  case SQL_INPUT_CHAR_CONVFACTOR:
  case SQL_OUTPUT_CHAR_CONVFACTOR:
    result = Nan::New<Number>(*((SQLFLOAT *)info));
    break;
#endif

  // 32 bit binary Mask
  case SQL_CATALOG_USAGE:
  case SQL_FETCH_DIRECTION:
  case SQL_SCROLL_CONCURRENCY:
  case SQL_SCROLL_OPTIONS:
  case SQL_CONVERT_FUNCTIONS:
  case SQL_NUMERIC_FUNCTIONS:
  case SQL_STRING_FUNCTIONS:
  case SQL_SYSTEM_FUNCTIONS:
  case SQL_TIMEDATE_FUNCTIONS:
  case SQL_CONVERT_BIGINT:
  case SQL_CONVERT_INTERVAL_YEAR_MONTH:
  case SQL_CONVERT_INTERVAL_DAY_TIME:
  case SQL_CONVERT_WCHAR:
  case SQL_CONVERT_WVARCHAR:
  case SQL_CONVERT_WLONGVARCHAR:
  case SQL_UNION:
  case SQL_CONVERT_BINARY:
  case SQL_CONVERT_BIT:
  case SQL_CONVERT_CHAR:
  case SQL_CONVERT_DATE:
  case SQL_CONVERT_DECIMAL:
  case SQL_CONVERT_DOUBLE:
  case SQL_CONVERT_FLOAT:
  case SQL_CONVERT_INTEGER:
  case SQL_CONVERT_LONGVARCHAR:
  case SQL_CONVERT_NUMERIC:
  case SQL_CONVERT_REAL:
  case SQL_CONVERT_SMALLINT:
  case SQL_CONVERT_TIME:
  case SQL_CONVERT_TIMESTAMP:
  case SQL_CONVERT_TINYINT:
  case SQL_CONVERT_VARBINARY:
  case SQL_CONVERT_VARCHAR:
  case SQL_CONVERT_LONGVARBINARY:
  case SQL_TXN_ISOLATION_OPTION:
  case SQL_LOCK_TYPES:
  case SQL_POS_OPERATIONS:
  case SQL_POSITIONED_STATEMENTS:
  case SQL_GETDATA_EXTENSIONS:
  case SQL_BOOKMARK_PERSISTENCE:
  case SQL_STATIC_SENSITIVITY:
  case SQL_ALTER_TABLE:
  case SQL_ALTER_DOMAIN:
  case SQL_SCHEMA_USAGE:
#if !defined(__MVS__)
  case SQL_IBM_ALTERTABLEVARCHAR:
  case SQL_MODULE_USAGE:
  case SQL_CREATE_MODULE:
  case SQL_DROP_MODULE:
#endif
  case SQL_SUBQUERIES:
  case SQL_TIMEDATE_ADD_INTERVALS:
  case SQL_TIMEDATE_DIFF_INTERVALS:
  case SQL_OJ_CAPABILITIES:
  case SQL_BATCH_ROW_COUNT:
  case SQL_BATCH_SUPPORT:
  case SQL_CREATE_ASSERTION:
  case SQL_CREATE_CHARACTER_SET:
  case SQL_CREATE_COLLATION:
  case SQL_CREATE_DOMAIN:
  case SQL_CREATE_SCHEMA:
  case SQL_CREATE_TABLE:
  case SQL_CREATE_TRANSLATION:
  case SQL_CREATE_VIEW:
  case SQL_DROP_ASSERTION:
  case SQL_DROP_CHARACTER_SET:
  case SQL_DROP_COLLATION:
  case SQL_DROP_DOMAIN:
  case SQL_DROP_SCHEMA:
  case SQL_DROP_TABLE:
  case SQL_DROP_TRANSLATION:
  case SQL_DROP_VIEW:
  case SQL_DYNAMIC_CURSOR_ATTRIBUTES1:
  case SQL_DYNAMIC_CURSOR_ATTRIBUTES2:
  case SQL_KEYSET_CURSOR_ATTRIBUTES1:
  case SQL_KEYSET_CURSOR_ATTRIBUTES2:
  case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1:
  case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2:
  case SQL_INDEX_KEYWORDS:
  case SQL_INFO_SCHEMA_VIEWS:
  case SQL_SQL92_DATETIME_FUNCTIONS:
  case SQL_SQL92_FOREIGN_KEY_DELETE_RULE:
  case SQL_SQL92_FOREIGN_KEY_UPDATE_RULE:
  case SQL_SQL92_GRANT:
  case SQL_SQL92_NUMERIC_VALUE_FUNCTIONS:
  case SQL_SQL92_PREDICATES:
  case SQL_SQL92_RELATIONAL_JOIN_OPERATORS:
  case SQL_SQL92_REVOKE:
  case SQL_SQL92_ROW_VALUE_CONSTRUCTOR:
  case SQL_SQL92_STRING_FUNCTIONS:
  case SQL_SQL92_VALUE_EXPRESSIONS:
  case SQL_STANDARD_CLI_CONFORMANCE:
  case SQL_STATIC_CURSOR_ATTRIBUTES1:
  case SQL_STATIC_CURSOR_ATTRIBUTES2:
  case SQL_AGGREGATE_FUNCTIONS:
  case SQL_DATETIME_LITERALS:
  case SQL_DDL_INDEX:
  case SQL_INSERT_STATEMENT:
  case SQL_SQL_CONFORMANCE:
  case CLI_INTERNAL_ATTRIBUTES:
    result = Nan::New<Number>(*((SQLINTEGER *)info));
    break;

  default:
    result = Nan::New((char *)info).ToLocalChecked();
    break;
  }
  return result;
}

/*
 * GetTypeInfoSync
 */

NAN_METHOD(ODBCConnection::GetTypeInfoSync)
{
  DEBUG_PRINTF("ODBCConnection::GetTypeInfoSync - Entry\n");
  Nan::HandleScope scope;
  SQLRETURN ret = SQL_SUCCESS;
  SQLHSTMT hSTMT;
  SQLSMALLINT fSqlType = 0;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  REQ_ARGS(1);
  REQ_INT_ARG(0, datatype);
  fSqlType = (SQLSMALLINT)datatype;

  // allocate a new statment handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT,
                       conn->m_hDBC,
                       &hSTMT);

  if (!SQL_SUCCEEDED(ret))
  {
    // We'll check again later
  }
  else
  {
    ret = SQLGetTypeInfo(hSTMT, fSqlType);
  }

  // Ingnore SQL_NO_DATA_FOUND warning, fix for issue 573
  if (ret == SQL_NO_DATA_FOUND)
    ret = SQL_SUCCESS;

  // check to see if there was an error during execution
  if (ret != SQL_SUCCESS)
  {
    // Free stmt handle and then throw error.
    Local<Value> err = ODBC::GetSQLError(
        SQL_HANDLE_STMT,
        hSTMT,
        (char *)"[node-ibm_db] Error in ODBCConnection::GetTypeInfoSync.");
    SQLFreeHandle(SQL_HANDLE_STMT, hSTMT);
    hSTMT = (SQLHSTMT)NULL;
    Nan::ThrowError(err);
  }
  else
  {
    Local<Value> result[4];
    bool *canFreeHandle = new bool(true);

    result[0] = Nan::New<External>((void *)(intptr_t)conn->m_hENV);
    result[1] = Nan::New<External>((void *)(intptr_t)conn->m_hDBC);
    result[2] = Nan::New<External>((void *)(intptr_t)hSTMT);
    result[3] = Nan::New<External>((void *)canFreeHandle);

    Local<Object> js_result = Nan::NewInstance(Nan::New(ODBCResult::constructor), 4, result).ToLocalChecked();

    info.GetReturnValue().Set(js_result);
  }
  DEBUG_PRINTF("ODBCConnection::GetTypeInfoSync - Exit\n");
  return;
}

/*
 * GetTypeInfo
 */

NAN_METHOD(ODBCConnection::GetTypeInfo)
{
  DEBUG_PRINTF("ODBCConnection::GetTypeInfo - Entry\n");
  Nan::HandleScope scope;

  REQ_ARGS(2);
  REQ_INT_ARG(0, datatype);
  REQ_FUN_ARG(1, cb);

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  gettypeinfo_work_data *data =
      (gettypeinfo_work_data *)calloc(1, sizeof(gettypeinfo_work_data));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;
  data->dataType = (SQLSMALLINT)datatype;

  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_GetTypeInfo,
      (uv_after_work_cb)UV_AfterGetTypeInfo);

  conn->Ref();

  DEBUG_PRINTF("ODBCConnection::GetTypeInfo - Exit\n");
  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCConnection::UV_GetTypeInfo(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_GetTypeInfo - Entry\n");

  gettypeinfo_work_data *data = (gettypeinfo_work_data *)(req->data);
  SQLHSTMT hSTMT;
  SQLRETURN ret = SQL_SUCCESS;

  // allocate a new statment handle
  ret = SQLAllocHandle(SQL_HANDLE_STMT,
                       data->conn->m_hDBC,
                       &hSTMT);

  DEBUG_PRINTF("ODBCConnection::UV_GetTypeInfo - ret=%d, dataType = %d\n",
               ret, data->dataType);
  if (!SQL_SUCCEEDED(ret))
  {
    // We'll check again later
  }
  else
  {
    ret = SQLGetTypeInfo(hSTMT, data->dataType);
  }

  data->rc = ret;
  data->hSTMT = hSTMT;
  DEBUG_PRINTF("ODBCConnection::UV_GetTypeInfo - Exit: rc=%d\n", data->rc);
}

void ODBCConnection::UV_AfterGetTypeInfo(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterGetTypeInfo - Entry\n");
  Nan::HandleScope scope;
  Local<Value> info[2];

  gettypeinfo_work_data *data = (gettypeinfo_work_data *)(req->data);

  if (data->rc != SQL_SUCCESS)
  {
    info[0] = ODBC::GetSQLError(SQL_HANDLE_DBC, data->conn->m_hDBC, (char *)"[node-ibm_db] SQL_ERROR");
    info[1] = Nan::Null();
    SQLFreeHandle(SQL_HANDLE_STMT, data->hSTMT);
    data->hSTMT = (SQLHSTMT)NULL;
  }
  else
  {
    Local<Value> result[4];
    bool *canFreeHandle = new bool(true);

    result[0] = Nan::New<External>((void *)(intptr_t)data->conn->m_hENV);
    result[1] = Nan::New<External>((void *)(intptr_t)data->conn->m_hDBC);
    result[2] = Nan::New<External>((void *)(intptr_t)data->hSTMT);
    result[3] = Nan::New<External>((void *)canFreeHandle);

    Local<Object> js_result = Nan::NewInstance(Nan::New(ODBCResult::constructor), 4, result).ToLocalChecked();
    info[0] = Nan::Null();
    info[1] = js_result;
  }

  Nan::TryCatch try_catch;

  data->cb->Call(2, info);

  data->conn->Unref();

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  delete data->cb;
  free(data);
  free(req);
  DEBUG_PRINTF("ODBCConnection::UV_AfterGetTypeInfo - Exit\n");
}

/*
 * GetFunctionsSync
 */

NAN_METHOD(ODBCConnection::GetFunctionsSync)
{
  DEBUG_PRINTF("ODBCConnection::GetFunctionsSync - Entry\n");
  Nan::HandleScope scope;

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  Local<Value> objError;
  SQLRETURN ret = SQL_SUCCESS;
  SQLUSMALLINT funcId = 0;
  SQLUSMALLINT supportedPtr = 0;
  SQLUSMALLINT supportedArr[100] = {0};

  REQ_ARGS(1);
  REQ_UINT_ARG(0, functionId);
  funcId = (SQLUSMALLINT)functionId;

  if (funcId)
  {
    ret = SQLGetFunctions(conn->m_hDBC, funcId, &supportedPtr);
  }
  else
  { // For ALL_FUNCTIONS
    ret = SQLGetFunctions(conn->m_hDBC, funcId, supportedArr);
  }

  DEBUG_PRINTF("ODBCConnection::GetFunctionsSync funcId=%i; supportedPtr=%i, \
                ret=%d\n",
               funcId, supportedPtr, ret);

  if (!SQL_SUCCEEDED(ret))
  {
    objError = ODBC::GetSQLError(SQL_HANDLE_DBC, conn->m_hDBC);
    Nan::ThrowError(objError);
    info.GetReturnValue().Set(Nan::False());
  }
  else
  {
    if (funcId)
    {
      Local<Value> value = Nan::New<Number>(supportedPtr);
      info.GetReturnValue().Set(value);
    }
    else
    {
      Local<Array> value = Nan::New<Array>();
      for (int i = 0; i < 100; i++)
      {
        Nan::Set(value, i, Nan::New<Number>(supportedArr[i]));
      }
      info.GetReturnValue().Set(value);
    }
  }
  DEBUG_PRINTF("ODBCConnection::GetFunctionsSync - Exit\n");
}

/*
 * GetFunctions
 */

NAN_METHOD(ODBCConnection::GetFunctions)
{
  DEBUG_PRINTF("ODBCConnection::GetFunctions - Entry\n");
  Nan::HandleScope scope;

  REQ_ARGS(2);
  REQ_UINT_ARG(0, functionId);
  REQ_FUN_ARG(1, cb);

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  getfunctions_work_data *data =
      (getfunctions_work_data *)calloc(1, sizeof(getfunctions_work_data));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  data->cb = new Nan::Callback(cb);
  data->conn = conn;
  data->funcId = (SQLUSMALLINT)functionId;

  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_GetFunctions,
      (uv_after_work_cb)UV_AfterGetFunctions);

  conn->Ref();

  DEBUG_PRINTF("ODBCConnection::GetFunctions - Exit\n");
  info.GetReturnValue().Set(Nan::Undefined());
}

void ODBCConnection::UV_GetFunctions(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_GetFunctions - Entry\n");

  // get our work data
  getfunctions_work_data *data = (getfunctions_work_data *)(req->data);

  DEBUG_PRINTF("ODBCConnection::UV_GetFunctions m_hDBC=%X, funcId=%d\n",
               data->conn->m_hDBC, data->funcId);

  if (data->funcId)
  {
    data->rc = SQLGetFunctions(
        data->conn->m_hDBC,
        data->funcId,
        &data->supportedPtr);
  }
  else
  {
    data->rc = SQLGetFunctions(
        data->conn->m_hDBC,
        data->funcId,
        data->supportedArr);
  }

  DEBUG_PRINTF("ODBCConnection::UV_GetFunctions - Exit: supported=%d, rc=%d\n",
               data->funcId ? data->supportedPtr : data->supportedArr[1], data->rc);
}

void ODBCConnection::UV_AfterGetFunctions(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterGetFunctions - Entry\n");
  Nan::HandleScope scope;
  Local<Value> info[2];

  getfunctions_work_data *data = (getfunctions_work_data *)(req->data);

  if (data->rc != SQL_SUCCESS)
  {
    info[0] = ODBC::GetSQLError(SQL_HANDLE_DBC, data->conn->m_hDBC, (char *)"[node-ibm_db] SQL_ERROR");
    info[1] = Nan::Null();
  }
  else
  {
    info[0] = Nan::Null();
    if (data->funcId)
    {
      info[1] = Nan::New<Number>(data->supportedPtr);
    }
    else
    {
      Local<Array> value = Nan::New<Array>();
      for (int i = 0; i < 100; i++)
      {
        Nan::Set(value, i, Nan::New<Number>(data->supportedArr[i]));
      }
      info[1] = value;
    }
  }

  Nan::TryCatch try_catch;

  data->cb->Call(2, info);

  if (try_catch.HasCaught())
  {
    FatalException(try_catch);
  }

  delete data->cb;

  free(data);
  free(req);
  DEBUG_PRINTF("ODBCConnection::UV_AfterGetFunctions - Exit\n");
}

/*
 * SetAttrSync
 *
 */

NAN_METHOD(ODBCConnection::SetAttrSync)
{
  DEBUG_PRINTF("ODBCConnection::SetAttrSync - Entry\n");
  Nan::HandleScope scope;

  REQ_ARGS(2);
  REQ_INT_ARG(0, attr);
  SQLPOINTER valuePtr = NULL;
  SQLINTEGER stringLength = SQL_IS_INTEGER;

  if (info[1]->IsInt32())
  {
    valuePtr = (SQLPOINTER)(Nan::To<v8::Int32>(info[1]).ToLocalChecked()->Value());
  }
  else if (info[1]->IsNull())
  {
    valuePtr = (SQLPOINTER)0;
  }
  else if (info[1]->IsString())
  {
    Nan::Utf8String value(info[1]);
    stringLength = value.length();
    GETCPPSTR(valuePtr, value, stringLength);
  }
  else
  {
    return Nan::ThrowTypeError("Unsupported Connection Attribute Value.");
  }

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());
  DEBUG_PRINTF("ODBCConnection::SetAttrSync: hENV=%X, hDBC=%X, "
               "Attribute=%d, value=%i, length=%d\n",
               conn->m_hENV, conn->m_hDBC, attr, valuePtr, stringLength);

  SQLRETURN ret = SQLSetConnectAttr(conn->m_hDBC, attr, valuePtr, stringLength);

  if (stringLength != SQL_IS_INTEGER)
  {
    FREE(valuePtr);
  }

  if (SQL_SUCCEEDED(ret))
  {
    info.GetReturnValue().Set(Nan::True());
  }
  else
  {
    Nan::ThrowError(ODBC::GetSQLError(
        SQL_HANDLE_DBC,
        conn->m_hDBC,
        (char *)"[node-ibm_db] Error in ODBCConnection::SetAttrSync"));

    info.GetReturnValue().Set(Nan::False());
  }

  DEBUG_PRINTF("ODBCConnection::SetAttrSync - Exit\n");
}

/*
 * SetAttr
 *
 */

NAN_METHOD(ODBCConnection::SetAttr)
{
  DEBUG_PRINTF("ODBCConnection::SetAttr - Entry\n");

  Nan::HandleScope scope;
  REQ_ARGS(3);
  REQ_INT_ARG(0, attr);
  REQ_FUN_ARG(2, cb);
  SQLPOINTER valuePtr = NULL;
  SQLINTEGER stringLength = SQL_IS_INTEGER;

  if (info[1]->IsInt32())
  {
    valuePtr = (SQLPOINTER)(Nan::To<v8::Int32>(info[1]).ToLocalChecked()->Value());
  }
  else if (info[1]->IsNull())
  {
    valuePtr = (SQLPOINTER)0;
  }
  else if (info[1]->IsString())
  {
    Nan::Utf8String value(info[1]);
    stringLength = value.length();
    GETCPPSTR(valuePtr, value, stringLength);
  }
  else
  {
    return Nan::ThrowTypeError("Unsupported Connection Attribute Value.");
  }

  ODBCConnection *conn = Nan::ObjectWrap::Unwrap<ODBCConnection>(info.Holder());

  uv_work_t *work_req = (uv_work_t *)(calloc(1, sizeof(uv_work_t)));
  MEMCHECK(work_req);

  setattr_work_data *data =
      (setattr_work_data *)calloc(1, sizeof(setattr_work_data));
  if (!data)
    free(work_req);
  MEMCHECK(data);

  data->conn = conn;
  data->attr = attr;
  data->valuePtr = valuePtr;
  data->stringLength = stringLength;

  DEBUG_PRINTF("ODBCConnection::SetAttr: hENV=%X, hDBC=%X, "
               "Attribute=%d, value=%i, length=%d\n",
               data->conn->m_hENV, data->conn->m_hDBC,
               data->attr, data->valuePtr, data->stringLength);

  data->cb = new Nan::Callback(cb);
  work_req->data = data;

  uv_queue_work(
      uv_default_loop(),
      work_req,
      UV_SetAttr,
      (uv_after_work_cb)UV_AfterSetAttr);

  conn->Ref();

  info.GetReturnValue().Set(Nan::Undefined());
  DEBUG_PRINTF("ODBCConnection::SetAttr - Exit\n");
}

void ODBCConnection::UV_SetAttr(uv_work_t *req)
{
  DEBUG_PRINTF("ODBCConnection::UV_SetAttr - Entry\n");

  setattr_work_data *data = (setattr_work_data *)(req->data);

  data->result = SQLSetConnectAttr(data->conn->m_hDBC,
                                   data->attr,
                                   data->valuePtr,
                                   data->stringLength);

  DEBUG_PRINTF("ODBCConnection::UV_SetAttr - Exit\n");
}

void ODBCConnection::UV_AfterSetAttr(uv_work_t *req, int status)
{
  DEBUG_PRINTF("ODBCConnection::UV_AfterSetAttr - Entry\n");

  setattr_work_data *data = (setattr_work_data *)(req->data);

  Nan::HandleScope scope;

  // an easy reference to the statment object
  ODBCConnection *self = data->conn->self();

  // Check if there were errors
  if (data->result == SQL_ERROR)
  {
    ODBC::CallbackSQLError(
        SQL_HANDLE_DBC,
        self->m_hDBC,
        data->cb);
  }
  else
  {
    Local<Value> info[2];

    info[0] = Nan::Null();
    info[1] = Nan::True();

    Nan::TryCatch try_catch;

    data->cb->Call(2, info);

    if (try_catch.HasCaught())
    {
      FatalException(try_catch);
    }
  }

  self->Unref();
  delete data->cb;
  if (data->stringLength != SQL_IS_INTEGER)
  {
    FREE(data->valuePtr);
  }

  free(data);
  free(req);
  DEBUG_PRINTF("ODBCConnection::UV_AfterSetAttr - Exit\n");
}

# ibm_db API Documentation

## Database APIs

**APIs for creating and droping Database using node.js application**
*  [.createDbSync(dbName, connectionString, [options])](#createDbSyncApi)
*  [.dropDBSync(dbName, connectionString)](#dropDbSyncApi)

**Database APIs**
1.  [.open(connectionString, [options,] callback)](#openApi)
2.  [.openSync(connectionString)](#openSyncApi)
3.  [.query(sqlQuery [, bindingParameters], callback)](#queryApi)
4.  [.querySync(sqlQuery [, bindingParameters])](#querySyncApi) 
5.  [.queryStream(sqlQuery [, bindingParameters])](#queryStreamApi) 
6.  [.queryResult(sqlQuery [, bindingParameters], callback)](#queryResultApi)
7.  [.queryResultSync(sqlQuery [, bindingParameters])](#queryResultSyncApi)
8.  [.close(callback)](#closeApi)
9.  [.closeSync()](#closeSyncApi)
10. [.prepare(sql, callback)](#prepareApi)
11. [.prepareSync(sql)](#prepareSyncApi)
12. [.bind(bindingParameters, callback)](#bindApi)
13. [.bindSync(bindingParameters)](#bindSyncApi)
14. [.execute([bindingParameters], callback)](#executeApi)
15. [.executeSync([bindingParameters])](#executeSyncApi)
16. [.executeNonQuery([bindingParameters], callback)](#executeNonQueryApi)
17. [.fetch(option, callback)](#fetchApi)
18. [.fetchSync(option)](#fetchSyncApi)
19. [.fetchAll(option, callback)](#fetchAllApi)
20. [.fetchAllSync(option)](#fetchAllSyncApi)
21. [.beginTransaction(callback)](#beginTransactionApi)
22. [.beginTransactionSync()](#beginTransactionSyncApi)
23. [.commitTransaction(callback)](#commitTransactionApi)
24. [.commitTransactionSync()](#commitTransactionSyncApi)
25. [.rollbackTransaction(callback)](#rollbackTransactionApi)
26. [.rollbackTransactionSync()](#rollbackTransactionSyncApi)
27. [.setIsolationLevel(isolationLevel)](#setIsolationLevelApi)
28. [.getColumnNamesSync()](#getColumnNamesSyncApi)
29. [.getColumnMetadataSync()](#getColumnMetadataSyncApi)
30. [.getSQLErrorSync()](#getSQLErrorSyncApi)
31. [.debug(value)](#enableDebugLogs)

*   [**Connection Pooling APIs**](#PoolAPIs)
*   [**bindingParameters**](#bindParameters)
*   [**CALL Statement**](#callStmt)


### <a name="openApi"></a> 1) .open(connectionString, [options,] callback)

Open a connection to a database.

* **connectionString** - The connection string for your database.
    * For distributed platforms, the connection string is typically defined as:
    `DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=passwd`
    * For z/OS, the ODBC driver makes both local and remote connections using DSN, UID and PWD.
    The connection string is typically defined as: `DSN=dbname;UID=username;PWD=passwd`.  To
    connect to remote Db2 databases, the connectivity information will need to be set up in the
    Communications Database (CDB).  Please refer to scenario 1 in the following
    [article](https://www.ibm.com/developerworks/data/library/techarticle/0310chong/0310chong.html).
* **options** - _OPTIONAL_ - Object type. Can be used to avoid multiple
    loading of native ODBC library for each call of `.open`. Also, can be used
    to pass connectTimeout value and systemNaming(true/false) for i5/OS server.
* **callback** - `callback (err, conn)`

```javascript
var ibmdb = require("ibm_db")
  , connStr = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=passwd";

ibmdb.open(connStr, function (err, connection) {
    if (err)
    {
      console.log(err);
      return;
    }
    connection.query("select 1 from sysibm.sysdummy1", function (err1, rows) {
      if (err1) console.log(err1);
      else console.log(rows);
      connection.close(function(err2) {
        if(err2) console.log(err2);
      });
    });
});

```

* **Secure Database Connection using SSL/TSL** - ibm_db supports secure connection to Database Server over SSL same as ODBC/CLI driver. If you have SSL Certificate from server or an CA signed certificate, just use it in connection string as below:

```javascript
connStr = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=passwd;Security=SSL;SSLServerCertificate=<cert.arm_file_path>;";
```
To connect to dashDB in bluemix, just use below connection string:
```
connStr = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=passwd;Security=SSL";
```
You can also create a KeyStore DB using GSKit command line tool and use it in connection string along with other keywords as documented in [DB2 Infocenter](http://www.ibm.com/support/knowledgecenter/en/SSEPGG_11.1.0/com.ibm.db2.luw.admin.sec.doc/doc/t0053518.html).

**Note:** The SSL Certificate file must have `.arm` extention. Certificate file with `.crt` or any other extention would not work with `SSLServerCertificate` keyword. You need to create keystoredb using GSKit and add certificate of other extention to keystoredb to use as documented in [DB2 Infocenter](http://www.ibm.com/support/knowledgecenter/en/SSEPGG_11.1.0/com.ibm.db2.luw.admin.sec.doc/doc/t0053518.html).

### <a name="openSyncApi"></a> 2) .openSync(connectionString [,options])

Synchronously open a connection to a database.

* **connectionString** - The connection string for your database
* **options** - _OPTIONAL_ - Object type. Can be used to avoid multiple
    loading of native ODBC library for each call of `.open`. Also, can be used
    to pass connectTimeout value and systemNaming value for i5/OS server.

```javascript
var ibmdb = require("ibm_db"),
	connString = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password;";

try {
      var option = { connectTimeout : 40, systemNaming : true };// Connection Timeout after 40 seconds.
      var conn = ibmdb.openSync(connString, option);
      conn.query("select * from customers fetch first 10 rows only", function (err, rows) {
		if (err) {
			console.log(err);
		} else {
		  console.log(rows);
		}
		conn.close();
      });
    } catch (e) {
      console.log(e.message);
    }
```

### <a name="queryApi"></a> 3) .query(sqlQuery [, bindingParameters], callback)

Issue an asynchronous SQL query to the database which is currently open.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue}. noResults accepts only true or false values.
If true - query() will not return any result. "sql" field is mandatory in Object, others are _OPTIONAL_.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`. bindingParameters in sqlQuery Object takes precedence over it.

* **callback** - `callback (err, rows, sqlca)`

```javascript
var ibmdb = require("ibm_db")
	, cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password;"
	;

ibmdb.open(cn, function (err, conn) {
    if (err) {
      return console.log(err);
    }

    // we now have an open connection to the database, so lets get some data.
    // Execute multiple query and get multiple result sets.
    // In case of multiple resultset, query will return an array of result sets.
    conn.query("select 1 from sysibm.sysdummy1;select 2 from sysibm.sysdummy1;" +
               "select 3 from sysibm.sysdummy1", function (err, rows, sqlca)
    {
        if (err) {
            console.log(err);
        } else {
            console.log(rows); // rows = [ [ { '1': 1 } ], [ { '1': 2 } ], [ { '1': 3 } ] ]
        }
    });
});
```

### <a name="querySyncApi"></a> 4) .querySync(sqlQuery [, bindingParameters])

Synchronously issue a SQL query to the database that is currently open.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue}. noResults accepts only true or false values.
If true - query() will not return any result. If noResults is true for CALL statement, querySync returns only OutParams. "sql" field is mandatory in Object, others are optional.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`.

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn){

  //blocks until the query is completed and all data has been acquired
  var rows = conn.querySync("select * from customers fetch first 10 rows only");

  console.log(rows);
});
```

### <a name="queryStreamApi"></a> 5) .queryStream(sqlQuery [, bindingParameters])

Synchronously issue a SQL query to the database that is currently open and returns
a Readable stream. Application can listen the events emmitted by returned stream
and take action.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue}. noResults accepts only true or false values. If true - query() will not return any result. "sql" field is mandatory in Object, others are optional.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`.

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err, conn)
{
    var stream = conn.queryStream("select 1 from sysibm.sysdummy1");

    stream.once('data', function (result) {
      console.log(result);
    }).once('error', function (err) {
      conn.closeSync();
      throw err;
    }).once('end', function () {
      conn.close(function(){ console.log("done.") });
    });
});
```

### <a name="queryResultApi"></a> 6) .queryResult(sqlQuery, [, bindingParameters], callback)

Issue an asynchronous SQL query to the database which is currently open and return (err, result, outparams) to callback function. `result` is ODBCResult object. Uisng `result`, call `result.fetchAllSync()` to retrieve all rows; call `result.getColumnMetadataSync()` to get meta data info or call `result.fetchSync()` to retrieve each row one by one and process. Execute `result.closeSync()` once done with the `result` object.
`query` returns all the rows on call, but `queryResult` returns the result object and rows need to be fetched by the caller.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue}.
noResults accepts only true or false values. If true - queryResult() will not return any result object and value of result will be null.
"sql" field is mandatory in Object, others are _OPTIONAL_.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any ? characters (called parameter marker) in `sqlQuery`. bindingParameters in sqlQuery Object takes precedence over it.

* **callback** - `callback (err, result, outparams)`.
outparams is returned only for CALL statement with OUT parameters. Any resultset expected from SP should get retrieved using result.fetch apis.

```javascript
var ibmdb = require("ibm_db")
	, cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password;"
	;
ibmdb.open(cn, function (err,conn) {
    if (err) return console.log(err);
    var query = 'select creator, name from sysibm.systables where 1 = ?';
    conn.queryResult(query, [1], function (err, result) {
        if(err) { console.log(err); }
        else {
          console.log("data = ", result.fetchAllSync());
          console.log("metadata = ", result.getColumnMetadataSync());
          result.closeSync(); // Must call in application.
          conn.closeSync();
          console.log("Executed ", ++loop, " times.");
        }
    });
});
```
**Note:** Once you are done with the `result` object, must close it to avoid error when garbage collector of javascript free it. Not calling the `result.closeSync() may cause invalid handle error in application or no data.

### <a name="queryResultSyncApi"></a> 7) .queryResultSync(sqlQuery [, bindingParameters])

Synchronously issue a SQL query to the database that is currently open and return a result object to the callback function on success. In case of CALL statement with OUT parameters, it returns an array of [result, outparams]. `result` is an ODBCResult object that can be used to fetch rows.

`querySync`API returns all the rows on call, but `queryResultSync` API returns the `ODBCResult` object using which application should call fetch APIs to get data.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue}. noResults accepts only true or false values. If true - the value of `result` will be null. "sql" field is mandatory in Object, others are optional.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`.

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn){
  if (err) return console.log(err);
  var query = 'select creator, name from sysibm.systables';
  var result = conn.queryResultSync(query);
  console.log("data = ", result.fetchAllSync());
  console.log("metadata = ", result.getColumnMetadataSync());
  result.closeSync(); // Must call to free to avoid application error.
  conn.closeSync();
});
```
**Note:** Once you are done with the `result` object, must close it to avoid error when garbage collector of javascript free it. Not calling the `result.closeSync() may cause invalid handle error in application or no data.

In case of CALL statement with OUT params, check result[0] is an object or not.

### <a name="closeApi"></a> 8) .close(callback)

Close the currently opened database.

* **callback** - `callback (err)`

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function (err, conn) {
	if (err) {
		return console.log(err);
	}

	//we now have an open connection to the database
	conn.close(function (err) {
		console.log("the database connection is now closed");
	});
});
```

### <a name="closeSyncApi"></a> 9) .closeSync()

Synchronously close the currently opened database.

```javascript
var ibmdb = require("ibm_db")()
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

//Blocks until the connection is open
ibmdb.openSync(cn);

//Blocks until the connection is closed
ibmdb.closeSync();
```

### <a name="prepareApi"></a> 10) .prepare(sql, callback)

Prepare a statement for execution.

* **sql** - SQL string to prepare
* **callback** - `callback (err, stmt)`

Returns a `Statement` object via the callback

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  conn.prepare("insert into hits (col1, col2) VALUES (?, ?)", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }

    //Bind and Execute the statment asynchronously
    stmt.execute(['something', 42], function (err, result) {
      if( err ) console.log(err);
      else result.closeSync();

      //Close the connection
	  conn.close(function(err){});
    });
  });
});
```

### <a name="prepareSyncApi"></a> 11) .prepareSync(sql)

Synchronously prepare a statement for execution.

* **sql** - SQL string to prepare

Returns a `Statement` object

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  var stmt = conn.prepareSync("select * from employee where empid = ?");

  //Bind and Execute the statment asynchronously
  stmt.execute([142], function (err, result) {
    data = result.fetchAllSync();
    console.log(data);
    result.closeSync();
    stmt.closeSync();

    //Close the connection
	conn.close(function(err){});
  });
});
```

### <a name="bindApi"></a> 12) .bind(bindingParameters, callback)

Binds the parameters for prepared statement.

* **bindingParameters** - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be array or object itself. Check [bindingParameters](#bindParameters) doc for detail.
* **callback** - `callback (err)`

### <a name="bindSyncApi"></a> 13) .bindSync(bindingParameters)

Binds the parameters for prepared statement synchronously. If `bindSync()` is used, then no need to pass `bindingParameters` to next `execute()` or `executeSync()` statement.

* **bindingParameters** - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be array or object itself. Check [bindingParameters](#bindParameters) doc for detail.

### <a name="executeApi"></a> 14) .execute([bindingParameters], callback)

Execute a prepared statement.

* **bindingParameters** - OPTIONAL - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be array or object itself. Check [bindingParameters](#bindParameters) doc for detail.
* **callback** - `callback (err, result, outparams)`
outparams - will have result for INOUT and OUTPUT parameters of Stored Procedure.

Returns a `Statement` object via the callback

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  conn.querySync("create table mytab (id int, photo BLOB(30K))");
  conn.prepare("insert into mytab (id, photo) VALUES (?, ?)", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }

    // Create params object
    var img = {ParamType:"FILE", DataType: "BLOB", "Data": "smile.jpg"};

    //Bind and Execute the statment asynchronously
    stmt.execute([ 42, img ], function (err, result) {
      if( err ) console.log(err);
      else result.closeSync();

      //Close the connection
      stmt.closeSync();
	  conn.close(function(err){});
    });
  });
});
```

### <a name="executeSyncApi"></a> 15) .executeSync([bindingParameters])

Execute a prepared statement synchronously.

* **bindingParameters** - OPTIONAL - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be array or object itself. Check [bindingParameters](#bindParameters) doc for detail. Instead of passing bindingParameters to executeSync(), parameters can also be binded using bind() or bindSync() APIs.

Returns a `Statement` object. If prepared statement is a stored procedure with INOUT or OUT parameter, executeSync() returns an array of two elements in the form [stmt, outparams]. The first element of such array is an `Statement` object and second element is an `Array` of INOUT and OUTPUT parameters in sequence.

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  var stmt = conn.prepareSync("select empname from emptable where empid = ?");

  //Bind and Execute the statment asynchronously
  var result = stmt.executeSync([142]);
  var data = result.fetchAllSync({fetchMode:3}); // Fetch data in Array mode.
  console.log(data);
  result.closeSync();
  stmt.closeSync();

  //Close the connection
  conn.close(function(err){});
});
```

### <a name="executeNonQueryApi"></a> 16) .executeNonQuery([bindingParameters], callback)

Execute a non query prepared statement and returns the number of rows affected in a table by the statement.

* **bindingParameters** - OPTIONAL - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be array or object itself. Check [bindingParameters](#bindParameters) doc for detail.
* **callback** - `callback (err, affectedRowCount)`

It returns the number of rows in a table that were affected by an UPDATE, an INSERT, a DELETE, or a MERGE statement issued against the table, or a view based on the table. If no rows are affected, it returns -1 via the callback function.

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  conn.querySync("create table mytab (id int, text varchar(30))");
  conn.prepare("insert into mytab (id, text) VALUES (?, ?)", function (err, stmt) {
    if (err) {
      console.log(err);
      return conn.closeSync();
    }

    //Bind and Execute the statment asynchronously
    stmt.executeNonQuery([ 42, 'hello world' ], function (err, ret) {
      if( err ) console.log(err);
      else console.log("Affected rows = " + ret);

      //Close the connection
	  conn.close(function(err){});
    });
  });
});
```

### <a name="fetchApi"></a> 17) .fetch(option, callback)

Fetch a row of data from ODBCResult object asynchronously.

* **option** - _OPTIONAL_ - Object type.
    * fetchMode - Format of returned row data. By default row data get returned in object form. option = {fetchMode:3} will return row in array form.

* **callback** - `callback (err, row)`

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int)");
  conn.querySync("insert into hits values ('something', 42)");
  conn.querySync("insert into hits values ('für', 43)");
  conn.prepare("select * from hits", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }
    stmt.execute(function (err, result) {
      if( err ) console.log(err);
      result.fetch(function (err, row) {
          if(err) { console.log(err); }
          else {
            console.log("Row1 = ", row);
            result.fetch({fetchMode:3}, function (err, row) {
              if(err) { console.log(err); }
              console.log("Row2 = ", row);
              result.closeSync();
              conn.querySync("drop table hits");
              //Close the connection
              conn.close(function(err){console.log("Connection Closed.");});
            });
          }
      });
    });
  });
});
```

### <a name="fetchSyncApi"></a> 18) .fetchSync(option)

Fetch a row of data from ODBCResult object synchronously.

* **option** - _OPTIONAL_ - Object type.
    * fetchMode - Format of returned row data. By default row data get returned in object form. option = {fetchMode:3} will return row in array form.

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int)");
  conn.querySync("insert into hits values ('something', 42)");
  conn.querySync("insert into hits values ('für', 43)");
  var stmt = conn.prepareSync("select * from hits");
  var result = stmt.executeSync();
  var data = 0;
  while( data = result.fetchSync({fetchMode:3}) ) {
    console.log(data);
  }
  result.closeSync();
  conn.querySync("drop table hits");
  conn.closeSync();
});
```

### <a name="fetchAllApi"></a> 19) .fetchAll(option, callback)

Fetch all rows from ODBCResult object asynchronously for the executed statement.

* **option** - _OPTIONAL_ - Object type.
    * fetchMode - Format of returned row data. By default row data get returned in object form. option = {fetchMode:3} will return rows in array form. {fetchMode:4} - return rows in object form.

* **callback** - `callback (err, data, noOfColumns)`

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int)");
  conn.querySync("insert into hits values ('something', 42)");
  conn.querySync("insert into hits values ('für', 43)");
  conn.prepare("select * from hits", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }
    stmt.execute(function (err, result) {
      if( err ) console.log(err);
      result.fetchAll({fetchMode:4}, function (err, data, colcount) {
          if(err) { console.log(err); }
          else {
            console.log("Data = ", data);
            console.log("No of columns = ", colcount);
          }
          result.closeSync();
          conn.querySync("drop table hits");
          //Close the connection
          conn.close(function(err){console.log("Connection Closed.");});
      });
    });
  });
});
```

### <a name="fetchAllSyncApi"></a> 20) .fetchAllSync(option)

Fetch all rows from ODBCResult object Synchronously for the executed statement.

* **option** - Optional object to specify return type of data. By default row data get returned in object form. option = {fetchMode:3} will return row in array form.

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int)");
  conn.querySync("insert into hits values ('something', 42)");
  conn.querySync("insert into hits values ('für', 43)");
  var stmt = conn.prepareSync("select * from hits");
  var result = stmt.executeSync();
  var data = result.fetchAllSync();
  console.log("Fetched data = ", data);
  result.closeSync();
  conn.querySync("drop table hits");
  conn.closeSync();
});
```
For example of prepare once and execute many times with above fetch APIs, please see test file [test-fetch-apis.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-fetch-apis.js).

### <a name="beginTransactionApi"></a> 21) .beginTransaction(callback)

Begin a transaction

* **callback** - `callback (err)`

### <a name="beginTransactionSyncApi"></a> 22) .beginTransactionSync()

Synchronously begin a transaction

### <a name="commitTransactionApi"></a> 23) .commitTransaction(callback)

Commit a transaction

* **callback** - `callback (err)`

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err,conn) {

  conn.beginTransaction(function (err) {
    if (err) {
      //could not begin a transaction for some reason.
      console.log(err);
      return conn.closeSync();
    }

    var result = conn.querySync("insert into customer (customerCode) values ('stevedave')");

    conn.commitTransaction(function (err) {
      if (err) {
        //error during commit
        console.log(err);
        return conn.closeSync();
      }

    console.log(conn.querySync("select * from customer where customerCode = 'stevedave'"));

     //Close the connection
     conn.closeSync();
    });
  });
});
```

### <a name="commitTransactionSyncApi"></a> 24) .commitTransactionSync()

Synchronously commit a transaction

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err,conn) {

  conn.beginTransaction(function (err) {
    if (err) {
      //could not begin a transaction for some reason.
      console.log(err);
      return conn.closeSync();
    }

    var result = conn.querySync("insert into customer (customerCode) values ('stevedave')");

    conn.commitTransactionSync();

    console.log(conn.querySync("select * from customer where customerCode = 'stevedave'"));

     //Close the connection
    conn.closeSync();
  });
});
```

### <a name="rollbackTransactionApi"></a> 25) .rollbackTransaction(callback)

Rollback a transaction

* **callback** - `callback (err)`

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err,conn) {

  conn.beginTransaction(function (err) {
    if (err) {
      //could not begin a transaction for some reason.
      console.log(err);
      return conn.closeSync();
    }

    var result = conn.querySync("insert into customer (customerCode) values ('stevedave')");

    conn.rollbackTransaction(function (err) {
      if (err) {
        //error during rollback
        console.log(err);
        return conn.closeSync();
      }

    console.log(conn.querySync("select * from customer where customerCode = 'stevedave'"));

     //Close the connection
     conn.closeSync();
    });
  });
});
```

### <a name="rollbackTransactionSyncApi"></a> 26) .rollbackTransactionSync()

Synchronously rollback a transaction

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err,conn) {

  conn.beginTransaction(function (err) {
    if (err) {
      //could not begin a transaction for some reason.
      console.log(err);
      return conn.closeSync();
    }

    var result = conn.querySync("insert into customer (customerCode) values ('stevedave')");

    conn.rollbackTransactionSync();

    console.log(conn.querySync("select * from customer where customerCode = 'stevedave'"));

     //Close the connection
    conn.closeSync();
  });
});
```

### <a name="setIsolationLevelApi"></a> 27) .setIsolationLevel(isolationLevel)

Synchronously sets the default isolation level passed as argument. It is only applicable when the default isolation level is used. It will have no effect if the application has specifically set the isolation level for a transaction.

* **isolationLevel:** An integer representing the isolation level to be set. Its value must be only - 1|2|4|8|32. For details check this [doc](https://www.ibm.com/support/knowledgecenter/en/SSEPGG_11.1.0/com.ibm.db2.luw.apdv.cli.doc/doc/r0008832.html).

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err,conn) {
  conn.setIsolationLevel(2);  // SQL_TXN_READ_COMMITTED
  conn.setIsolationLevel(4); // SQL_TXN_REPEATABLE_READ
  conn.querySync("create table mytab1 (c1 int, c2 varchar(10))");
});
```

### <a name="getColumnNamesSyncApi"></a> 28) .getColumnNamesSync()

Synchronously retrieve the name of columns returned by the resulset. It
 operates on ODBCResult object.
```javascript
  conn.querySync("insert into mytab1 values ( 5, 'abc')");
  conn.prepare("select * from mytab1", function (err, stmt) {
    stmt.execute(function(err, result) {
      console.log("Column Names = ", result.getColumnNamesSync());
      result.closeSync(); conn.closeSync(); }); });
```

### <a name="getColumnMetadataSyncApi"></a> 29) .getColumnMetadataSync()

Synchronously retrieve the metadata about columns returned by the resulset. It
 operates on ODBCResult object.
```javascript
  conn.querySync("insert into mytab1 values ( 5, 'abc')");
  conn.prepare("select * from mytab1", function (err, stmt) {
    stmt.execute(function(err, result) {
      console.log("Column Names = ", result.getColumnNamesSync());
      console.log("Column Meta Data = ", result.getColumnMetadataSync());
      console.log("Fetched Data = ", result.fetchAllSync() );
      result.closeSync();
      conn.closeSync();
    });
  });
```

### <a name="getSQLErrorSyncApi"></a> 30) .getSQLErrorSync()

Synchronously retrieve the sqlerror message and codes for last instruction executed on a statement handle using SQLGetDiagRec ODBC API. It operates on ODBCResult object.
```javascript
  conn.querySync("insert into mytab1 values ( 5, 'abc')");
  conn.prepare("select * from mytab1", function (err, stmt) {
    stmt.execute(function(err, result) {
      console.log("Fetched Data = ", result.fetchAllSync() );
      console.log("SQLError = ", result.getSQLErrorSync());
      result.closeSync();
      conn.closeSync();
    });
  });
```

### <a name="enableDebugLogs"></a> 31) .debug(value)

Enable console logs.

* **value** - true/false.

```javascript
var ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.debug(true);  // **==> ENABLE CONSOLE LOGS. <==**

[ibmdb.open](#openApi)(cn, function (err, connection) {
    if (err)
    {
        console.log(err);
        return;
    }
    connection.query("select 1 from sysibm.sysdummy1", function (err1, rows) {
        if (err1) console.log(err1);
        else console.log(rows);

        ibmdb.debug(false);  // Disable console logs.

        connection.close(function(err2) {
            if(err2) console.log(err2);
        });
    });
});
```

## Create and Drop Database APIs

### <a name="createDbSyncApi"></a> .createDbSync(dbName, connectionString, [options])

To create a database (dbName) through Node.js application.

* **dbName** - The database name.
* **connectionString** - The connection string for your database instance.
* **options** - _OPTIONAL_ - Object type.
    * codeSet - Database code set information.
    * mode    - Database logging mode (applicable only to "IDS data servers").

```javascript
var ibmdb = require("ibm_db");
// Connection string without "DATABASE" keyword and value.
var cn = "HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

var DB_NAME = "TESTDB";

var createDB = ibmdb.createDbSync(DB_NAME, cn);

if(createDB) {
  console.log("Database created successfully.");
  // Connection string with newly created "DATABASE" name.
	var conStr = cn + ";" + "DATABASE=" + DB_NAME;

	ibmdb.open(conStr, function(err, conn) {
		if(err) console.log(err);
		else console.log("Database connection opened.");
	});
}
```

Note: This API is not supported for Db2 on z/OS servers.  Given that connection
to Db2 on z/OS is to a specific subsystem, this API is not applicable.

### <a name="dropDbSyncApi"></a> .dropDbSync(dbName, connectionString)

To drop a database (dbName) through node.js application.

* **dbName** - The database name.
* **connectionString** - The connection string for your database instance.

```javascript
var ibmdb = require("ibm_db");
// Connection string without "DATABASE" keyword and value.
var cn = "HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

var DB_NAME = "TESTDB";

var dropDB = ibmdb.dropDbSync(DB_NAME, cn);

if(dropDB) {
  console.log("Database dropped successfully.");
}
```

Note: This API is not supported for Db2 on z/OS servers.  Given that connection
to Db2 on z/OS is to a specific subsystem, this API is not applicable.

## <a name="PoolAPIs"></a>Connection Pooling APIs

node-ibm_db reuses node-odbc pool.
The node-odbc `Pool` is a rudimentary connection pool which will attempt to have
database connections ready and waiting for you when you call the `open` method.

If you use a `Pool` instance, any connection that you close will get added to
the list of available connections immediately. Such connection will be used
the next time you call `Pool.open()` for the same connection string.

For applications using multiple connections simultaneously, it is recommended to
use Pool.open instead of [ibmdb.open](#openApi).

1.  [.open(connectionString, callback)](#openPoolApi)
2.  [.close(callback)](#closePoolApi)
3.  [.init(N, connStr)](#initPoolApi)
4.  [.setMaxPoolSize(N)](#setMaxPoolSize)

### <a name="openPoolApi"></a> 1) .open(connectionString, callback)

Get a `Database` instance which is already connected to `connectionString`

* **connectionString** - The connection string for your database
* **callback** - `callback (err, db)`

```javascript
var Pool = require("ibm_db").Pool
	, pool = new Pool()
    , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

pool.open(cn, function (err, db) {
	if (err) {
		return console.log(err);
	}

	//db is now an open database connection and can be used like normal
	//if we run some queries with db.query(...) and then call db.close();
	//a connection to `cn` will be re-opened silently behind the scene
	//and will be ready the next time we do `pool.open(cn)`
});
```

### <a name="closePoolApi"></a> 2) .close(callback)

Close all connections in the `Pool` instance

* **callback** - `callback (err)`

```javascript
var Pool = require("ibm_db").Pool
	, pool = new Pool()
    , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

pool.open(cn, function (err, db) {
	if (err) {
		return console.log(err);
	}

	//db is now an open database connection and can be used like normal
	//but all we will do now is close the whole pool

	pool.close(function () {
		console.log("all connections in the pool are closed");
	});
});
```

### <a name="initPoolApi"></a> 3) .init(N, connStr)

Initialize `Pool` with N no of active connections using supplied connection string.

* **N** - No of connections to be initialized.
* **connStr** - The connection string for your database
```
var ret = pool.init(5, connStr);
if(ret != true)
{
    console.log(ret);
    return false;
}

pool.open(connStr, function(err, db) { ...
```

### <a name="setMaxPoolSize"></a> 4) .setMaxPoolSize(N)

Number of maximum connection to database supported by current pool.

* **N** - No of maximum connections in the pool.
```
pool.setMaxPoolSize(20);
pool.open(connStr, function(err, db) { ...
```

## <a name="bindParameters"></a>bindingParameters

Bind arguments for each parameter marker(?) in SQL query.
These parameters can be used with query(), querySync, bind(), execute() APIs.
bindingParameters is an array of Values like: [val1, val2, ...]
Each value in itself can be an array or Object holing multiple bind options.
If parameters are not an integer or string, it is recomended to pass an Object with different bind options. The object can have following keys:

`{"ParamType":"INOUT", CType:"BINARY", SQLType:"BLOB",DataType: "BLOB", Data:imgfile, Length:50}`

Either SQLType or DataType must be used. If SQLType is used, DataType will be ignored.

* **ParmType**: Type of the Parameter. Supported Values are:
 - INPUT - Bind the parameter using SQL_PARAM_INPUT(defined in ibm_db/installer/clidriver/include/sqlext.h file). It is used as input value and it is the default value, if you don't use this key in object.
 - OUTPUT - Bind the parameter using SQL_PARAM_OUTPUT. It is basically used for Stored Procedure call which has output parameters.
 - INOUT - Bind the parameter using SQL_PARAM_INPUT_OUTPUT. It is also used for Stored Procedure call.
 - FILE  - It tells the Data is a filename that contains actual data to load. If you want to load an image to database, use this input type along with DataType as BLOB for binary file.
   f.e. `{ParamType: "FILE", DataType: "BLOB", Data: "mypic.jpg"}`

* **CType**: C Data type of the parameter to be bound. Default value is CHAR.
* **SQLType**: Data type of the parameter on Server. It is actually the column Type of the parameter. Default value is CHAR
* **DataType**: Same as SQLType. Use either SQLType or DataType. Added for simple name. Default Value is CHAR.
* **Data**: Its value is actuall data for the parameter. For binary data, it should represent the full buffer containing binary data. For ParamType:"FILE", it must have the filename on disc that contains data. It is mandatory key in the data Object.
* **Length**: It denotes the buffer length in byte to store the OUT Pamameter value when ParamType is INOUT or OUTPUT in a Stored Procedure call..

* Few example of bidningParameters that we can use in node.js program:
```
[18, 'string']
[3, 5, 3.8, 'string', 9.1]
[18, [1, 1, 1, 'string']]
[[1, 1, 1, 18], [1, 1, 1, 'string']]
[18, {ParamType:"INPUT", "Data": "string"}]
[18, {ParamType:"INPUT", CType: "CHAR", SQLType: "CHAR", "Data": "string"}]
[38, {ParamType:"INPUT", SQLType: "CHAR", "Data": "string"}]
[38, {ParamType:"INPUT", DataType: "CHAR", "Data": "string"}]
[[1,1,1,38], {"Data": "string"}]
[38, {ParamType:"INPUT", DataType: "CLOB", "Data": var1}] - here var1 contains full CLOB data to be inserted.
[38, {ParamType:"FILE", DataType: "CLOB", "Data": filename}] - here filename is the name of file which has large character data.
```
The values in array parameters used in above example is not recommened to use as it is dificult to understand. These values are macro values from ODBC specification and we can directly use those values. To understand it, see the [SQLBindParameter](http://www.ibm.com/support/knowledgecenter/en/SSEPGG_11.1.0/com.ibm.db2.luw.apdv.cli.doc/doc/r0002218.html) documentation for DB2.

Pass bind parameters as Object if you want to insert a BLOB or CLOB data to DB2. Check below test files to know how to insert a BLOB and CLOB data from buffer and file:

 - [test-blob-insert.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-blob-insert.js) - To insert a BLOB and CLOB data using memory buffer. Application need to read the file contents and then use as bind parameter.
 - [test-blob-file.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-blob-file.js) - To insert an image file and large text file directly to database without reading it by application.

## <a name="callStmt"></a>CALL Statement

* If stored procedure has any OUT or INOUT parameter, always call it with
parmeter markers only. i.e. pass the input values using bind params.

* Pass the Bind Params as objects only.

* If SP has result set to return, it will be returned in the array after out params. f.e. if SP has 2 out params and it returns 2 result set too, the result returned by query() or querySync() would be in the form [outValue1, outValue2, resultSet1, resultSet2]. Each resultset would be an array of row objects.

* [test-call-stmt.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-call-stmt.js) - Example using conn.querySync().

* [test-call-async.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-call-async.js) - Example using conn.query().

* [test-sp-resultset.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-sp-resultset.js) - Example using Out Params and Result Set using query() and querySync() APIs.

* [test-sp-resultset-execute.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-sp-resultset-execute.js) - Example using Out Params and Result Set using prepare() and execute() APIs.


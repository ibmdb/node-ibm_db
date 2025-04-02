# ibm_db API Documentation

## Database APIs

**APIs for creating and dropping Database using node.js application**
* [.createDbSync(dbName, connectionString, [options])](#-ibmdb-createdbsyncdbname-connectionstring--options)
* [.dropDBSync(dbName, connectionString)](#-ibmdb-dropdbsyncdbname-connectionstring--options)

**Global (ibmdb) APIs**

* [.open(connectionString [, options] [, callback])](#-1-ibmdb-openconnectionstring-options-callback)
* [.openSync(connectionString [,options])](#-2-ibmdb-opensyncconnectionstring-options)
* [.debug(value)](#-38-ibmdb-debugvalue)

**Database APIs**
* [.query(sqlQuery [, bindingParameters] [, callback])](#-3-database-querysqlquery--bindingparameters--callback)
* [.querySync(sqlQuery [, bindingParameters])](#-4-database-querysyncsqlquery--bindingparameters)
* [.queryStream(sqlQuery [, bindingParameters])](#-5-database-querystreamsqlquery--bindingparameters)
* [.queryResult(sqlQuery, [, bindingParameters] [, callback])](#-6-database-queryresultsqlquery--bindingparameters--callback)
* [.queryResultSync(sqlQuery [, bindingParameters])](#-7-database-queryresultsyncsqlquery--bindingparameters)
* [.close([callback])](#-8-database-closecallback)
* [.closeSync()](#-9-database-closesync)
* [.prepare(sql [, callback])](#-10-database-preparesql--callback)
* [.prepareSync(sql)](#-11-database-preparesyncsql)
* [.beginTransaction([callback])](#-28-database-begintransactioncallback)
* [.beginTransactionSync()](#-29-database-begintransactionsync)
* [.commitTransaction([callback])](#-30-database-committransactioncallback)
* [.commitTransactionSync()](#-31-database-committransactionsync)
* [.rollbackTransaction([callback])](#-32-database-rollbacktransactioncallback)
* [.rollbackTransactionSync()](#-33-database-rollbacktransactionsync)
* [.setIsolationLevel(isolationLevel)](#-34-database-setisolationlevelisolationlevel)
* [.executeFileSync(sqlFile, [delimiter], [outputFile])](#-39-database-executefilesyncsqlfile-delimiteroutputfile)
* [.executeFile(sqlFile, [delimiter], [outputFile])](#-40-database-executefilesqlfile-delimiter-outputfile)
* [.setAttr(attributeName, value [, callback])](#-41-database-setattrattributename-value--callback)
* [.setAttrSync(attributeName, value)](#-42-database-setattrsyncattributename-value)
* [.getInfo(infoType, [infoLength] [, callback])](#-43-database-getinfoinfotype-infolength--callback)
* [.getInfoSync(infoType, [infoLength])](#-44-database-getinfosyncinfotype-infolength)
* [.getTypeInfo(dataType [, callback])](#-45-database-gettypeinfodatatype--callback)
* [.getTypeInfoSync(dataType)](#-46-database-gettypeinfosyncdatatype)
* [.getFunctions(functionId, callback)](#-47-database-getfunctionsfunctionid-callback)
* [.getFunctionsSync(functionId)](#-48-database-getfunctionssyncfunctionid)

**ODBCStatement APIs**
* [.bind(bindingParameters [, callback])](#-12-odbcstatement-bindbindingparameters--callback)
* [.bindSync(bindingParameters)](#-13-odbcstatement-bindsyncbindingparameters)
* [.execute([bindingParameters] [, callback])](#-14-odbcstatement-executebindingparameters--callback)
* [.executeSync([bindingParameters])](#-15-odbcstatement-executesyncbindingparameters)
* [.executeNonQuery([bindingParameters] [, callback])](#-16-odbcstatement-executenonquerybindingparameters--callback)
* [.executeNonQuerySync([bindingParameters])](#-17-odbcstatement-executenonquerysyncbindingparameters)
* [close([closeOption] [, callback])](#-18-odbcstatement-closecloseoption--callback)
* [closeSync([closeOption])](#-19-odbcstatement-closesynccloseoption)

**ODBCResult APIs**
* [.fetch([option] [, callback])](#-20-odbcresult-fetchoption--callback)
* [.fetchSync([option])](#-21-odbcresult-fetchsyncoption)
* [.fetchAll([option] [, callback])](#-22-odbcresult-fetchalloption--callback)
* [.fetchAllSync([option])](#-23-odbcresult-fetchallsyncoption)
* [.getData([colNum] [, size] [, callback])](#-24-odbcresult-getdatacolnum--size--callback)
* [.getDataSync(colNum, size)](#-25-odbcresult-getdatasynccolnum-size)
* [.close([closeOption] [, callback])](#-26-odbcresult-closecloseoption--callback)
* [.closeSync([closeOption])](#-27-odbcresult-closesynccloseoption)
* [.getColumnNamesSync()](#-35-odbcresult-getcolumnnamessync)
* [.getColumnMetadataSync()](#-36-odbcresult-getcolumnmetadatasync)
* [.getSQLErrorSync()](#-37-odbcresult-getsqlerrorsync)


[**Connection Pooling APIs**](#connection-pooling-apis)
* [.open(connectionString [, callback])](#-1-pool-openconnectionstring--callback)
* [.openSync(connectionString)](#-2-pool-opensyncconnectionstring)
* [.close([callback])](#-3-pool-closecallback)
* [.closeSync()](#-4-pool-closesync)
* [.init(N, connStr)](#-5-pool-initn-connstr)
* [.initAsync(N, connStr [, callback])](#-6-pool-initasyncn-connstr--callback)
* [.setMaxPoolSize(N)](#-7-pool-setmaxpoolsizen)

## [**bindingParameters**](#bindingparameters)

## [**CALL Statement**](#call-statement)


### <a name="openApi"></a> 1) (ibmdb) .open(connectionString [, options] [, callback])

Open a connection to a database.

* **connectionString** - The connection string for your database.
    * For distributed platforms, the connection string is typically defined as:
    `DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=passwd`
    * For z/OS, the ODBC driver makes both local and remote connections using DSN, UID and PWD.
    The connection string is typically defined as: `DSN=dbname;UID=username;PWD=passwd`.  To
    connect to remote Db2 databases, the connectivity information will need to be set up in the
    Communications Database (CDB).  Please refer to scenario 1 in the following
    [article](https://www.ibm.com/developerworks/data/library/techarticle/0310chong/0310chong.html).
    For a complete list of supported configuration keywords,
    please refer to [CLI/ODBC configuration keywords](https://www.ibm.com/support/knowledgecenter/en/SSEPGG_11.1.0/com.ibm.db2.luw.apdv.cli.doc/doc/r0007964.html)
    * Selecting default schema is also supported using `CURRENTSCHEMA=schemaname;` (or CurrentSchema)
* **options** - _OPTIONAL_ - Object type. Can be used to avoid multiple
    loading of native ODBC library for each call of `.open`. Also, can be used
    to pass connectTimeout value and systemNaming(true/false) for i5/OS server.
* **callback** - _OPTIONAL_ - `callback (err, conn)`. If callback is not provided, a Promise will be returned.

```javascript
const ibmdb = require("ibm_db")
  , connStr = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=passwd";

ibmdb.open(connStr, function (err, conn) {
    if (err)
    {
      console.log(err);
      return;
    }
    conn.query("select 1 from sysibm.sysdummy1", function (err1, rows) {
      if (err1) console.log(err1);
      else console.log(rows);
      conn.close(function(err2) {
        if(err2) console.log(err2);
      });
    });
});

```

* <a name="SSLConnection"></a> **Secure Database Connection using SSL/TSL** - ibm_db supports secure connection to Database Server over SSL same as ODBC/CLI driver. If you have SSL Certificate from server or an CA signed certificate, just use it in connection string as below:

```javascript
connStr = "DATABASE=database;HOSTNAME=hostname;PORT=port;Security=SSL;SSLServerCertificate=<cert.arm_file_path>;PROTOCOL=TCPIP;UID=username;PWD=passwd;";
```
> Note the two extra keywords **Security** and **SSLServerCertificate** used in connection string. `SSLServerCertificate` should point to the SSL Certificate from server or an CA signed certificate. Also, `PORT` must be `SSL` port and not the TCP/IP port. Make sure Db2 server is configured to accept connection on SSL port else `ibm_db` will throw SQL30081N error.

> Value of `SSLServerCertificate` keyword must be full path of a certificate file generated for client authentication.
 It normally has `*.arm` or `*.cert` or `*.pem` extension. `ibm_db` do not support `*.jks` format file as it is not a
 certificate file but a Java KeyStore file, extract certificate from it using keytool and then use the cert file.

> `ibm_db` uses IBM ODBC/CLI Driver for connectivity and it do not support a `*.jks` file as keystoredb as `keystore.jks` is meant for Java applications.
 Note that `*.jks` file is a `Java Key Store` file and it is not an SSL Certificate file. You can extract SSL certificate from JKS file using below `keytool` command:
 ```
 keytool -exportcert -alias your_certificate_alias -file client_cert.cert -keystore  keystore.jks
 ```
 Now, you can use the generated `client_cert.cert` as the value of `SSLServerCertificate` in connection string.

> `ibm_db` supports only ODBC/CLI Driver keywords in connection string: https://www.ibm.com/docs/en/db2/11.5?topic=odbc-cliodbc-configuration-keywords

> Do not use keywords like `sslConnection=true` in connection string as it is a JDBC connection keyword and ibm_db
 ignores it. Corresponding ibm_db connection keyword for `sslConnection` is `Security` hence, use `Security=SSL;` in
 connection string instead.

* To connect to dashDB in IBM Cloud, use below connection string:
```
connStr = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=passwd;Security=SSL";
```
> We just need to add **Security=SSL** in connection string to have a secure connection against Db2 server in IBM Cloud.

**Note:** You can also create a KeyStore DB using GSKit command line tool and use it in connection string along with other keywords as documented in [DB2 Infocenter](http://www.ibm.com/support/knowledgecenter/en/SSEPGG_11.5.0/com.ibm.db2.luw.admin.sec.doc/doc/t0053518.html).

If you have created a KeyStore DB using GSKit using password or you have got *.kdb file with *.sth file, use
connection string in below format:
```
connStr = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=db2pwd;" +
          "Security=SSL;SslClientKeystoredb=C:/client.kdb;SSLClientKeystash=C:/client.sth;";
OR,
connStr = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=db2pwd;" +
          "Security=SSL;SslClientKeystoredb=C:/client.kdb;SSLClientKeystoreDBPassword=kdbpasswd;";
```

> If you have downloaded `IBMCertTrustStore` from IBM site, ibm_db will not work with it; you need to
 download `Secure Connection Certificates.zip` file that comes for IBM DB2 Command line tool(CLP).
 `Secure Connection Certificates.zip` has *.kdb and *.sth files that should be used as the value of
 `SSLClientKeystoreDB` and `SSLClientKeystash` in connection string.

### <a name="openSyncApi"></a> 2) (ibmdb) .openSync(connectionString [,options])

Synchronously open a connection to a database.

* **connectionString** - The connection string for your database
* **options** - _OPTIONAL_ - Object type. Can be used to avoid multiple
    loading of native ODBC library for each call of `.open`. Also, can be used
    to pass connectTimeout value and systemNaming value for i5/OS server.

```javascript
const ibmdb = require("ibm_db"),
	connString = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password;";

try {
      const option = { connectTimeout : 40, systemNaming : true };// Connection Timeout after 40 seconds.
      const conn = ibmdb.openSync(connString, option);
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

### <a name="queryApi"></a> 3) (Database) .query(sqlQuery [, bindingParameters] [, callback])

Issue an asynchronous SQL query to the database which is currently open.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue, "ArraySize": n}. noResults accepts only true or false values.
If true - query() will not return any result. "sql" field is mandatory in Object, others are _OPTIONAL_.
For **Array Insert**, `ArraySize` must be passed and sqlQuery must be an object. Check [test-array-insert.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-array-insert.js) for example.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`. bindingParameters in sqlQuery Object takes precedence over it.

* **callback** - _OPTIONAL_ - `callback (err, rows, sqlca)`.  If no callback is provided, query() will return a Promise.

```javascript
const ibmdb = require("ibm_db")
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
Example for Array Insert:
```javascript
  const param1 = {ParamType:"ARRAY", DataType:1, Data:[4,5,6,7,8]};
  const param2 = {ParamType:"ARRAY", DataType:"DOUBLE", Data:[4.1,5.3,6.14,7,8.3]};
  const param3 = {ParamType:"ARRAY", DataType:1, Data:[0,1,false,true,0]};
  const namearr = ["Row 10", "Row 203456", "Row 30", "Row 40", "Last Row"];

  const param4 = {ParamType:"ARRAY", DataType:1, Data:namearr, Length:8};
  // *** Use "Length: <maxDataLen>" in param Object for unequal size of data.
  // Default value is the length of first member of Array.

  const queryOptions = {sql:"insert into arrtab values (?, ?, ?, ?)",
                      params: [param1, param2, param3, param4],
                      ArraySize:5};

  conn.querySync("create table arrtab (c1 int, c2 double, c3 boolean, c4 varchar(10))");
  conn.query(queryOptions, function(err, result) {
    if(err) console.log(err);
    else {
      const data = conn.querySync("select * from arrtab");
      console.log("\nSelected data for table ARRTAB =\n", data);
    }
  });
```

### <a name="querySyncApi"></a> 4) (Database) .querySync(sqlQuery [, bindingParameters])

Synchronously issue a SQL query to the database that is currently open.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue, "ArraySize": n}. noResults accepts only true or false values.
If true - query() will not return any result. If noResults is true for CALL statement, querySync returns only OutParams. "sql" field is mandatory in Object, others are optional.
For **Array Insert**, `ArraySize` must be passed and sqlQuery must be an object. Check [test-array-insert.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-array-insert.js) for example.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn) {

  //blocks until the query is completed and all data has been acquired
  const rows = conn.querySync("select * from customers fetch first 10 rows only");

  console.log(rows);
});
```
Example for Array Insert:
```javascript
  const param1 = {ParamType:"ARRAY", DataType:1, Data:[4,5,6,7,8]};
  const param2 = {ParamType:"ARRAY", DataType:"DOUBLE", Data:[4.1,5.3,6.14,7,8.3]};
  const param3 = {ParamType:"ARRAY", DataType:1, Data:[0,1,false,true,0]};
  const namearr = ["Row 10", "Row 20", "Row 30", "Row 40", "Last Row"];

  const param4 = {ParamType:"ARRAY", DataType:1, Data:namearr, Length:8};
  // *** Use "Length: <maxDataLen>" in param Object for unequal size of data.
  // Default value is the length of first member of Array.

  const queryOptions = {sql:"insert into arrtab values (?, ?, ?, ?)",
                      params: [param1, param2, param3, param4],
                      ArraySize:5};

  conn.querySync("create table arrtab (c1 int, c2 double, c3 boolean, c4 varchar(10))");
  conn.querySync(queryOptions);
```

### <a name="queryStreamApi"></a> 5) (Database) .queryStream(sqlQuery [, bindingParameters])

Synchronously issue a SQL query to the database that is currently open and returns
a Readable stream. Application can listen for the events emitted by returned stream
and take action.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue, "ArraySize": n}.
noResults accepts only true or false values. If true - query() will not return any result. "sql" field is mandatory in Object, others are optional.
For **Array Insert**, `ArraySize` must be passed and sqlQuery must be an object. Check [test-array-insert.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-array-insert.js) for example.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err, conn)
{
    const stream = conn.queryStream("select 1 from sysibm.sysdummy1");

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

### <a name="queryResultApi"></a> 6) (Database) .queryResult(sqlQuery, [, bindingParameters] [, callback])

Issue an asynchronous SQL query to the database which is currently open and return `(err, result, outparams)` to callback function. `result` is ODBCResult object. Using `result`, call one of the `result.fetch*` APIs to retrieve rows.  Call `result.getColumnMetadataSync()` to get meta data info. Execute `result.closeSync()` once done with the `result` object.
`query` returns all the rows on call, but `queryResult` returns an `ODBCResult` object and rows need to be fetched by the caller.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue, "ArraySize": n}.
noResults accepts only true or false values. If true - queryResult() will not return any result object and value of result will be null.
"sql" field is mandatory in Object, others are _OPTIONAL_.
For **Array Insert**, `ArraySize` must be passed and sqlQuery must be an object. Check [test-array-insert.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-array-insert.js) for example.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any ? characters (called parameter marker) in `sqlQuery`. bindingParameters in sqlQuery Object takes precedence over it.

* **callback** - _OPTIONAL_ - `callback (err, result, outparams)`.
outparams is returned only for CALL statement with OUT parameters. Any resultset expected from SP should get retrieved using the `result.fetch*` apis.  If `callback` is not provided, queryResult() will return a Promise of `[result, outparams]`. `result` is an ODBCResult object that can be used to fetch rows.

```javascript
const ibmdb = require("ibm_db")
	, cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password;"
	;
ibmdb.open(cn, function (err, conn) {
    if (err) return console.log(err);
    const query = 'select creator, name from sysibm.systables where 1 = ?';
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
**Note:** Once you are done with the `result` object, you must close it to avoid an error when the Javascript garbage collector frees it. Not calling `result.close()` or `result.closeSync()` may cause an invalid handle error in the application or no data.

### <a name="queryResultSyncApi"></a> 7) (Database) .queryResultSync(sqlQuery [, bindingParameters])

Synchronously issue a SQL query to the database that is currently open and return a result object to the callback function on success. In the case of CALL statement with OUT parameters, it returns an Array of `[result, outparams]`. `result` is an ODBCResult object that can be used to fetch rows.

`querySync` returns all the rows on call, but `queryResultSync` returns an `ODBCResult` object and rows need to be fetched by the caller.

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue, "ArraySize": n}.
noResults accepts only true or false values. If true - the value of `result` will be null. "sql" field is mandatory in Object, others are optional.
For **Array Insert**, `ArraySize` must be passed and sqlQuery must be an object. Check [test-array-insert.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-array-insert.js) for example.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn) {
  if (err) return console.log(err);
  const query = 'select creator, name from sysibm.systables';
  const result = conn.queryResultSync(query);
  console.log("data = ", result.fetchAllSync());
  console.log("metadata = ", result.getColumnMetadataSync());
  result.closeSync(); // Must call to free to avoid application error.
  conn.closeSync();
});
```
**Note:** Once you are done with the `result` object, you must close it to avoid an error when the Javascript garbage collector frees it. Not calling `result.close()` or `result.closeSync()` may cause an invalid handle error in the application or no data.

In case of a CALL statement with OUT params, check whether result[0] is an object or not.

### <a name="closeApi"></a> 8) (Database) .close([callback])

Close the currently opened database.

* **callback** - _OPTIONAL_ - `callback (err)`.  If callback is not provided, a Promise will be returned.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function (err, conn) {
	if (err) {
		return console.log(err);
	}

	//we now have an open connection to the conn
	conn.close(function (err) {
		console.log("the database connection is now closed");
	});
});
```

### <a name="closeSyncApi"></a> 9) (Database) .closeSync()

Synchronously close the currently opened database.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err, conn) {
  if (err) return console.log(err);

  //Blocks until the connection is closed
  conn.closeSync();
});

const conn = ibmdb.openSync(connString, option);
conn.closeSync();
```

### <a name="prepareApi"></a> 10) (Database) .prepare(sql [, callback])

Prepare a statement for execution.

* **sql** - SQL string to prepare
* **callback** - _OPTIONAL_ - `callback (err, stmt)`.  If callback is not provided, a Promise will be returned.

Returns an `ODBCStatement` object.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
  conn.prepare("insert into hits (col1, col2) VALUES (?, ?)", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }

    //Bind and Execute the statement asynchronously
    stmt.execute(['something', 42], function (err, result) {
      if( err ) console.log(err);
      else result.closeSync();

      //Close the connection
	  conn.close(function(err){});
    });
  });
});
```

### <a name="prepareSyncApi"></a> 11) (Database) .prepareSync(sql)

Synchronously prepare a statement for execution.

* **sql** - SQL string to prepare

Returns an `ODBCStatement` object.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err,conn){
  const stmt = conn.prepareSync("select * from employee where empid = ?");

  //Bind and Execute the statement asynchronously
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

### <a name="bindApi"></a> 12) (ODBCStatement) .bind(bindingParameters [, callback])

Binds the parameters for the prepared statement.

* **bindingParameters** - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be array or object itself. Check [bindingParameters](#bindParameters) doc for detail.
* **callback** - _OPTIONAL_ - `callback (err)`. If callback is not provided, a Promise will be returned.

* For **ARRAY INSERT** - Each value should be an array of size passed as `ArraySize` in query() APIs or equal to the value of attribute SQL_ATTR_PARAMSET_SIZE set using setAttr() APIs for prepared statement.

### <a name="bindSyncApi"></a> 13) (ODBCStatement) .bindSync(bindingParameters)

Binds the parameters for the prepared statement synchronously. If `bindSync()` is used, then there is no need to pass `bindingParameters` to the next `execute()` or `executeSync()` statement.

* **bindingParameters** - An array of values that will be bound to any '?' characters in the prepared SQL statement. Values can be an array or object. See [bindingParameters](#bindParameters) for detail.

* For **ARRAY INSERT** - Each value should be an array of size passed as `ArraySize` in query() APIs or equal to the value of attribute SQL_ATTR_PARAMSET_SIZE set using setAttr() APIs for prepared statement.

### <a name="executeApi"></a> 14) (ODBCStatement) .execute([bindingParameters] [, callback])

Execute a prepared statement.

* **bindingParameters** - OPTIONAL - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be an array or object. See [bindingParameters](#bindParameters) for detail.
* **callback** - _OPTIONAL_ - `callback (err, result, outparams)`.  If callback is not provided, a Promise will be returned.
outparams - will have result for INOUT and OUTPUT parameters of Stored Procedure.
* For **ARRAY INSERT** - Statement attribute SQL_ATTR_PARAMSET_SIZE must be set before calling execute() API.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
  conn.querySync("create table mytab (id int, photo BLOB(30K))");
  conn.prepare("insert into mytab (id, photo) VALUES (?, ?)", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }

    // Create params object
    const img = {ParamType:"FILE", DataType: "BLOB", "Data": "smile.jpg"};

    //Bind and Execute the statement asynchronously
    stmt.execute([ 42, img ], function (err, result) {
      if( err ) console.log(err);
      else result.closeSync();

      //Close the connection
      stmt.close(function(err){
        if(err){
          console.log(err)
        }
        conn.close(function(err){});
      });
    });
  });
});
```

### <a name="executeSyncApi"></a> 15) (ODBCStatement) .executeSync([bindingParameters])

Execute a prepared statement synchronously.

* **bindingParameters** - OPTIONAL - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be an array or object. See [bindingParameters](#bindParameters) for detail. Instead of passing bindingParameters to executeSync(), parameters can also be bound using the `bind()` and `bindSync()` APIs.
* For **ARRAY INSERT** - Statement attribute SQL_ATTR_PARAMSET_SIZE must be set before calling execute() API.

Returns an `ODBCResult` object. If the prepared statement is a stored procedure with INOUT or OUT parameters, executeSync() returns an array of two elements in the form [stmt, outparams]. The first element of the array is an `ODBCResult` object and second element is an `Array` of INOUT and OUTPUT parameters in sequence.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
  const stmt = conn.prepareSync("select empname from emptable where empid = ?");

  //Bind and Execute the statment asynchronously
  const result = stmt.executeSync([142]);
  const data = result.fetchAllSync({fetchMode:3}); // Fetch data in Array mode.
  console.log(data);
  result.closeSync();
  stmt.closeSync();

  //Close the connection
  conn.close(function(err){});
});
```

### <a name="executeNonQueryApi"></a> 16) (ODBCStatement) .executeNonQuery([bindingParameters] [, callback])

Executes a non query prepared statement and returns the number of rows affected in a table by the statement.

* **bindingParameters** - OPTIONAL - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be an array or object. See [bindingParameters](#bindParameters) for detail.
* **callback** - _OPTIONAL_ - `callback (err, affectedRowCount)`.  If callback is not provided, a Promise will be returned.
* For **ARRAY INSERT** - Statement attribute SQL_ATTR_PARAMSET_SIZE must be set before calling execute() API.

It returns the number of rows in a table that were affected by an UPDATE, an INSERT, a DELETE, or a MERGE statement issued against the table, or a view based on the table. If no rows are affected, it returns -1.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
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

      //Close the stmt and connection
      stmt.close();
      conn.close(function(err){});
    });
  });
});
```

### <a name="executeNonQuerySyncApi"></a> 17) (ODBCStatement) .executeNonQuerySync([bindingParameters])

Executes a non query prepared statement synchronously and returns the number of rows affected in a table by the statement.

* **bindingParameters** - OPTIONAL - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be an array or object. See [bindingParameters](#bindParameters) for detail.
* For **ARRAY INSERT** - Statement attribute SQL_ATTR_PARAMSET_SIZE must be set before calling execute() API.

It returns the number of rows in a table that were affected by an UPDATE, an INSERT, a DELETE, or a MERGE statement issued against the table, or a view based on the table. If no rows are affected, it returns -1.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
  conn.querySync("create table mytab (id int, text varchar(30))");
  conn.prepare("insert into mytab (id, text) VALUES (?, ?)", function (err, stmt) {
    if (err) {
      console.log(err);
      return conn.closeSync();
    }

    //Bind and Execute the statment asynchronously
    const rowCount = stmt.executeNonQuerySync([ 42, 'hello world' ]);
    console.log("Affected rows = " + rowCount);

    //Close the stmt and connection
    stmt.closeSync();
    conn.closeSync();
  });
});
```

### <a name="stmtCloseApi"></a> 18) (ODBCStatement) close([closeOption] [, callback])

Close the currently opened statement object and free resources.

* **callback** - _OPTIONAL_ - `callback (err)`.  If callback is not provided, a Promise will be returned.

```javascript
    stmt.close(function(err) {
        if(err) console.log(err);
    });
    //OR
    await stmt.close();
```

### <a name="stmtCloseSyncApi"></a> 19) (ODBCStatement) closeSync([closeOption])

Synchronously close the currently opened statement object and free resources.

```javascript
    stmt.closeSync();
```

### <a name="fetchApi"></a> 20) (ODBCResult) .fetch([option] [, callback])

Fetch a row of data from an ODBCResult object asynchronously.

* **option** - _OPTIONAL_ - Object type.
    * fetchMode - Format of the returned row data. By default, the row data will be returned in object form. option = {fetchMode:3} or option = {fetchMode: ibmdb.FETCH_ARRAY} will return row data in array form. Default value of fetchMode is ibmdb.FETCH_OBJECT.
    * When option = {fetchMode : 0} or {fetchMode: ibmdb.FETCH_NODATA} is used, the fetch() API will not return any results, and the application needs to call the result.getData() or result.getDataSync() APIs to retrieve data for a column.

* **callback** - _OPTIONAL_ - `callback (err, row)`. If callback is not provided, a Promise will be returned.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
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
      result.fetch((err, row) => {
          if(err) { console.log(err); }
          else {
            console.log("Row1 = ", row);
            result.fetch({fetchMode:ibmdb.FETCH_ARRAY}).then(row => {
              console.log("Row2 = ", row);
              result.closeSync();
              conn.querySync("drop table hits");
              //Close the connection
              conn.close(function(err){console.log("Connection Closed.");});
            }).catch(err => console.log(err));
          }
      });
    });
  });
});
```

### <a name="fetchSyncApi"></a> 21) (ODBCResult) .fetchSync([option])

Fetch a row of data from the ODBCResult object synchronously.

* **option** - _OPTIONAL_ - Object type.
    * fetchMode - Format of the returned row data. By default, the row data will be returned in object form. option = {fetchMode:3} or option = {fetchMode: ibmdb.FETCH_ARRAY} will return row data in array form. Default value of fetchMode is ibmdb.FETCH_OBJECT.
    * When option = {fetchMode : 0} or {fetchMode: ibmdb.FETCH_NODATA} is used, the fetch() API will not return any results, and the application needs to call the result.getData() or result.getDataSync() APIs to retrieve data for a column.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int)");
  conn.querySync("insert into hits values ('something', 42)");
  conn.querySync("insert into hits values ('für', 43)");
  const stmt = conn.prepareSync("select * from hits");
  const result = stmt.executeSync();
  const data = 0;
  while( data = result.fetchSync({fetchMode:3}) ) {
    console.log(data);
  }
  result.closeSync();
  conn.querySync("drop table hits");
  conn.closeSync();
});
```

### <a name="fetchAllApi"></a> 22) (ODBCResult) .fetchAll([option] [, callback])

Fetch all rows from ODBCResult object asynchronously for the executed statement.

* **option** - _OPTIONAL_ - Object type.
    * fetchMode - Format of the returned row data. By default, the row data will be returned in object form. option = {fetchMode:3} or option = {fetchMode: ibmdb.FETCH_ARRAY} will return row data in array form. Default value of fetchMode is ibmdb.FETCH_OBJECT.

* **callback** - _OPTIONAL_ - `callback (err, data, noOfColumns)`.  If callback is not provided, a Promise will be returned.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
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

### <a name="fetchAllSyncApi"></a> 23) (ODBCResult) .fetchAllSync([option])

Fetch all rows from ODBCResult object Synchronously for the executed statement.

  * **option** - Format of the returned row data. By default, the row data will be returned in object form. option = {fetchMode:3} or option = {fetchMode: ibmdb.FETCH_ARRAY} will return row data in array form. Default value of fetchMode is ibmdb.FETCH_OBJECT.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int)");
  conn.querySync("insert into hits values ('something', 42)");
  conn.querySync("insert into hits values ('für', 43)");
  const stmt = conn.prepareSync("select * from hits");
  const result = stmt.executeSync();
  const data = result.fetchAllSync();
  console.log("Fetched data = ", data);
  result.closeSync();
  conn.querySync("drop table hits");
  conn.closeSync();
});
```
For example of prepare once and execute many times with above fetch APIs, please see test file [test-fetch-apis.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-fetch-apis.js).

### <a name="getDataApi"></a> 24) (ODBCResult) .getData([colNum] [, size] [, callback])

Retrive data for colNum of specified size from ODBCResult object asynchronously.

* **colNum** - Integer - Column Number in the resultset starting from 1.

* **size** - Integer -  Size of the data being retrieved. Ignored for fixed length data.

* **callback** - _OPTIONAL_ - `callback (err, row)`. When no `callback` function is passed, getData() will return Promise. If callback is not provided, a Promise will be returned.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int)");
  conn.querySync("insert into hits values ('something', 42)");
  conn.querySync("insert into hits values ('für', 43)");
  const stmt = conn.prepareSync("select * from hits");
  const result = stmt.executeSync();
  result.fetch({fetchMode:0})
      .then(() => {
        return result.getData(1, 4);
      }).then(data => {
        console.log(data);
        return result.getData(1, 5);
      }).then(data => {
        console.log(data);
        return result.getData(2, 5);
      }).then(data => {
        console.log(data);
        return result.getData(3, 5);
      }).then(data => {
        console.log(data);
      }).catch(err => console.log(err));
```
See test file [test-fetch-apis.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-fetch-apis.js) for detail example.

### <a name="getDataSyncApi"></a> 25) (ODBCResult) .getDataSync(colNum, size)

Retrive data for colNum of specified size from ODBCResult object synchronously.

* **colNum** - Integer - Column Number in the resultset starting from 1.

* **size** - Integer -  Size of the data being retrieved. Ignored for fixed length data.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn,function(err, conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int)");
  conn.querySync("insert into hits values ('something', 42)");
  conn.querySync("insert into hits values ('für', 43)");
  const stmt = conn.prepareSync("select * from hits");
  const result = stmt.executeSync();
  console.log(result.fetchSync({fetchMode:0}));
  console.log("First Row Data = ");
  console.log(result.getDataSync(1, 4));
  console.log(result.getDataSync(1, 5));
  console.log(result.getDataSync(2, 5));
  console.log(result.getDataSync(3, 5));
  result.fetchSync({fetchMode:0});
  console.log("Second Row Data = ");
  console.log(result.getDataSync(1, 4));
  console.log(result.getDataSync(1, 5));
  console.log(result.getDataSync(2, 5));
  console.log(result.getDataSync(3, 5));
  result.closeSync();
  conn.closeSync();
}
```
See test file [test-fetch-apis.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-fetch-apis.js) for detail example.

### <a name="resultCloseApi"></a> 26) (ODBCResult) close([closeOption] [, callback])

Close the currently opened ODBC Result object and free resources.

* **callback** - _OPTIONAL_ - `callback (err)`. If callback is not provided, a Promise will be returned.

```javascript
    result.close(function(err) {
        if(err) console.log(err);
    });
    //OR
    await result.close();
```

### <a name="resultCloseSyncApi"></a> 27) (ODBCResult) closeSync([closeOption])

Synchronously close the currently opened ODBC Result object and free resources.

```javascript
    result.closeSync();
```

### <a name="beginTransactionApi"></a> 28) (Database) .beginTransaction([callback])

Begin a transaction

* **callback** - _OPTIONAL_ - `callback (err)`. If callback is not provided, a Promise will be returned.

### <a name="beginTransactionSyncApi"></a> 29) (Database) .beginTransactionSync()

Synchronously begin a transaction

### <a name="commitTransactionApi"></a> 30) (Database) .commitTransaction([callback])

Commit a transaction

* **callback** - _OPTIONAL_ - `callback (err)`, If callback is not provided, a Promise will be returned.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err, conn) {

  conn.beginTransaction(function (err) {
    if (err) {
      //could not begin a transaction for some reason.
      console.log(err);
      return conn.closeSync();
    }

    const result = conn.querySync("insert into customer (customerCode) values ('stevedave')");

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

### <a name="commitTransactionSyncApi"></a> 31) (Database) .commitTransactionSync()

Synchronously commit a transaction

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err, conn) {

  conn.beginTransaction(function (err) {
    if (err) {
      //could not begin a transaction for some reason.
      console.log(err);
      return conn.closeSync();
    }

    const result = conn.querySync("insert into customer (customerCode) values ('stevedave')");

    conn.commitTransactionSync();

    console.log(conn.querySync("select * from customer where customerCode = 'stevedave'"));

     //Close the connection
    conn.closeSync();
  });
});
```

### <a name="rollbackTransactionApi"></a> 32) (Database) .rollbackTransaction([callback])

Rollback a transaction

* **callback** - _OPTIONAL_ - `callback (err)`. If callback is not provided, a Promise will be returned.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err, conn) {

  conn.beginTransaction(function (err) {
    if (err) {
      //could not begin a transaction for some reason.
      console.log(err);
      return conn.closeSync();
    }

    const result = conn.querySync("insert into customer (customerCode) values ('stevedave')");

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

### <a name="rollbackTransactionSyncApi"></a> 33) (Database) .rollbackTransactionSync()

Synchronously rollback a transaction

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err, conn) {

  conn.beginTransaction(function (err) {
    if (err) {
      //could not begin a transaction for some reason.
      console.log(err);
      return conn.closeSync();
    }

    const result = conn.querySync("insert into customer (customerCode) values ('stevedave')");

    conn.rollbackTransactionSync();

    console.log(conn.querySync("select * from customer where customerCode = 'stevedave'"));

     //Close the connection
    conn.closeSync();
  });
});
```

### <a name="setIsolationLevelApi"></a> 34) (Database) .setIsolationLevel(isolationLevel)

Synchronously sets the default isolation level passed as argument. It is only applicable when the default isolation level is used. It will have no effect if the application has specifically set the isolation level for a transaction.

* **isolationLevel:** An integer representing the isolation level to be set. Its value must be one of: 1|2|4|8|32. For details, see [doc](https://www.ibm.com/support/knowledgecenter/en/SSEPGG_11.1.0/com.ibm.db2.luw.apdv.cli.doc/doc/r0008832.html).

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.open(cn, function(err, conn) {
  conn.setIsolationLevel(2);  // SQL_TXN_READ_COMMITTED
  conn.setIsolationLevel(4); // SQL_TXN_REPEATABLE_READ
  conn.querySync("create table mytab1 (c1 int, c2 varchar(10))");
});
```

### <a name="getColumnNamesSyncApi"></a> 35) (ODBCResult) .getColumnNamesSync()

Synchronously retrieve the name of columns returned by the resulset.

```javascript
  conn.querySync("insert into mytab1 values ( 5, 'abc')");
  conn.prepare("select * from mytab1", function (err, stmt) {
    stmt.execute(function(err, result) {
      console.log("Column Names = ", result.getColumnNamesSync());
      result.closeSync(); conn.closeSync();
    });
  });
```

### <a name="getColumnMetadataSyncApi"></a> 36) (ODBCResult) .getColumnMetadataSync()

Synchronously retrieve the metadata about columns returned by the resulset.

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

### <a name="getSQLErrorSyncApi"></a> 37) (ODBCResult) .getSQLErrorSync()

Synchronously retrieve the sqlerror message and codes for last instruction executed on a statement handle using SQLGetDiagRec ODBC API.

```javascript
  conn.querySync("insert into mytab1 values ( 5, 'abc')");
  conn.prepare("select * from mytab1", function (err, stmt) {
    stmt.execute(function(err, result) {
      console.log("Fetched Data = ", result.fetchAllSync() );
      const problem = result.getSQLErrorSync();
      if (problem.sqlcode < 0) { // This sqlcode is negative and is therefore an error
        console.log("SQLError = ", problem);
      } else if (problem.sqlcode > 0) { // This sqlcode is positive and is therefore a warning
        console.log("SQLWarning = ", problem);
      }
      result.closeSync();
      conn.closeSync();
    });
  });
```

### <a name="enableDebugLogs"></a> 38) (ibmdb) .debug(value)

Enable console logs. debug(true) do not log params that may have sensitive data. Support for debug(2) added to dump bind params.

* **value** - true/false/2. Any truthy value enables debug mode.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

ibmdb.debug(true);  // **==> ENABLE CONSOLE LOGS, but do not log params. <==**
ibmdb.debug(2);     // **==> ENABLE CONSOLE LOGS and log parameter values too if passed. <==**

ibmdb.open(cn, function (err, conn) {
    if (err)
    {
        console.log(err);
        return;
    }
    conn.query("select 1 from sysibm.sysdummy1", function (err1, rows) {
        if (err1) console.log(err1);
        else console.log(rows);

        ibmdb.debug(false);  // Disable console logs.

        conn.close(function(err2) {
            if(err2) console.log(err2);
        });
    });
});
```

### <a name="executeFileSyncApi"></a> 39) (Database) .executeFileSync(sqlFile, [delimiter], [outputFile])

Synchronously issue multiple SQL query from the file to the database that is currently open.

* **sqlFile** - sqlFile input should be the full path of the file. sqlFile can be an Object in the form { "sql": sqlFile, "delimiter": delimiter, "outputfile": outputfile }.
"sql" field is mandatory in Object.

* **delimiter** - _OPTIONAL_ - Delimiter separates multiple queries in `sqlFile`. Defaults to `;`.

* **outputfile** - _OPTIONAL_ - Outputfile should be the full path of the file and only select queries data will be copied to outputfile split by the delimiter.
If the outputfile already exists it will be overwritten. If the outputfile is not provided the result will be returned split by the delimiter.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn){
    conn.executeFileSync('sample2.txt', '%','out.txt');
    const rows = conn.executeFileSync('sample2.txt', '%');
    console.log(rows)
});
```

### <a name="executeFileApi"></a> 40) (Database) .executeFile(sqlFile, [delimiter], [outputFile])

Asynchronously issue multiple SQL query from the file to the database that is currently open.

* **sqlFile** - sqlFile input should be the full path of the file. sqlFile can be an Object in the form { "sql": sqlFile, "delimiter": delimiter, "outputfile": outputfile }.
"sql" field is mandatory in Object.

* **delimiter** - _OPTIONAL_ - Delimiter separates multiple queries in `sqlFile`. Defaults to `;`.

* **outputfile** - _OPTIONAL_ - Outputfile should be the full path of the file and only select queries data will be copied to outputfile split by the delimiter.
If the outputfile already exists it will be overwritten. If the outputfile is not provided the result will be returned split by the delimiter.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn){
   conn.executeFile('sample3.txt', '@', 'out.txt', function (err, rows) {
        if (err) {
            console.log(err);
        } else {
            console.log(rows);
        }
    });
    conn.executeFile('sample3.txt', '@', function (err, rows) {
        if (err) {
            console.log(err);
        } else {
            console.log(rows);
        }
    });
});
```

### <a name="setAttrApi"></a> 41) (Database) .setAttr(attributeName, value [, callback])

Set connection and statement level attributes asynchronously. It requires attributeName and corresponding value.
`conn.setAttr()` - sets connection level attributes post connection.
`stmt.setAttr()` - sets statement level attributes post creation of statement handle.

```javascript
await conn.setAttr("SQL_ATTR_INFO_USERID", 'appuser');

stmt.setAttr(ibmdb.SQL_ATTR_PARAMSET_SIZE, 4, function(err, result) {
    if(err) { console.log(err); stmt.closeSync(); }
    else { ... }
});
```

### <a name="setAttrSyncApi"></a> 42) (Database) .setAttrSync(attributeName, value)

Set connection and statement level attributes synchronously. It requires attributeName and corresponding value.
`conn.setAttrSync()` - sets connection level attributes post connection.
`stmt.setAttrSync()` - sets statement level attributes post creation of statement handle.

```javascript
conn.setAttrSync(ibmdb.SQL_ATTR_INFO_APPLNAME, 'mynodeApp');

const err = stmt.setAttrSync(ibmdb.SQL_ATTR_PARAMSET_SIZE, 5);
err = stmt.setAttrSync(ibmdb.SQL_ATTR_QUERY_TIMEOUT, 50);
err = stmt.setAttrSync(3, 2); //SQL_ATTR_MAX_LENGTH = 3
```

### <a name="getInfoApi"></a> 43) (Database) .getInfo(infoType, [infoLength] [, callback])

Asynchronously retrieve the general information about the database management system (DBMS) that the application is connected to. It also retrives the information about ODBC driver used for connection.

* **infoType** - The type of information that is required. The possible values for this argument are described in [Information returned by SQLGetInfo()](https://www.ibm.com/support/knowledgecenter/SSEPGG_11.5.0/com.ibm.db2.luw.apdv.cli.doc/doc/r0000615.html#r0000615__tbginfo). The value for this argument should be an integer value if macro is not defined in `ibm_db/lib/climacros.js` file.

* **infoLength** - _OPTIONAL_ - Length of the string value to be retrieved. If not provided, getInfo() can return a string value of maximum size 255 bytes.

* **callback** - _OPTIONAL_- `callback (error, value)`. If callback is not provided, a Promise will be returned.

Depending on the type of information that is being retrieved, 2 types of information can be returned:
  - String value
  - Number value

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn) {
    conn.getInfo(ibmdb.SQL_DBMS_NAME, function(error, data) {
      if(error) console.log(error);
      else console.log("SQL_DBMS_NAME(Server Type) = ", data);
      conn.closeSync();
    });
});
```

### <a name="getInfoSyncApi"></a> 44) (Database) .getInfoSync(infoType, [infoLength])

Synchronously retrieve the general information about the database management system (DBMS) that the application is connected to. It also retrives the information about ODBC driver used for connection.

* **infoType** - The type of information that is required. The possible values for this argument are described in [Information returned by SQLGetInfo()](https://www.ibm.com/support/knowledgecenter/SSEPGG_11.5.0/com.ibm.db2.luw.apdv.cli.doc/doc/r0000615.html#r0000615__tbginfo). The value for this argument should be an integer value if macro is not defined in `ibm_db/lib/climacros.js` file.

* **infoLength** - _OPTIONAL_ - Length of the string value to be retrieved. If not provided, getInfo() can return a string value of maximum size 255 bytes.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn)
{
    let serverVersion = conn.getInfoSync(ibmdb.SQL_DBMS_VER);
    console.log("SQL_DBMS_VER(Server Version) = ", serverVersion);
    conn.closeSync();
});
```

### <a name="getTypeInfoApi"></a> 45) (Database) .getTypeInfo(dataType [, callback])

Asynchronously retrieve the information about the SQL data types that are supported by the connected database server.
If `ibmdb.SQL_ALL_TYPES` is specified, information about all supported data types would be returned in ascending order by `TYPE_NAME`. All unsupported data types would be absent from the result set.

* **dataType** - The SQL data type being queried. The supported values for this argument are described in [SQLGetTypeInfo function (CLI) - Get data type information](https://www.ibm.com/docs/en/db2/11.5?topic=functions-sqlgettypeinfo-function-get-data-type-information). The value for this argument should be an integer value if macro is not defined in `ibm_db/lib/climacros.js` file.

* **callback** - _OPTIONAL_ - `callback (error, result)`. If callback is not provided, a Promise will be returned.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn) {
    conn.getTypeInfo(ibmdb.SQL_BLOB, function(error, result) {
      if(error) console.log(error);
      else console.log("SQL_BLOB Data Type Info = ", result);
      conn.closeSync();
    });
});

async function main()
{
    let conn = await ibmdb.open(cn);
    let data = await conn.getTypeInfo(ibmdb.SQL_ALL_TYPES);
    console.log("All supported data types info = ", data);
    await conn.close();
}
```

### <a name="getTypeInfoSyncApi"></a> 46) (Database) .getTypeInfoSync(dataType)

Synchronously retrieve the information about the SQL data types that are supported by the connected database server.
If `ibmdb.SQL_ALL_TYPES` is specified, information about all supported data types would be returned in ascending order by `TYPE_NAME`. All unsupported data types would be absent from the result set.

* **dataType** - The SQL data type being queried. The supported values for this argument are described in [SQLGetTypeInfo function (CLI) - Get data type information](https://www.ibm.com/docs/en/db2/11.5?topic=functions-sqlgettypeinfo-function-get-data-type-information). The value for this argument should be an integer value if macro is not defined in `ibm_db/lib/climacros.js` file.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn) {
    let result = conn.getTypeInfoSync(ibmdb.SQL_BIGINT);
    console.log("SQL_BIGINT Data Type Info = ", result);
    conn.closeSync();
});
```

### <a name="getFunctionsApi"></a> 47) (Database) .getFunctions(functionId, callback)

Asynchronously determines whether a specific CLI or ODBC function is supported. This allows applications to adapt to varying levels of support when connecting to different database servers.

* **functionId** - The value for a function being queried. The value for this argument should be an integer value if macro is not defined in `ibm_db/lib/climacros.js` file.

* **callback** - `callback (error, value)`

 - value will have only two values: true or false if a valid function id is passed. For ibmdb.ALLFUNCTIONS or 0, it returns an object of all supported functions with true/false value.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn) {
    conn.getFunctions(ibmdb.SQLCONNECT, function(error, value) {
      if(error) console.log(error);
      else console.log("Is SQLConnect supported : ", value);
      conn.closeSync();
    });
});
```

### <a name="getFunctionsSyncApi"></a> 48) (Database) .getFunctionsSync(functionId)

Synchronously determines whether a specific CLI or ODBC function is supported. This allows applications to adapt to varying levels of support when connecting to different database servers.

* **functionId** - The value for a function being queried. The value for this argument should be an integer value if macro is not defined in `ibm_db/lib/climacros.js` file.

```javascript
const ibmdb = require("ibm_db")
  , cn = "DATABASE=database;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

ibmdb.open(cn, function(err, conn)
{
    let fExists = conn.getFunctionsSync(ibmdb.SQLFREECONNECT);
    console.log("Function SQLFreeConnect Exist : ", fExists);
    conn.closeSync();
});
```

## Create and Drop Database APIs

### <a name="createDbSyncApi"></a> (ibmdb) .createDbSync(dbName, connectionString [, options])

To create a database (dbName) through Node.js application.

* **dbName** - The database name.
* **connectionString** - The connection string for your database instance.
* **options** - _OPTIONAL_ - Object type.
    * codeSet - Database code set information.
    * mode    - Database logging mode (applicable only to "IDS data servers").

```javascript
const ibmdb = require("ibm_db");
// Connection string without "DATABASE" keyword and value.
const cn = "HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

const DB_NAME = "TESTDB";

const createDB = ibmdb.createDbSync(DB_NAME, cn);

if (createDB) {
  console.log("Database created successfully.");
  // Connection string with newly created "DATABASE" name.
	const conStr = cn + ";" + "DATABASE=" + DB_NAME;

	ibmdb.open(conStr, function(err, conn) {
		if(err) console.log(err);
		else console.log("Database connection opened.");
	});
}
```

Note: This API is not supported for Db2 on z/OS servers.  Given that connection
to Db2 on z/OS is to a specific subsystem, this API is not applicable.

### <a name="dropDbSyncApi"></a> (ibmdb) .dropDbSync(dbName, connectionString [, options])

To drop a database (dbName) through node.js application.

* **dbName** - The database name.
* **connectionString** - The connection string for your database instance.
* **options** - _OPTIONAL_ - Object type.
    * codeSet - Database code set information.
    * mode    - Database logging mode (applicable only to "IDS data servers").

```javascript
const ibmdb = require("ibm_db");
// Connection string without "DATABASE" keyword and value.
const cn = "HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=username;PWD=password";

const DB_NAME = "TESTDB";

const dropDB = ibmdb.dropDbSync(DB_NAME, cn);

if (dropDB) {
  console.log("Database dropped successfully.");
}
```

Note: This API is not supported for Db2 on z/OS servers.  Given that connection
to Db2 on z/OS is to a specific subsystem, this API is not applicable.

## <a name="PoolAPIs"></a>Connection Pooling APIs

node-ibm_db reuses its own connection pooling mechanism.
The ibm_db `Pool` is a rudimentary connection pool which will attempt to have
database connections ready and waiting for you when you call the `open` method.

If you use a `Pool` instance, any connection that you close will get added to
the list of available connections immediately. These connection will be used
the next time you call `Pool.open()` for the same connection string.

For applications using multiple connections simultaneously, it is recommended to
use Pool.open instead of [ibmdb.open](#1-openconnectionstring-options-callback).

### <a name="openPoolApi"></a> 1) (Pool) .open(connectionString [, callback])

Get a `Database` instance which is already connected to `connectionString`

* **connectionString** - The connection string for your database
* **callback** - _OPTIONAL_ - `callback (err, db)`. If callback is not provided, a Promise will be returned.

```javascript
const Pool = require("ibm_db").Pool
	, pool = new Pool()
    , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

pool.open(cn, function (err, db) {
	if (err) {
		return console.log(err);
	}
  console.log("Connection opened successfully.");
  console.log("Data = ", db.querySync("select 1 as c1 from sysibm.sysdummy1"));
  db.close(function (error) { // RETURN CONNECTION TO POOL
      if (error) {
        console.log("Error while closing connection,", error);
        return;
      }
  });
});
```

### <a name="openSyncPoolApi"></a> 2) (Pool) .openSync(connectionString)

Get a `Database` connection synchronously which is already connected to `connectionString`

* **connectionString** - The connection string for your database

See [test-pool-close.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-pool-close.js) for an example.

```javascript
const Pool = require("ibm_db").Pool
	, pool = new Pool()
    , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

try {
    const conn = pool.openSync(connectionString);
} catch(error) {
    console.log("Unable to open connection,", error);
    return;
}
console.log("Connection opened successfully.");
console.log("Data = ", conn.querySync("select 1 as c1 from sysibm.sysdummy1"));
const err = conn.closeSync(); // RETURN DB CONNECTION TO POOL.
if (err) {
    console.log("Error while closing connection,", err);
    return;
}
```

### <a name="closePoolApi"></a> 3) (Pool) .close([callback])

Close all connections in the `Pool` instance asynchronously.

* **callback** - _OPTIONAL_ - `callback (err)`. If callback is not provided, a Promise will be returned.

```javascript
const Pool = require("ibm_db").Pool
	, pool = new Pool()
    , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

pool.open(cn, function (err, db) {
    if (err) {
        return console.log(err);
    }

    //db is now an open database connection and can be used like normal connection.
    //but all we will do now is close the whole pool using close() API.
    //Use db.close() to return the connection back to pool for next use.

    pool.close(function () {
        console.log("All connections in the pool are closed.");
    });
});
```

### <a name="closeSyncPoolApi"></a> 4) (Pool) .closeSync()

Close all connections in the `Pool` instance synchronously.
See [test-pool-close.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-pool-close.js) for an example.

```javascript
const Pool = require("ibm_db").Pool
	, pool = new Pool()
    , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

try {
    const conn = pool.openSync(connectionString);
} catch(error) {
    console.log("Unable to open connection,", error);
    return;
}
console.log("Connection opened successfully.");

//conn is now an open database connection and can be used like a normal connection.
//but all we will do now is close the whole pool using closeSync() API.
//Use conn.closeSync() to return the connection back to pool for next use.

const error = pool.closeSync();
if (error) { console.log("Error while closing pool,", error); return; }
console.log("All connections in the pool are closed.");
```

### <a name="initPoolApi"></a> 5) (Pool) .init(N, connStr)

Initialize `Pool` with N no of active connections using supplied connection string.
It is a synchronous API.

* **N** - No of connections to be initialized.
* **connStr** - The connection string for your database

```javascript
const ret = pool.init(5, connStr);
if(ret != true)
{
    console.log(ret);
    return false;
}

pool.open(connStr, function(err, db) { ...
```

See [test-max-pool-size.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-max-pool-size.js) for an example.


### <a name="initAsyncApi"></a> 6) (Pool) .initAsync(N, connStr [, callback])

Initialize `Pool` with N no of active connections using supplied connection string.
It is an asynchronous API.

* **N** - No of connections to be initialized.
* **connStr** - The connection string for your database or a JSON Object with connection information
* **callback** - _OPTIONAL_ - `callback (err)`. If callback is not provided, a Promise will be returned.

```javascript
pool.initAsync(5, connStr, function(err) {
  if(err) {
    console.log(err);
    return false;
  }
  pool.open(connStr, function(err, db) { ... });
});

try {
  await pool.initAsync(1, cn);
  let conn = await pool.open(cn);
  let data = await conn.query("select 1 from sysibm.sysdummy1");
  console.log("data = ", data);
} catch(err) {console.log(err);}
```

See [test-asyc-await.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-asyc-await.js#L50) for an example.

### <a name="setMaxPoolSize"></a> 7) (Pool) .setMaxPoolSize(N)

Set the maximum number of connections to the database supported by the current pool.

* **N** - No of maximum connections in the pool.
If we call the `pool.open()` or `openSync()` APIs and **N** connections are already in use,
subsequent connection requests will be queued and wait until a connection a connection is closed or the maximum connection timeout.  See [test-max-pool-size.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-max-pool-size.js) for an example.

```javascript
pool.setMaxPoolSize(20);
pool.open(connStr, function(err, db) { ...
```

## <a name="bindParameters"></a>bindingParameters

Bind arguments for each parameter marker(?) in SQL query.
These parameters can be used with query(), querySync, bind(), execute() APIs.
bindingParameters is an array of Values like: [val1, val2, ...]
Each value in itself can be an Array or Object having multiple bind options.
If parameters are not an integer or string, it is recomended to pass an Object with different bind options. The object can have following keys:

`{"ParamType":"INOUT", CType:"BINARY", SQLType:"BLOB",DataType: "BLOB", Data:imgfile, Length:50}`

Either SQLType or DataType must be used. If SQLType is used, DataType will be ignored.

* **ParmType**: Type of the Parameter. Supported Values are:
 - INPUT - Bind the parameter using SQL_PARAM_INPUT(defined in ibm_db/installer/clidriver/include/sqlext.h file). It is used as input value and it is the default value, if you don't use this key in object.
 - OUTPUT - Bind the parameter using SQL_PARAM_OUTPUT. It is basically used for Stored Procedure call which has output parameters.
 - INOUT - Bind the parameter using SQL_PARAM_INPUT_OUTPUT. It is also used for Stored Procedure call.
 - FILE  - It tells the Data is a filename that contains actual data to load. If you want to load an image to database, use this input type along with DataType as BLOB for binary file.
   f.e. `{ParamType: "FILE", DataType: "BLOB", Data: "mypic.jpg"}`
 - ARRAY - It tells the Data is an Array of same type and size. It must be used for Array Insert i.e. to insert data for multiple rows using single execute.
   If one parameter is of type ARRAY, all parameters passed to an API must be of type ARRAY and of equal size. Check [test/test-array-insert.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-array-insert.js) for example.
    Use "Length: <maxDataLen>" in param Object for unequal size of data.
    Default value is the length of first member of Array.

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
[{ParamType:"ARRAY", DataType:1, Data:[4,5,6,7,8]}, {ParamType:"ARRAY", DataType:"DOUBLE", Data:[4.1,5.3,6.14,7,8.3]}] - for Array insert.
```
The values in array parameters used in above example is not recommened to use as it is dificult to understand. These values are macro values from ODBC specification and we can directly use those values. To understand it, see the [SQLBindParameter](http://www.ibm.com/support/knowledgecenter/en/SSEPGG_11.1.0/com.ibm.db2.luw.apdv.cli.doc/doc/r0002218.html) documentation for DB2.

Pass bind parameters as Object if you want to insert an Array or BLOB or CLOB data to DB2. Check below test files to know how to insert a BLOB and CLOB data from buffer and file:

 - [test-blob-insert.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-blob-insert.js) - To insert a BLOB and CLOB data using memory buffer. Application need to read the file contents and then use as bind parameter.
 - [test-blob-file.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-blob-file.js) - To insert an image file and large text file directly to database without reading it by application.
 - [test-array-insert.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-array-insert.js) - For Array Insert.

## <a name="callStmt"></a>CALL Statement

* If stored procedure has any OUT or INOUT parameter, always call it with
parmeter markers only. i.e. pass the input values using bind params.

* Pass the Bind Params as objects only.

* If SP has result set to return, it will be returned in the array after out params. f.e. if SP has 2 out params and it returns 2 result set too, the result returned by query() or querySync() would be in the form [outValue1, outValue2, resultSet1, resultSet2]. Each resultset would be an array of row objects.

* [test-call-stmt.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-call-stmt.js) - Example using conn.querySync().

* [test-call-async.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-call-async.js) - Example using conn.query().

* [test-sp-resultset.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-sp-resultset.js) - Example using Out Params and Result Set using query() and querySync() APIs.

* [test-sp-resultset-execute.js](https://github.com/ibmdb/node-ibm_db/blob/master/test/test-sp-resultset-execute.js) - Example using Out Params and Result Set using prepare() and execute() APIs.


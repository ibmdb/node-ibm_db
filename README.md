# node-ibm_db

An asynchronous/synchronous interface for node.js to IBM DB2 and IBM Informix.

**Supported Platforms** - Windows64, MacOS64, Linuxx64, Linuxia32, AIX, Linux on z and Linux on Power PC.

## Prerequisite

- For higher versions of node (When building with Node 4 onwards) the compiler must support
C++11. Note the default compiler on RHEL 6 does not have the required support.
Install a newer compiler or upgrade older one.

- Python 2.7 is needed by node-gyp.

- You need not to install any db2 ODBC client driver for connectivity. `ibm_db` itself download and install odbc/cli driver from ibm website during installation. Just install `ibm_db` and it is ready for use.

- Recommended versions of node.js is V4.x, V6.x and V7.x. Support for node.js V0.12.x is deprecated on Windows and will be discontinued from next release.

## Install

You may install the package using npm install command:

```
npm install ibm_db
```

> For more installation details please refer:  [INSTALLAION GUIDE](https://github.com/ibmdb/node-ibm_db/blob/master/INSTALL.md)


### Important Environment Variables and Download Essentials 

`IBM_DB_HOME :`

- USE: Set this environment variable if you want to avoid downloading of clidriver from the [IBM Hosted URL](#downloadCli) or from the internet.

- How: Set **IBM_DB_HOME** environment variable to a pre-installed **db2 client or server installation directory**.

`IBM_DB_INSTALLER_URL :`

- USE: Set this environment variable to by-pass the IBM Hosted URL for downloading odbc/clidriver.

- HOW: Set **IBM_DB_INSTALLER_URL** environment variable with alternate odbc/clidriver downloading URL link or with locally downloaded "tar/zipped clidriver's parent directory path.

- TIP: If you don't have alternate hosting URL then, you can download the tar/zipped file of clidriver from the [IBM Hosted URL](#downloadCli) and can set the **IBM_DB_INSTALLER_URL** environment variable to the downloaded "tar/zipped clidriver's" parent directory path. No need to untar/unzip the clidriver and do not change the name of downloaded file.


### <a name="downloadCli"></a> Download clidriver ([based on your platform & architecture](#systemDetails)) from the below IBM Hosted URL:
> [DOWNLOAD CLI DRIVER](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/)

#### <a name="systemDetails"></a> Cli Drivers for Specific Platform and Architecture

|Platform      |Architecture    |Cli Driver               |Supported     |
| :---:        |  :---:         |  :---:                  |  :---:       |
|AIX           |  ppc           |aix32_odbc_cli.tar.gz    |  Yes         |
|              |  others        |aix64_odbc_cli.tar.gz    |  Yes         |
|Darwin        |  x64           |macos64_odbc_cli.tar.gz  |  Yes         |
|Linux         |  x64           |linuxx64_odbc_cli.tar.gz |  Yes         |
|              |  s390x         |s390x64_odbc_cli.tar.gz  |  Yes         |
|              |  s390          |s390_odbc_cli.tar.gz     |  Yes         |
|              |  ppc64  (LE)   |ppc64le_odbc_cli.tar.gz  |  Yes         |
|              |  ppc64         |ppc64_odbc_cli.tar.gz    |  Yes         |
|              |  ppc32         |ppc32_odbc_cli.tar.gz    |  Yes         |
|              |  others        |linuxia32_odbc_cli.tar.gz|  Yes         |
|Windows       |  x64           |ntx64_odbc_cli.zip       |  Yes         |
|              |  x32           |nt32_odbc_cli.zip        |  Not supported with node-ibm_db          |



## Quick Example

```javascript
var ibmdb = require('ibm_db');

ibmdb.open("DATABASE=<dbname>;HOSTNAME=<myhost>;UID=db2user;PWD=password;PORT=<dbport>;PROTOCOL=TCPIP", function (err,conn) {
  if (err) return console.log(err);
  
  conn.query('select 1 from sysibm.sysdummy1', function (err, data) {
    if (err) console.log(err);
    else console.log(data);

    conn.close(function () {
      console.log('done');
    });
  });
});
```

## Un-Install

To uninstall node-ibm_db from your system, just delete the node-ibm_db or ibm_db directory.


## For z/OS and iSeries Connectivity

For connectivity against DB2 for LUW or Informix Server using node-ibm_db, 
no license file is required. However, if you want to use node-ibm_db 
against DB2 for z/OS or DB2 for i(AS400) Servers, you must have db2connect 
license if server is not db2connectactivated to accept unlimited number of 
client connection. You can buy db2connect license from IBM. The connectivity 
can be enabled either on server using db2connectactivate utility or on client 
using client side license file. If you have client side license file, just 
copy it under `.../ibm_db/installer/clidriver/license` folder to be effective. 

To know more about license and purchasing cost, please contact [IBM Customer Support](http://www-05.ibm.com/support/operations/zz/en/selectcountrylang.html).

## For AIX install issue

If `npm install ibm_db` aborts with "Out Of Memory" error on AIX, first run `ulimit -d unlimited` and then `npm install ibm_db`.

## For Missing Package/Binding issue

If your application is able to connect to IBM Database Server but query execution is throwing SQL0805N error, run below commnads to fix the package related issues:
```
cd .../ibm_db/installer
source setenv.sh
db2cli bind $IBM_DB_HOME/bnd/@db2cli.lst -database <dbname>:<hostname>:<port> -user <dbuser> -passwd <passwd> -options "grant public action replace blocking no"
```

If above command prints 0 error at end, then you can proceed to run query. If 
it reports non-zero error, open a new issue on github and share the output 
of above `db2cli bind` command along with query execution error.

Alternatively, if you have any other DB2 client with CLP, you can bind packages using db2 bind command too. f.e. use below command against DB2 for z/OS server:
```
db2 bind .../sqllib/bnd/@ddcsmvs.lst action replace grant public sqlerror continue messages msg.txt
```
Note: "db2cli bind" command print the logs on output prompt, so you need to redirect output to some file to capture it. 
    To capture logs of "db2 bind" command, you need to use `messages` option as in above example.


## Need Help?

If you encountered any issue with ibm_db, first check for existing solution or
work-around under `issues` or on google groups forum. Links are:   
    
https://github.com/ibmdb/node-ibm_db/issues    
https://groups.google.com/forum/#!forum/node-ibm_db   
   
If no solution found, you can open a new issue on github or start a new topic in google groups.

## Database APIs

The simple api is based on instances of the `Database` class. You may get an 
instance in one of the following ways:

```javascript
require("ibm_db").open(connectionString, function (err, conn){
  //conn is already open now if err is falsy
});
```

or by using the helper function:

```javascript
var ibmdb = require("ibm_db")();
``` 

or by creating an instance with the constructor function:

```javascript
var Database = require("ibm_db").Database
  , ibmdb = new Database();
```

1.  [.open(connectionString, [options,] callback)](#openApi)
2.  [.openSync(connectionString)](#openSyncApi)
3.  [.query(sqlQuery [, bindingParameters], callback)](#queryApi)
4.  [.querySync(sqlQuery [, bindingParameters])](#querySyncApi) 
5.  [.queryStream(sqlQuery [, bindingParameters])](#queryStreamApi) 
6.  [.close(callback)](#closeApi)
7.  [.closeSync()](#closeSyncApi)
8.  [.prepare(sql, callback)](#prepareApi)
9.  [.prepareSync(sql)](#prepareSyncApi)
10. [.execute([bindingParameters], callback)](#executeApi)
11. [.executeSync([bindingParameters])](#executeSyncApi)
12. [.executeNonQuery([bindingParameters], callback)](#executeNonQueryApi)
13. [.bind(bindingParameters, callback)](#bindApi)
14. [.bindSync(bindingParameters)](#bindSyncApi)
15. [.beginTransaction(callback)](#beginTransactionApi)
16. [.beginTransactionSync()](#beginTransactionSyncApi)
17. [.commitTransaction(callback)](#commitTransactionApi)
18. [.commitTransactionSync()](#commitTransactionSyncApi)
19. [.rollbackTransaction(callback)](#rollbackTransactionApi)
20. [.rollbackTransactionSync()](#rollbackTransactionSyncApi)
21. [.debug(value)](#enableDebugLogs)

*   [**Connection Pooling APIs**](#PoolAPIs)
*   [**bindingParameters**](#bindParameters)
*   [**CALL Statement**](#callStmt)
*   [**Build Options**](#buildOptions)


### <a name="openApi"></a> 1) .open(connectionString, [options,] callback)

Open a connection to a database.

* **connectionString** - The connection string for your database
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
You can also create a KeyStore DB using GSKit command line tool and use it in connection string along with other keywords as documented in [DB2 Infocenter](http://www.ibm.com/support/knowledgecenter/en/SSEPGG_10.5.0/com.ibm.db2.luw.admin.sec.doc/doc/t0053518.html).

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

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue}. noResults accepts only true or false values. If true - query() will not return any result. noResults must be true for CALL statements. "sql" field is mandatory in Object, others are _OPTIONAL_.

* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`. bindingParameters in sqlQuery Object takes precedence over it.

* **callback** - `callback (err, rows)`

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
               "select 3 from sysibm.sysdummy1", function (err, rows) 
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

* **sqlQuery** - The SQL query to be executed or an Object in the form {"sql": sqlQuery, "params":bindingParameters, "noResults": noResultValue}. noResults accepts only true or false values. If true - query() will not return any result. If noResults is true for CALL statement, querySync returns only OutParams. "sql" field is mandatory in Object, others are optional.

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

### <a name="closeApi"></a> 6) .close(callback)

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

### <a name="closeSyncApi"></a> 7) .closeSync()

Synchronously close the currently opened database.

```javascript
var ibmdb = require("ibm_db")()
  , cn = "DATABASE=dbname;HOSTNAME=hostname;PORT=port;PROTOCOL=TCPIP;UID=dbuser;PWD=xxx";

//Blocks until the connection is open
ibmdb.openSync(cn);

//Blocks until the connection is closed
ibmdb.closeSync();
```

### <a name="prepareApi"></a> 8) .prepare(sql, callback)

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

### <a name="prepareSyncApi"></a> 9) .prepareSync(sql)

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

### <a name="executeApi"></a> 10) .execute([bindingParameters], callback)

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

### <a name="executeSyncApi"></a> 11) .executeSync([bindingParameters])

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

### <a name="executeNonQueryApi"></a> 12) .executeNonQuery([bindingParameters], callback)

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

### <a name="bindApi"></a> 13) .bind(bindingParameters, callback)

Binds the parameters for prepared statement.

* **bindingParameters** - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be array or object itself. Check [bindingParameters](#bindParameters) doc for detail.
* **callback** - `callback (err)`

### <a name="bindSyncApi"></a> 14) .bindSync(bindingParameters)

Binds the parameters for prepared statement synchronously. If `bindSync()` is used, then no need to pass `bindingParameters` to next `execute()` or `executeSync()` statement.

* **bindingParameters** - An array of values that will be bound to any '?' characters in prepared sql statement. Values can be array or object itself. Check [bindingParameters](#bindParameters) doc for detail.

### <a name="beginTransactionApi"></a> 15) .beginTransaction(callback)

Begin a transaction

* **callback** - `callback (err)`

### <a name="beginTransactionSyncApi"></a> 16) .beginTransactionSync()

Synchronously begin a transaction

### <a name="commitTransactionApi"></a> 17) .commitTransaction(callback)

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

### <a name="commitTransactionSyncApi"></a> 18) .commitTransactionSync()

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

### <a name="rollbackTransactionApi"></a> 19) .rollbackTransaction(callback)

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

### <a name="rollbackTransactionSyncApi"></a> 20) .rollbackTransactionSync()

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

### <a name="enableDebugLogs"></a> 21) .debug(value)

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
	//a connection to `cn` will be re-opened silently behind the scense
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

`{"ParamType":"INPUT", CType:"BINARY", SQLType:"BLOB",DataType: "BLOB", Data:imgfile}`

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
The values in array parameters used in above example is not recommened to use as it is dificult to understand. These values are macro values from ODBC specification and we can directly use those values. To understand it, see the [SQLBindParameter](http://www.ibm.com/support/knowledgecenter/en/SSEPGG_10.5.0/com.ibm.db2.luw.apdv.cli.doc/doc/r0002218.html) documentation for DB2.

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

## <a name="buildOptions"></a>Build Options

### Debug

If you would like to enable debugging messages to be displayed you can add the 
flag `DEBUG` to the defines section of the `binding.gyp` file and then execute 
`node-gyp rebuild`.

```javascript
<snip>
'defines' : [
  "DEBUG"
],
<snip>
```

### Unicode

By default, UNICODE suppport is enabled. This should provide the most accurate
way to get Unicode strings submitted to your database. For best results, you 
may want to put your Unicode string into bound parameters. 

However, if you experience issues or you think that submitting UTF8 strings will
work better or faster, you can remove the `UNICODE` define in `binding.gyp`

```javascript
<snip>
'defines' : [
  "UNICODE"
],
<snip>
```

### timegm vs timelocal

When converting a database time to a C time one may use `timegm` or `timelocal`. See
`man timegm` for the details of these two functions. By default the node-ibm_db bindings
use `timelocal`. If you would prefer for it to use `timegm` then specify the `TIMEGM`
define in `binding.gyp`

```javascript
<snip>
'defines' : [
  "TIMEGM"
],
<snip>
```

### Strict Column Naming

When column names are retrieved from DB2 CLI, you can request by SQL_DESC_NAME or
SQL_DESC_LABEL. SQL_DESC_NAME is the exact column name or none if there is none
defined. SQL_DESC_LABEL is the heading or column name or calculation. 
SQL_DESC_LABEL is used by default and seems to work well in most cases.

If you want to use the exact column name via SQL_DESC_NAME, enable the `STRICT_COLUMN_NAMES`
define in `binding.gyp`

```javascript
<snip>
'defines' : [
  "STRICT_COLUMN_NAMES"
],
<snip>
```

## Tips

### Using node < v0.10 on Linux

Be aware that through node v0.9 the uv_queue_work function, which is used to 
execute the ODBC functions on a separate thread, uses libeio for its thread 
pool. This thread pool by default is limited to 4 threads.

This means that if you have long running queries spread across multiple 
instances of ibmdb.Database() or using odbc.Pool(), you will only be able to 
have 4 concurrent queries.

You can increase the thread pool size by using @developmentseed's [node-eio]
(https://github.com/developmentseed/node-eio).

#### install: 
```bash
npm install eio
```

#### usage:
```javascript
var eio = require('eio'); 
eio.setMinParallel(threadCount);
```

### Issues while connecting to Informix Server

While using ibm_db against Informix server, you may get few issues if
server is not configured properly. Also, ibm_db connects to only DRDA port.
So, make sure drsoctcp of Informix is configured.

#### SQL1042C Error
If ibm_db is returning SQL1042C error while connecting to server, use
"Authentication=SERVER" in connection string. It should avoid the error.
Alternatively, you can set Authentication in db2cli.ini file or db2dsdriver.cfg file too.

#### code-set conversion error
If Informix server is not enabled for UNICODE clients or some code-set object
file is missing on server; server returns this error to ibm_db:
[IBM][CLI Driver][IDS/UNIX64] Error opening required code-set conversion object file.

To avoid this error, remove UNICODE from binding.gyp file and rebuild the ibm_db.

Also to avoid above issues, you can run [ibm_db/installer/ifx.sh](https://github.com/ibmdb/node-ibm_db/blob/master/installer/ifx.sh) script on non-windows system.

## Contributor

* Dan VerWeire (dverweire@gmail.com)
* Lee Smith (notwink@gmail.com)
* Bruno Bigras
* Christian Ensel
* Yorick
* Joachim Kainz
* Oleg Efimov
* paulhendrix
* IBM

## Contributing to the node-ibm_db

[Contribution Guidelines](https://github.com/ibmdb/node-ibm_db/blob/master/contributing/CONTRIBUTING.md)

```
Contributor should add a reference to the DCO sign-off as comment in the pull request(example below):
DCO 1.1 Signed-off-by: Random J Developer <random@developer.org>
```

## License

Copyright (c) 2013 Dan VerWeire <dverweire@gmail.com>

Copyright (c) 2010 Lee Smith <notwink@gmail.com>

Copyright (c) 2014 IBM Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

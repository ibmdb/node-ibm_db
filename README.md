node-odbc
---------

An asynchronous/synchronous interface for node.js to unixODBC and its supported
drivers.

requirements
------------

* unixODBC binaries and development libraries for module compilation
  * on Ubuntu/Debian `sudo apt-get install unixodbc unixodbc-dev`
  * on OSX using macports.org `sudo port unixODBC`
* odbc drivers for target database
* properly configured odbc.ini and odbcinst.ini.

install
-------

After insuring that all requirements are installed you may install by one of the
two following options:

### git

```bash
git clone git://github.com/wankdanker/node-odbc.git
cd node-odbc
node-gyp configure build
```
### npm

```bash
npm install odbc
```

quick example
-------------

```javascript
var db = require('odbc')()
  , cn = process.env.ODBC_CONNECTION_STRING
  ;

db.open(cn, function (err) {
  if (err) return console.log(err);
  
  db.query('select * from user where user_id = ?', [42], function (err, data) {
    if (err) console.log(err);
    
    console.log(data);

    db.close(function () {
      console.log('done');
    });
  });
});
```

api
---

### Database

The simple api is based on instances of the `Database` class. You may get an 
instance in one of the following ways:

```javascript
require("odbc").open(connectionString, function (err, db){
  //db is already open now if err is falsy
});
```

or by using the helper function:

```javascript
var db = require("odbc")();
``` 

or by creating an instance with the constructor function:

```javascript
var Database = require("odbc").Database
  , db = new Database();
```

#### .open(connectionString, callback)

Open a connection to a database.

* **connectionString** - The ODBC connection string for your database
* **callback** - `callback (err)`

```javascript
var db = require("odbc")()
	, cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
	;

db.open(cn, function (err) {
	if (err) {
		return console.log(err);
	}

	//we now have an open connection to the database
});
```
#### .openSync(connectionString)

Synchronously open a connection to a database.

* **connectionString** - The ODBC connection string for your database

```javascript
var db = require("odbc")()
  , cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
  ;

try {
  var result = db.openSync(cn);
}
catch (e) {
  console.log(e.message);
}

//we now have an open connection to the database
```

#### .query(sqlQuery [, bindingParameters], callback)

Issue an asynchronous SQL query to the database which is currently open.

* **sqlQuery** - The SQL query to be executed.
* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`.
* **callback** - `callback (err, rows, moreResultSets)`

```javascript
var db = require("odbc")()
	, cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
	;

db.open(cn, function (err) {
	if (err) {
		return console.log(err);
	}

	//we now have an open connection to the database
	//so lets get some data
	db.query("select top 10 * from customers", function (err, rows, moreResultSets) {
		if (err) {
			return console.log(err);
		}
		
		console.log(rows);

		//if moreResultSets is truthy, then this callback function will be called
		//again with the next set of rows.
	});
});
```

#### .querySync(sqlQuery [, bindingParameters])

Synchronously issue a SQL query to the database that is currently open.

* **sqlQuery** - The SQL query to be executed.
* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`.

```javascript
var db = require("odbc")()
  , cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
  ;

//blocks until the connection is opened.
db.openSync(cn);

//blocks until the query is completed and all data has been acquired
var rows = db.querySync("select top 10 * from customers");

console.log(rows);
```

#### .close(callback)

Close the currently opened database.

* **callback** - `callback (err)`

```javascript
var db = require("odbc")()
	, cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
	;

db.open(cn, function (err) {
	if (err) {
		return console.log(err);
	}
	
	//we now have an open connection to the database
	
	db.close(function (err) {
		console.log("the database connection is now closed");
	});
});
```

#### .closeSync()

Synchronously close the currently opened database.

```javascript
var db = require("odbc")()
  , cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
  ;

//Blocks until the connection is open
db.openSync(cn);

//Blocks until the connection is closed
db.closeSync();
```

#### .prepare(sql, callback)

Prepare a statement for execution.

* **sql** - SQL string to prepare
* **callback** - `callback (err, stmt)`

Returns a `Statement` object via the callback

```javascript
var db = require("odbc")()
  , cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
  ;

//Blocks until the connection is open
db.openSync(cn);

db.prepare("insert into hits (col1, col2) VALUES (?, ?)", function (err, stmt) {
  if (err) {
    //could not prepare for some reason
    console.log(err);
    return db.closeSync();
  }

  //Bind and Execute the statment asynchronously
  stmt.execute(['something', 42], function (err, result) {
    result.closeSync();

    //Close the connection
    db.closeSync();
  });
})
```

#### .prepareSync(sql)

Synchronously prepare a statement for execution.

* **sql** - SQL string to prepare

Returns a `Statement` object

```javascript
var db = require("odbc")()
  , cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
  ;

//Blocks until the connection is open
db.openSync(cn);

//Blocks while preparing the statement
var stmt = db.prepareSync("insert into hits (col1, col2) VALUES (?, ?)")

//Bind and Execute the statment asynchronously
stmt.execute(['something', 42], function (err, result) {
  result.closeSync();

  //Close the connection
  db.closeSync();
});
```

#### .beginTransaction(callback)

Begin a transaction

* **callback** - `callback (err)`

#### .beginTransactionSync()

Synchronously begin a transaction

#### .commitTransaction(callback)

Commit a transaction

* **callback** - `callback (err)`

```javascript
var db = require("odbc")()
  , cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
  ;

//Blocks until the connection is open
db.openSync(cn);

db.beginTransaction(function (err) {
  if (err) {
    //could not begin a transaction for some reason.
    console.log(err);
    return db.closeSync();
  }

  var result = db.querySync("insert into customer (customerCode) values ('stevedave')");

  db.commitTransaction(function (err) {
    if (err) {
      //error during commit
      console.log(err);
      return db.closeSync();
    }

    console.log(db.querySync("select * from customer where customerCode = 'stevedave'"));

    //Close the connection
    db.closeSync();
  });
})
```

#### .commitTransactionSync()

Synchronously commit a transaction

```javascript
var db = require("odbc")()
  , cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
  ;

//Blocks until the connection is open
db.openSync(cn);

db.beginTransactionSync();

var result = db.querySync("insert into customer (customerCode) values ('stevedave')");

db.commitTransactionSync();

console.log(db.querySync("select * from customer where customerCode = 'stevedave'"));

//Close the connection
db.closeSync();
```

#### .rollbackTransaction(callback)

Rollback a transaction

* **callback** - `callback (err)`

```javascript
var db = require("odbc")()
  , cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
  ;

//Blocks until the connection is open
db.openSync(cn);

db.beginTransaction(function (err) {
  if (err) {
    //could not begin a transaction for some reason.
    console.log(err);
    return db.closeSync();
  }

  var result = db.querySync("insert into customer (customerCode) values ('stevedave')");

  db.rollbackTransaction(function (err) {
    if (err) {
      //error during rollback
      console.log(err);
      return db.closeSync();
    }

    console.log(db.querySync("select * from customer where customerCode = 'stevedave'"));

    //Close the connection
    db.closeSync();
  });
})
```

#### .rollbackTransactionSync()

Synchronously rollback a transaction

```javascript
var db = require("odbc")()
  , cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
  ;

//Blocks until the connection is open
db.openSync(cn);

db.beginTransactionSync();

var result = db.querySync("insert into customer (customerCode) values ('stevedave')");

db.rollbackTransactionSync();

console.log(db.querySync("select * from customer where customerCode = 'stevedave'"));

//Close the connection
db.closeSync();
```

----------

### Pool

The node-odbc `Pool` is a rudimentary connection pool which will attempt to have
database connections ready and waiting for you when you call the `open` method.

If you use a `Pool` instance, any connection that you close will cause another
connection to be opened for that same connection string. That connection will
be used the next time you call `Pool.open()` for the same connection string.

This should probably be changed.

#### .open(connectionString, callback)

Get a Database` instance which is already connected to `connectionString`

* **connectionString** - The ODBC connection string for your database
* **callback** - `callback (err, db)`

```javascript
var Pool = require("odbc").Pool
	, pool = new Pool()
	, cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
	;

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

#### .close(callback)

Close all connections in the `Pool` instance

* **callback** - `callback (err)`

```javascript
var Pool = require("odbc").Pool
	, pool = new Pool()
	, cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
	;

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

example
-------

```javascript
var odbc = require("odbc")
	, util = require('util')
	, db = new odbc.Database()
	;

var connectionString = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname";

db.open(connectionString, function(err) {
	db.query("select * from table", function(err, rows, moreResultSets) {
		console.log(util.inspect(rows, null, 10));
		
		db.close(function() {
			console.log("Database connection closed");
		});
	});
});
```

testing
-------

Tests can be run by executing `npm test` from within the root of the node-odbc
directory. You can also run the tests by executing `node run-tests.js` from
within the `/test` directory.

By default, the tests are setup to run against a sqlite3 database which is
created at test time. This will require proper installation of the sqlite odbc
driver. On Ubuntu: `sudo apt-get install libsqliteodbc`

build options
-------------

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

### Dynodbc

You may also enable the ability to load a specific ODBC driver and bypass the 
ODBC driver management layer. A performance increase of ~5Kqps was seen using
this method with the libsqlite3odbc driver. To do this, specify the `dynodbc`
flag in the defines section of the `binding.gyp` file. You will also need to 
remove any library references in `binding.gyp`. Then execute `node-gyp
rebuild`.

```javascript
<snip>
'defines' : [
  "dynodbc"
],
'conditions' : [
  [ 'OS == "linux"', {
    'libraries' : [ 
      //remove this: '-lodbc' 
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
`man timegm` for the details of these two functions. By default the node-odbc bindings
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

When column names are retrieved from ODBC, you can request by SQL_DESC_NAME or
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

tips
----
### Using node < v0.10 on Linux

Be aware that through node v0.9 the uv_queue_work function, which is used to 
execute the ODBC functions on a separate thread, uses libeio for its thread 
pool. This thread pool by default is limited to 4 threads.

This means that if you have long running queries spread across multiple 
instances of odbc.Database() or using odbc.Pool(), you will only be able to 
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

### Using the FreeTDS ODBC driver

* If you have column names longer than 30 characters, you should add 
  "TDS_Version=7.0" to your connection string to retrive the full column name.
  * Example : "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname;TDS_Version=7.0"
* Be sure that your odbcinst.ini has the proper threading configuration for your
  FreeTDS driver. If you choose the incorrect threading model it may cause
  the thread pool to be blocked by long running queries. This is what 
  @wankdanker currently uses on Ubuntu 12.04:

```
[FreeTDS]
Description     = TDS driver (Sybase/MS SQL)
Driver          = libtdsodbc.so
Setup           = libtdsS.so
CPTimeout       = 120
CPReuse         = 
Threading       = 0
```

contributors
------
* Dan VerWeire (dverweire@gmail.com)
* Lee Smith (notwink@gmail.com)
* Bruno Bigras
* Christian Ensel
* Yorick
* Joachim Kainz
* Oleg Efimov
* paulhendrix

license
-------

Copyright (c) 2013 Dan VerWeire <dverweire@gmail.com>

Copyright (c) 2010 Lee Smith <notwink@gmail.com>

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

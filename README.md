node-odbc
---------

An asynchronous interface for node.js to unixODBC and its supported drivers. 

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

api
---

### Database

#### .open(connectionString, callback)

Open a connection to a database.

* **connectionString** - The ODBC connection string for your database
* **callback** - `callback (err)`

```javascript
var Database = require("odbc").Database
	, db = new Database()
	, cn = "DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname"
	;

db.open(cn, function (err) {
	if (err) {
		return console.log(err);
	}

	//we now have an open connection to the database
});
```

#### .query(sqlQuery [, bindingParameters], callback)

Issue a SQL query to the database which is currently open.

* **sqlQuery** - The SQL query to be executed.
* **bindingParameters** - _OPTIONAL_ - An array of values that will be bound to
    any '?' characters in `sqlQuery`.
* **callback** - `callback (err, rows, moreResultSets)`

```javascript
var Database = require("odbc").Database
	, db = new Database()
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

#### .close(callback)

Close the currently opened database.

* **callback** - `callback (err)`

```javascript
var Database = require("odbc").Database
	, db = new Database()
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

There is a tests folder which contains scripts which are more examples than tests.
We will be working on bundling these tests into an actual test suite. Sorry about
the state of this. Please feel free to submit patches for this.

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

complete
--------

- Connection Management
- Querying
- Database Descriptions
- Binding Parameters (thanks to @gurzgri)

todo
----

- Option to emit on each record to avoid collecting the entire dataset first and
  increasing memory usage
- More error handling.
- Tests
- SQLGetData needs to support retrieving multiple chunks and concatenation in 
  the case of large column values

acknowledgements
----------------

- orlandov's node-sqlite binding was the framework I used to figure out using 
  eio's thread pool to handle blocking calls since non blocking odbc doesn't 
  seem to appear until 3.8.

authors
------

* Lee Smith (notwink@gmail.com)
* Dan VerWeire (dverweire@gmail.com)

license
-------

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

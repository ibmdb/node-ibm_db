NAME
----

node-odbc - An asynchronous Node interface to unixodbc and its supported drivers

SYNOPSYS
--------

	var sys     = require("sys");
	var odbc = require("odbc");

	var db = new odbc.Database();
	db.open("DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname", function(err)
	{
		db.query("select * from table", function(err, rows)
		{
			sys.debug(sys.inspect(rows));
			db.close(function(){});
		}
	});


DESCRIPTION
-----------

unixODBC binding to node. Needs a properly configured odbc(inst).ini.  Tested locally using the FreeTDS and Postgres drivers.

Installation
------------

- Make sure you have unixODBC installed and the drivers configured.
- node-waf configure build

BUGS
----

None known, but there might be one ;).


TODO
----

- Not complete, supports connection management and querying.
- Better (any?) error handling.
- Tests
- SQLGetData needs to support retrieving multiple chunks and concatenation in the case of large	column values

ACKNOWLEDGEMENTS
----------------

- orlandov's node-sqlite binding was the framework I used to figure out using eio's thread pool to handle blocking calls since non blocking odbc doesn't seem to appear until 3.8.

AUTHOR
------

Lee Smith (notwink@gmail.com)

NAME
----

node-odbc - An asynchronous Node interface to unixodbc and its supported drivers

SYNOPSYS
--------

	var sys  = require("sys");
	var odbc = require("odbc");

	var db = new odbc.Database();
	db.open("DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname", function(err)
	{
		db.query("select * from table", function(err, rows, moreResultSets)
		{
			sys.debug(sys.inspect(rows));
			db.close(function(){});
		});
	});


DESCRIPTION
-----------

unixODBC binding to node. Needs a properly configured odbc(inst).ini.  Tested locally using the FreeTDS and Postgres drivers.


INSTALLATION
------------

- Make sure you have the unixODBC binaries and unixODBC headers installed and the drivers configured.
	- On ubuntu and probably most linux distros the unixODBC header files are in the unixodbc-dev package (apt-get install unixodbc-dev)
- node-waf configure build



TIPS
----

- If you are using the FreeTDS ODBC driver and you have column names longer than 30 characters, you should add "TDS_Version=7.0" to your connection string to retrive the full column name.
  Example: 

	"DRIVER={FreeTDS};SERVER=host;UID=user;PWD=password;DATABASE=dbname;TDS_Version=7.0"


BUGS
----

None known, but there might be one ;).


TODO
----

- Not complete; supports connection management, querying and database descriptions
- Binding parameters (SQLBindParameter)?
- Option to emit on each record to avoid collecting the entire dataset first and increasing memory usage
- More error handling.
- Tests
- SQLGetData needs to support retrieving multiple chunks and concatenation in the case of large	column values

ACKNOWLEDGEMENTS
----------------

- orlandov's node-sqlite binding was the framework I used to figure out using eio's thread pool to handle blocking calls since non blocking odbc doesn't seem to appear until 3.8.

AUTHORS
------

Lee Smith (notwink@gmail.com)

Dan VerWeire (dverweire@gmail.com)
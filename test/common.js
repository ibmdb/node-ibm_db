var odbc = require("../");
//odbc.library = '/usr/lib/odbc/libsqlite3odbc-0.91';
//odbc.library = '/usr/lib/x86_64-linux-gnu/odbc/libtdsodbc';
//odbc.library = '/opt/sqlncli-11.0.1790.0/lib64/libsqlncli-11.0';

//exports.connectionString = "DRIVER={DB2 ODBC Driver};DATABASE=SAMPLE;UID=db2admin;PWD=db2admin;HOSTNAME=localhost;port=50000;PROTOCOL=TCPIP";
exports.connectionString = "";

try {
  exports.connectionObject = require('./config.testConnectionStrings.json');
}
catch (e) {
  exports.connectionObject = {
	DRIVER : "{DB2 ODBC Driver}",
	DATABASE : "SAMPLE",
	HOSTNAME : "localhost",
	UID : "db2admin",
	PWD : "db2admin",
	PORT : "50000",
	PROTOCOL : "TCPIP"
  };
}

for(key in exports.connectionObject) 
{
    exports.connectionString = exports.connectionString + key + "=" +
                               exports.connectionObject[key] + ";" ;
}

if (process.argv.length === 3) {
  exports.connectionString = process.argv[2];
}

exports.testConnectionStrings = [{ title : "DB2", 
                        connectionString : exports.connectionString }];
exports.benchConnectionStrings = exports.testConnectionStrings;

if (process.argv.length === 3) {
  //look through the testConnectionStrings to see if there is a title that matches
  //what was requested.
  var lookup = process.argv[2];
  
  exports.testConnectionStrings.forEach(function (connectionString) {
    if (connectionString && connectionString.title && connectionString.title == lookup) {
      exports.connectionString = connectionString.connectionString
    }
  });
}

exports.databaseName = "SAMPLE";
exports.tableName = "NODE_ODBC_TEST_TABLE";

exports.dropTables = function (db, cb) {
  db.query("drop table " + exports.tableName, cb);
};

exports.createTables = function (db, cb) {
  db.query("create table " + exports.tableName + " (COLINT INTEGER, COLDATETIME TIMESTAMP, COLTEXT VARCHAR(255))", cb);
};

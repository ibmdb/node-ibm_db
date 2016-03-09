var odbc = require("../");

exports.connectionString = "";

exports.connectionObject = {
	DRIVER : "{DB2 ODBC Driver}",
	DATABASE : "SAMPLE",
	HOSTNAME : "localhost",
	UID : "db2admin",
	PWD : "db2admin",
	PORT : "50000",
	PROTOCOL : "TCPIP"
  };

exports.connectionObject.DATABASE = process.env.IBM_DB_DBNAME 	|| exports.connectionObject.DATABASE;
exports.connectionObject.HOSTNAME = process.env.IBM_DB_HOSTNAME || exports.connectionObject.HOSTNAME;
exports.connectionObject.UID	  = process.env.IBM_DB_UID	|| exports.connectionObject.UID;
exports.connectionObject.PWD	  = process.env.IBM_DB_PWD	|| exports.connectionObject.PWD;
exports.connectionObject.PORT	  = process.env.IBM_DB_PORT	|| exports.connectionObject.PORT;
exports.connectionObject.PROTOCOL = process.env.IBM_DB_PROTOCOL || exports.connectionObject.PROTOCOL;

for(key in exports.connectionObject) 
{
    exports.connectionString = exports.connectionString + key + "=" +
                               exports.connectionObject[key] + ";" ;
}

//if (process.argv.length === 3) {
//  exports.connectionString = process.argv[2];
//}

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

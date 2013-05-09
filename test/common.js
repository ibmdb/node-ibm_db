var odbc = require("../");
//odbc.library = '/usr/lib/odbc/libsqlite3odbc-0.91';
//odbc.library = '/usr/lib/x86_64-linux-gnu/odbc/libtdsodbc';
//odbc.library = '/opt/sqlncli-11.0.1790.0/lib64/libsqlncli-11.0';

exports.connectionString = "DRIVER={SQLite3};DATABASE=data/sqlite-test.db";
//exports.connectionString = "DRIVER={MySQL};DATABASE=test;HOST=localhost;USER=test;";
//exports.connectionString = process.env.ODBC_CONNETION_STRING;

exports.connectionObject = {
	DRIVER : "{SQLITE3}",
	DATABASE : "data/sqlite-test.db"
};

exports.connections = [
	{
		DRIVER : "{SQLITE3}",
		DATABASE : "data/sqlite-test.db"
	}
];

exports.databaseName = "MAIN";
exports.tableName = "NODE_ODBC_TEST_TABLE";

exports.dropTables = function (db, cb) {
  db.query("drop table " + exports.tableName, cb);
};

exports.createTables = function (db, cb) {
  db.query("create table " + exports.tableName + " (COLINT INTEGER, COLDATETIME DATETIME, COLTEXT TEXT)", cb);
};

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
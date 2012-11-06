module.exports.connectionString = "DRIVER={SQLite3};DATABASE=data/sqlite-test.db";
//module.exports.connectionString = "DRIVER={MySQL};DATABASE=test;HOST=localhost;USER=test;";
//module.exports.connectionString = process.env.ODBC_CONNETION_STRING;

module.exports.connectionObject = {
	DRIVER : "{SQLITE3}",
	DATABASE : "data/sqlite-test.db"
};

module.exports.connections = [
	{
		DRIVER : "{SQLITE3}",
		DATABASE : "data/sqlite-test.db"
	}
];

module.exports.dropTables = function (db, cb) {
  db.query("drop table TEST", cb);
};

module.exports.createTables = function (db, cb) {
  db.query("create table TEST(COLINT INTEGER, COLDATETIME DATETIME, COLTEXT TEXT)", cb);
};
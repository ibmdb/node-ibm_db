var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
assert.equal(db.connected, true);
var err = null;

try {
  err = db.querySync("select invalid query");
}
catch (e) {
  err = e;
}
console.log(err);

db.closeSync();
assert.equal(err.error, "[node-ibm_db] Error in ODBCConnection::QuerySync while executing query.");
assert.ok(err instanceof Error, "Error should be an instance of Error, got: " + (err && err.constructor && err.constructor.name));
assert.ok(typeof err.message === 'string' && err.message.length > 0, "Error message should be a non-empty string");
assert.ok(typeof err.sqlcode === 'number', "sqlcode should be a number");
assert.ok(typeof err.sqlstate === 'string' && err.sqlstate.length > 0, "sqlstate should be a non-empty string");
console.log("test-querySync-select-with-exception: instanceof and properties OK");

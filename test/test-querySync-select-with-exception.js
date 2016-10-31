var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
assert.equal(db.connected, true);
var err = null;

try {
  var data = db.querySync("select invalid query");
}
catch (e) {
  console.log(e);

  err = e;
}

db.closeSync();
assert.equal(err.error, "[node-ibm_db] Error in ODBCConnection::QuerySync while executing query.");

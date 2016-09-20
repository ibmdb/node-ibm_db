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
if( /^win/.test(process.platform) )
  assert.equal(err.error, "[node-odbc] Error in ODBCConnection::QuerySync while executing query.");
else
  assert.equal(err.error, "[node-ibm_db] Error in ODBCConnection::QuerySync while executing query.");


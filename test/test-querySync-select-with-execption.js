var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  assert.equal(db.connected, true);
  var err = null;
  
  try {
    var data = db.querySync("select invalid query");
  }
  catch (e) {
    console.log(e);
    
    err = e;
  }
  
  db.close(function () {
    assert.equal(err.error, "[node-odbc] Error in ODBCConnection::QuerySync");
  });
});

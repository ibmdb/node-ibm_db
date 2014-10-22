var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
issueQuery();

function issueQuery() {
  var count = 0
    , time = new Date().getTime()
    , stmt
    , result
    , data
    ;
  
  assert.doesNotThrow(function () {
    stmt = db.prepareSync('select cast(? as datetime) as test');
  });

  assert.throws(function () {
    result = stmt.executeSync();
  });
  
  assert.doesNotThrow(function () {
    stmt.bindSync([0]);
  });
  
  assert.doesNotThrow(function () {
    result = stmt.executeSync();
  });
  
  assert.doesNotThrow(function () {
    data = result.fetchAllSync();
  });
  
  assert.ok(data);
  
  finish(0);
}

function finish(exitCode) {
  db.closeSync();
  
  console.log("connection closed");
    
  process.exit(exitCode || 0);
}

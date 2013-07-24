var common = require("./common")
  , odbc = require("../")
  , assert = require("assert")
  , db = new odbc.Database()
  , iterations = 100
  ;

db.openSync(common.connectionString);
  
issueQuery3(function () {
  finish();
});

function issueQuery3(done) {
  var count = 0
    , time = new Date().getTime();
  
  var stmt = db.prepareSync('select ? as test');
    
  for (var x = 0; x < iterations; x++) {
    (function (x) {
      stmt.bindSync([x]);
      var result = stmt.executeSync()
      cb(result, x);
      
    })(x);
  }
  
  function cb (result, x) {
    assert.deepEqual(result.fetchAllSync(), [ { test : x } ]);
    
    result.closeSync();

    if (++count == iterations) {
      var elapsed = new Date().getTime() - time;
      
      console.log("%d queries issued in %d seconds, %d/sec : Prepare - Bind - Execute - CloseSync", count, elapsed/1000, Math.floor(count/(elapsed/1000)));
      return done();
    }
  }
}

function finish() {
  db.closeSync();
  console.log("connection closed");
}

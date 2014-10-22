var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  , iterations = 1000
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
      stmt.bind([x], function (err) {
        if (err) {
          console.log(err);
          return finish();
        }
        
        //console.log(x);
        
        stmt.execute(function (err, result) {
          cb(err, result, x);
        });
      });
    })(x);
  }
  
  function cb (err, result, x) {
    if (err) {
      console.error(err);
      return finish();
    }
    
    var a = result.fetchAllSync();
    
    assert.deepEqual(a, [{ test : x }]);
    
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

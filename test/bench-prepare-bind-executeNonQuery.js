var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , iterations = 10000
  ;

db.open(common.connectionString, function(err){ 
  if (err) {
    console.error(err);
    process.exit(1);
  }
  
  issueQuery2(function () {
    finish();
  });
});

function issueQuery2(done) {
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
        
        stmt.executeNonQuery(cb);
      });
    })(x);
  }
  
  function cb (err, data) {
    if (err) {
      console.error(err);
      return finish();
    }
    
    if (++count == iterations) {
      var elapsed = new Date().getTime() - time;
      
      console.log("%d queries issued in %d seconds, %d/sec : Prepare - Bind - ExecuteNonQuery ", count, elapsed/1000, Math.floor(count/(elapsed/1000)));
      return done();
    }
  }
}

function finish() {
  db.close(function () {});
}

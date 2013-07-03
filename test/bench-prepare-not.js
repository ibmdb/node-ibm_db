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
  
  issueQuery1(function () {
    finish();
  });
});

function issueQuery1(done) {
  var count = 0
    , time = new Date().getTime();
  
  for (var x = 0; x < iterations; x++) {
    db.query("select 1 + ? as test", [1], cb);
  }
  
  function cb (err, data) {
    if (err) {
      console.error(err);
      return finish();
    }
    
    if (++count == iterations) {
      var elapsed = new Date().getTime() - time;
      
      console.log("%d queries issued in %d seconds, %d/sec : Query", count, elapsed/1000, Math.floor(count/(elapsed/1000)));
      return done();
    }
  }
}

function finish() {
  db.close(function () {});
}

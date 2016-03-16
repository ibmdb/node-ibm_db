var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  //, iterations = 10000
  , iterations = 100
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
    db.query("select 1 + ? as test from sysibm.sysdummy1", [1], cb);
  }
  
  function cb (err, data) {
    if (err) {
      console.error(err);
      return finish();
    }
    
    if (++count == iterations) {
      var elapsed = (new Date().getTime() - time)/1000;
      process.stdout.write("(" + count + " queries issued in " + elapsed + " seconds, " + (count/elapsed).toFixed(2) + " query/sec)");
      return done();
    }
  }
}

function finish() {
  db.close(function () {});
}

var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , iterations = 100
  //, iterations = 10000
  ;

db.open(common.connectionString, function(err){ 
  if (err) {
    console.error(err);
    process.exit(1);
  }
  
  issueQuery3(function () {
    finish();
  });
});

function issueQuery3(done) {
  var count = 0
    , time = new Date().getTime();
  
  var stmt = db.prepareSync('select cast(? as INTEGER) as test from sysibm.sysdummy1');
  
  for (var x = 0; x < iterations; x++) {
    (function (x) {
      stmt.execute([x], cb);
    })(x);
  }
  
  function cb (err, result) {
    if (err) {
      console.error(err);
      return finish();
    }
    
    //console.log(result.fetchAllSync());
    
    result.closeSync();

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

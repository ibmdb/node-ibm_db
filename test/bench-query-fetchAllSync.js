var common = require("./common")
, odbc = require("../")
, db = new odbc.Database();

db.open(common.connectionString, function(err){ 
  if (err) {
    console.error(err);
    process.exit(1);
  }
  
  issueQuery();
});

function issueQuery() {
  var count = 0
  , iterations = 10000
  , time = new Date().getTime();
  
  for (var x = 0; x < iterations; x++) {
    db.queryResult("select 1 + 1 as test", cb);
  }
  
  function cb (err, result) {
    if (err) {
      console.error(err);
      return finish();
    }
    
    var data = result.fetchAllSync();
    result.closeSync();
        
    if (++count == iterations) {
      var elapsed = new Date().getTime() - time;
        
      console.log("%d queries issued in %d seconds, %d/sec", count, elapsed/1000, Math.floor(count/(elapsed/1000)));
      return finish();
    }
  }

  function finish() {
    db.close(function () {});
  }
}
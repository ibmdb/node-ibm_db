var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database({ fetchMode : odbc.FETCH_ARRAY });

db.open(common.connectionString, function(err){ 
  if (err) {
    console.error(err);
    process.exit(1);
  }
  
  issueQuery();
});

function issueQuery() {
  var count = 0
    , iterations = 100
    //, iterations = 10000
    , time = new Date().getTime();
  
  for (var x = 0; x < iterations; x++) {
    var data = db.querySync("select 1 + 1 as test from sysibm.sysdummy1");
    count += 1;
  }
  
  var elapsed = (new Date().getTime() - time)/1000;
  process.stdout.write("(" + count + " queries issued in " + elapsed + " seconds, " + (count/elapsed).toFixed(2) + " query/sec)");
  db.close(function () {});
}

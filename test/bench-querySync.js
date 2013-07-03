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
    var data = db.querySync("select 1 + 1 as test");
    count += 1;
  }
  
  var elapsed = new Date().getTime() - time;
   
  console.log("%d queries issued in %d seconds, %d/sec", count, elapsed/1000, Math.floor(count/(elapsed/1000)));
      
  db.close(function () { });
}
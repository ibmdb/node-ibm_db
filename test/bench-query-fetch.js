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
    
    fetchAll(result);
  }
  
  function fetchAll(rs) {
    rs.fetch(function (err, data) {
      if (err) {
        console.error(err);
        return finish();
      }
      
      //if data is null, then no more data
      if (!data) {
        rs.closeSync();
        
        if (++count == iterations) {
          var elapsed = new Date().getTime() - time;
          
          console.log("%d queries issued in %d seconds, %d/sec", count, elapsed/1000, Math.floor(count/(elapsed/1000)));
          return finish();
        }
      }
      else {
        fetchAll(rs);
      }
    }); 
  }
  
  function finish() {
    db.close(function () {});
  }
}
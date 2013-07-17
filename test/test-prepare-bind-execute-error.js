var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err){ 
  if (err) {
    console.error(err);
    process.exit(1);
  }
  
  issueQuery();
});

function issueQuery() {
  var count = 0
    , time = new Date().getTime();
  
  try {
    var stmt = db.prepareSync('select cast(? as datetime) as test');
  }
  catch (e) {
    console.log(e);
    return finish(1);
  }
  
  stmt.bind([], function (err) {
    if (err) {
      console.log(err);
      return finish(1);
    }
    
    stmt.execute(function (err, result) {
      if (err) {
        console.log(err);
        return finish(1);
      }
      
      console.log(result.fetchAllSync());
      
      finish(0);
    });
  });
}

function finish(exitCode) {
  db.close(function () {
    console.log("connection closed");
    
    process.exit(exitCode || 0);
  });
}

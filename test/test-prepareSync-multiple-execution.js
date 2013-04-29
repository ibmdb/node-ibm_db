var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

var count = 0;
var iterations = 10;

db.open(common.connectionString, function(err) {
  if(err) {
    console.log(err)
    
    return finish(1);
  }
  
  common.createTables(db, function (err, data) {
    if (err) {
      console.log(err);
      
      return finish(2);
    }
    
    var stmt = db.prepareSync("insert into " + common.tableName + " (colint, coltext) VALUES (?, ?)");
    assert.equal(stmt.constructor.name, "ODBCStatement");
    
    recursive(stmt);
  });
});

function finish(retValue) {
  console.log("finish exit value: %s", retValue);
  
  db.close(function (err) {
    if (err) console.log (err);
    
    process.exit(retValue || 0);
  });
}

function recursive (stmt) {
  try {
    var result = stmt.bindSync([4, 'hello world']);
    assert.equal(result, true);
  }
  catch (e) {
    console.log(e.message);
    finish(3);
  }
  
  stmt.execute(function (err, result) {
    if (err) {
      console.log(err.message);
      
      return finish(4);
    }
    
    result.closeSync();
    count += 1;
    
    console.log("count %s, iterations %s", count, iterations);
    
    if (count <= iterations) {
      setTimeout(function(){
        recursive(stmt);
      },100);
    }
    else {
      console.log(db.querySync("select * from " + common.tableName));
      
      common.dropTables(db, function () {
        return finish(0);
      });
    }
  });
}

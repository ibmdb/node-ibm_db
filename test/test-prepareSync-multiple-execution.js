var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  ;

var count = 0;
var iterations = 10;

var conn = ibmdb.openSync(common.connectionString);
  
common.dropTables(conn, function () {
  common.createTables(conn, function (err, data) {
    if (err) {
      console.log(err);
      
      return finish(2);
    }
    
    conn.beginTransactionSync();
    var stmt = conn.prepareSync("insert into " + common.tableName + " (colint, coltext) VALUES (?, ?)");
    assert.equal(stmt.constructor.name, "ODBCStatement");
    
    recursive(stmt);
  });
});

function finish(retValue) {
  console.log("finish exit value: %s", retValue);
  
  conn.closeSync();
  process.exit(retValue || 0);
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
    
    console.log("Executed iteration %s out of %s.", count, iterations);
    
    if (count < iterations) {
      setTimeout(function(){
        recursive(stmt);
      },100);
    }
    else {
      console.log("Inserted Rows = ");
      console.log(conn.querySync("select * from " + common.tableName));
      try {
        var result = stmt.bindSync(['abc', 'hello world']);
        assert.equal(result, true);
      }
      catch (e) {
        console.log(e.message);
        finish(5);
      }
  
      stmt.execute(function (err, result) {
        if (err) {   // Expecting Error here.
          console.log(err.message);
          conn.rollbackTransactionSync();
          var data = conn.querySync("select * from " + common.tableName);
          console.log("After roolback, selected rows = ", data);
          assert.deepEqual(data, []);
          common.dropTables(conn, function () { return finish(0); });
        }
        else
        {
          result.closeSync();
          conn.commitTransactionSync();
          var data = conn.querySync("select * from " + common.tableName);
          console.log("After commit, selected rows = ", data);
          common.dropTables(conn, function () { return finish(6); });
        }
      });
    }
  });
}

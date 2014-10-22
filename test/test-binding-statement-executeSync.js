var common = require("./common")
  , odbc = require("../")
  , db = new odbc.ODBC()
  , assert = require("assert")
  , exitCode = 0
  ;

db.createConnection(function (err, conn) {
  conn.openSync(common.connectionString);
  
  conn.createStatement(function (err, stmt) {
    var r, result, caughtError;
    
    //try excuting without preparing or binding.
    try {
      result = stmt.executeSync();
    }
    catch (e) {
      caughtError = e;
    }
    
    try {
      assert.ok(caughtError);
    }
    catch (e) {
      console.log(e.message);
      exitCode = 1;
    }
    
    //try incorrectly binding a string and then executeSync
    try {
      r = stmt.bind("select 1 + 1 as col1");
    }
    catch (e) {
      caughtError = e;
    }
    
    try {
      assert.equal(caughtError.message, "Argument 1 must be an Array");
      
      r = stmt.prepareSync("select 1 + ? as col1 from SYSIBM.SYSDUMMY1");
      assert.equal(r, true, "prepareSync did not return true");
      
      r = stmt.bindSync([2]);
      assert.equal(r, true, "bindSync did not return true");
      
      result = stmt.executeSync();
      assert.equal(result.constructor.name, "ODBCResult");
      
      r = result.fetchAllSync();
      assert.deepEqual(r, [ { COL1: 3 } ]);
      
      r = result.closeSync();
      assert.equal(r, true, "closeSync did not return true");
      
      result = stmt.executeSync();
      assert.equal(result.constructor.name, "ODBCResult");
      
      r = result.fetchAllSync();
      assert.deepEqual(r, [ { COL1: 3 } ]);
      
      console.log(r);
    }
    catch (e) {
      console.log(e);
      
      exitCode = 1;
    }
    
    conn.closeSync();
    
    if (exitCode) {
      console.log("failed");
    }
    else {
      console.log("success");
    }
    
    process.exit(exitCode);
  });
});

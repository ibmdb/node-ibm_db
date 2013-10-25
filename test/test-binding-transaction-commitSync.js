var common = require("./common")
  , odbc = require("../")
  , db = new odbc.ODBC()
  , assert = require("assert")
  , exitCode = 0
  ;

db.createConnection(function (err, conn) {
  conn.openSync(common.connectionString);
  
  common.createTables(conn, function (err, data) {
    try {
      conn.beginTransactionSync();
      
      var result = conn.querySync("insert into " + common.tableName + " (COLINT, COLDATETIME, COLTEXT) VALUES (42, null, null)" );

      conn.endTransactionSync(true); //rollback
      
      result = conn.querySync("select * from " + common.tableName);
      
      assert.deepEqual(result.fetchAllSync(), []);
    }
    catch (e) {
      console.log("Failed when rolling back");
      console.log(e);
      exitCode = 1
    }  
      
    try {
      //Start a new transaction
      conn.beginTransactionSync();
      
      result = conn.querySync("insert into " + common.tableName + " (COLINT, COLDATETIME, COLTEXT) VALUES (42, null, null)" );

      conn.endTransactionSync(false); //commit
      
      result = conn.querySync("select * from " + common.tableName);
      
      assert.deepEqual(result.fetchAllSync(), [ { COLINT: 42, COLDATETIME: null, COLTEXT: null } ]);
    }
    catch (e) {
      console.log("Failed when committing");
      console.log(e);
      
      exitCode = 2;
    }
    
    common.dropTables(conn, function (err) {
      conn.closeSync();
      process.exit(exitCode);
    });
  });
});

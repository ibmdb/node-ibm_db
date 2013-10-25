var common = require("./common")
  , odbc = require("../")
  , db = new odbc.ODBC()
  , assert = require("assert")
  , exitCode = 0
  ;

db.createConnection(function (err, conn) {
  
  conn.openSync(common.connectionString);
  
  common.createTables(conn, function (err, data) {
    test1()
    
    function test1() {
      conn.beginTransaction(function (err) {
        if (err) {
          console.log("Error beginning transaction.");
          console.log(err);
          exitCode = 1
        }
        
        var result = conn.querySync("insert into " + common.tableName + " (COLINT, COLDATETIME, COLTEXT) VALUES (42, null, null)" );

        //rollback
        conn.endTransaction(true, function (err) {
          if (err) {
            console.log("Error rolling back transaction");
            console.log(err);
            exitCode = 2
          }
          
          result = conn.querySync("select * from " + common.tableName);
          data = result.fetchAllSync();
          
          assert.deepEqual(data, []);
          
          test2();
        });
      });
    }
    
    function test2 () {
      //Start a new transaction
      conn.beginTransaction(function (err) {
        if (err) {
          console.log("Error beginning transaction");
          console.log(err);
          exitCode = 3
        }
          
        result = conn.querySync("insert into " + common.tableName + " (COLINT, COLDATETIME, COLTEXT) VALUES (42, null, null)" );

        //commit
        conn.endTransaction(false, function (err) {
          if (err) {
            console.log("Error committing transaction");
            console.log(err);
            exitCode = 3
          }
          
          result = conn.querySync("select * from " + common.tableName);
          data = result.fetchAllSync();
          
          assert.deepEqual(data, [ { COLINT: 42, COLDATETIME: null, COLTEXT: null } ]);
          
          finish();
        });
      });
    }
    
    function finish() {
      common.dropTables(conn, function (err) {
        conn.closeSync();
        process.exit(exitCode);
      });
    }
  });
});

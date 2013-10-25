var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  , exitCode = 0
  ;


db.openSync(common.connectionString);

common.createTables(db, function (err, data) {
  test1()
  
  function test1() {
    db.beginTransaction(function (err) {
      if (err) {
        console.log("Error beginning transaction.");
        console.log(err);
        exitCode = 1
      }
      
      var result = db.querySync("insert into " + common.tableName + " (COLINT, COLDATETIME, COLTEXT) VALUES (42, null, null)" );

      //rollback
      db.endTransaction(true, function (err) {
        if (err) {
          console.log("Error rolling back transaction");
          console.log(err);
          exitCode = 2
        }
        
        data = db.querySync("select * from " + common.tableName);
        
        assert.deepEqual(data, []);
        
        test2();
      });
    });
  }
  
  function test2 () {
    //Start a new transaction
    db.beginTransaction(function (err) {
      if (err) {
        console.log("Error beginning transaction");
        console.log(err);
        exitCode = 3
      }
      
      result = db.querySync("insert into " + common.tableName + " (COLINT, COLDATETIME, COLTEXT) VALUES (42, null, null)" );
      
      //commit
      db.endTransaction(false, function (err) {
        if (err) {
          console.log("Error committing transaction");
          console.log(err);
          exitCode = 3
        }
        
        data = db.querySync("select * from " + common.tableName);
        
        assert.deepEqual(data, [ { COLINT: 42, COLDATETIME: null, COLTEXT: null } ]);
        
        finish();
      });
    });
  }
  
  function finish() {
    common.dropTables(db, function (err) {
      db.closeSync();
      process.exit(exitCode);
    });
  }
});



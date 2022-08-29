var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
  
common.dropTables(db, function () {
  common.createTables(db, function () {
    
    db.describe({
      database : common.databaseName
      , table : common.tableName
    }, function (err, data) {
      console.log("db.describe data = ", data);
      assert.ok(data.length, "No records returned when attempting to describe the tabe " + common.tableName);
      test2();
    });
  });
});

async function test2() {
    var result = "";

    result = await db.describe({ database : common.databaseName , table : common.tableName });
    console.log("db.describe result = ", result);
    result = await db.describe({ database : common.databaseName });
    console.log("db.describe result = ", result);
    db.closeSync();
}

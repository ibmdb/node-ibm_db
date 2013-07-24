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
    }, function (err, data) {
      db.closeSync();
      assert.ok(data.length, "No records returned when attempting to describe the database");
    });
  });
});

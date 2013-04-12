var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  
  common.dropTables(db, function () {
    common.createTables(db, function () {
      
      db.describe({
        database : common.databaseName
        , table : common.tableName
      }, function (err, data) {
        db.close(function () {
          assert.equal(err, null);
          assert.ok(data.length, "No records returned when attempting to describe the tabe " + common.tableName);
        });
      });
    });
  });
});
var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  , insertCount = 0;
  ;

db.open(common.connectionString, function(err) {
  common.dropTables(db, function () {
    common.createTables(db, function (err) {
      assert.equal(err, null);
      
      db.query("insert into " + common.tableName + " (COLTEXT) values ('sandwich')", insertCallback);
      db.query("insert into " + common.tableName + " (COLTEXT) values ('fish')", insertCallback);
      db.query("insert into " + common.tableName + " (COLTEXT) values ('scarf')", insertCallback);
      
    });
  });
});

function insertCallback(err, data) {
  assert.equal(err, null);
  assert.deepEqual(data, []);
  
  insertCount += 1;
  
  if (insertCount === 3) {
    common.dropTables(db, function () {
      db.close(function () {
        console.error("Done");
      });
    });
  }
}
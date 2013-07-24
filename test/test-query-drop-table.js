var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
common.dropTables(db, function (err, data) {
  db.closeSync();
  assert.equal(err, null);
  assert.deepEqual(data, []);
});


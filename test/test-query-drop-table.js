var common = require("./common")
  , odbc = require("../odbc.js")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  common.dropTables(db, function (err, data) {
    db.close(function () {
      assert.equal(err, null);
      assert.deepEqual(data, []);
    });
  });
});

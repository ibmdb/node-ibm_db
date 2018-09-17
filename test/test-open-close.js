var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert");

assert.equal(db.connected, false);

db.query("select * from " + common.tableName, function (err, rs, sqlca) {
  assert.deepEqual(err.message, 'Connection not open.');
  assert.deepEqual(rs, []);
  assert.equal(sqlca.sqlcode, -30081);
  assert.equal(db.connected, false);
});

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  assert.equal(db.connected, true);
  
  db.close(function () {
    assert.equal(db.connected, false);
    
    db.query("select * from " + common.tableName, function (err, rs, sqlca) {
      assert.deepEqual(err.message, 'Connection not open.');
      assert.deepEqual(rs, []);
      assert.equal(sqlca.sqlcode, -30081);
      assert.equal(db.connected, false);
    });
  });
});

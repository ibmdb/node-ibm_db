var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

assert.equal(db.connected, false);

db.query("select * from test", function (err, rs, sqlca) {
  assert.deepEqual(err.message, 'Connection not open.');
  assert.deepEqual(rs, []);
  assert.equal(sqlca.sqlcode, -30081);
  assert.equal(db.connected, false);
});

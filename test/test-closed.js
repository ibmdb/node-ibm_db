var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

assert.equal(db.connected, false);

db.query("select * from test", function (err, rs, moreResultSets) {
  assert.deepEqual(err, { message: 'Connection not open.' });
  assert.deepEqual(rs, []);
  assert.equal(moreResultSets, false);
  assert.equal(db.connected, false);
});

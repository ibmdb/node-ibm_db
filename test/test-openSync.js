var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert");

assert.equal(db.connected, false);

db.query("select * from " + common.tableName, function (err, rs, moreResultSets) {
  assert.deepEqual(err, { message: 'Connection not open.' });
  assert.deepEqual(rs, []);
  assert.equal(moreResultSets, false);
  assert.equal(db.connected, false);
});

console.log("Attempting to connect to: %s", common.connectionString);

try {
  db.openSync(common.connectionString);
}
catch(e) {
  console.log(e);
  assert.deepEqual(e, null);
}

try {
  db.closeSync();
}
catch(e) {
  console.log(e);
  assert.deepEqual(e, null);
}

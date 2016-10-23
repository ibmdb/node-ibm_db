var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
assert.equal(db.connected, true);
var stream = db.queryStream("select 1 as COLINT, 'some test' as COLTEXT FROM SYSIBM.SYSDUMMY1");
stream.once('data', function (data) {
  assert.deepEqual(data, { COLINT: '1', COLTEXT: 'some test' });
}).once('error', function (err) {
  db.closeSync();
  throw err;
}).once('end', function () {
  db.closeSync();
});

var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
assert.equal(db.connected, true);

var stream = db.queryStream("🐌");
// adding the 'data' eventhandler starts the stream
stream.once('data', function (data) {
  throw new Error('data should not return from an erroring queryStream');
}).once('error', function (err) {
  assert.equal(err.state, '42601');
  db.closeSync();
});

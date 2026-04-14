var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

const os = require("os");

db.openSync(common.connectionString);
assert.equal(db.connected, true);

var stream = db.queryStream("wrong query");
// adding the 'data' eventhandler starts the stream
stream.once('data', function (data) {
  throw new Error("data should not return from an erroring queryStream.");
}).once('error', function (err) {
  assert.ok(err instanceof Error, "Async error should be an instance of Error, got: " + (err && err.constructor && err.constructor.name));
  assert.equal(err.sqlstate, (os.type() === "OS/390")?'37000':'42601');
  assert.ok(typeof err.sqlcode === 'number', "sqlcode should be a number");
  assert.ok(typeof err.message === 'string' && err.message.length > 0, "Error message should be a non-empty string");
  db.close(function(){
      console.log("Error test for queryStream completed successfully.");
  });
});

odbc.open(common.connectionString, function(err, conn) {
    if(err) return console.log(err);
    assert.equal(conn.connected, true);

    var sql = "select 1 as COLINT, 'some test' as COLTEXT FROM SYSIBM.SYSDUMMY1";
    var stream = conn.queryStream(sql);

    stream.once('data', function (data) {
      assert.deepEqual(data, { COLINT: '1', COLTEXT: 'some test' });
      console.log("Select test for queryStream completed successfully.");
    }).once('error', function (err) {
      conn.closeSync();
      throw err;
    }).once('end', function () {
      conn.close(function(){ console.log("done.") });
    });
});


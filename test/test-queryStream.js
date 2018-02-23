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
  assert.equal(err.state, (os.type() === "OS/390")?'37000':'42601');
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


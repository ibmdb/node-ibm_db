// Run this test case individually when build with --debug option and
// look for any error/return code from SetConnectionAttributes function.

// * test setting connectTimeout via the constructor works
// * test setting systemNaming via the constructor works
//   systemNaming is applicable only for i5/OS server.

var common = require("./common")
  , odbc = require("../")
  , assert = require("assert");

var db = new odbc.Database({ connectTimeout : 10, systemNaming : true })

db.open(common.connectionString, function(err) {
  assert.equal(db.conn.connectTimeout, 10);
  
  assert.equal(err, null);
  assert.equal(db.connected, true);

  assert.equal(db.conn.systemNaming, true);
  
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


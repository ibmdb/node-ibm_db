var common = require("./common")
  , ibmdb = require("../")
  , db = new ibmdb.Database()
  , assert = require("assert");

db.query("select * from " + common.tableName).then(null, function (err) {
  assert.deepEqual(err, { message: 'Connection not open.' });
  assert.equal(db.connected, false);
});

db.open(common.connectionString).then(function(res) {
  console.log("db = ", db);
  assert.equal(db.connected, true);
  db.query("select * from " + common.tableName).then(null, function (err) {
	var errorFound = false;
	if(err) console.log(err);
	if (err.message) {
	    var errorFound = err.message.includes("SQL0204N");
	}
    assert.equal(errorFound, true);
    assert.equal(db.connected, true);
    db.close().then(function () {
      assert.equal(db.connected, false);

      db.query("select * from " + common.tableName).then(null, function (err) {
	    if(err) console.log(err);
        assert.deepEqual(err, { message: 'Connection not open.' });
        assert.equal(db.connected, false);
      });
    });
  });
})
.catch(function(err){console.log(err);})
.done();

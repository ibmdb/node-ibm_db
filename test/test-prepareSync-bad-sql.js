var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

const os = require("os");

db.openSync(common.connectionString);
assert.equal(db.connected, true);

try {
  var stmt = db.prepareSync("asdf asdf asdf asdf sadf ");
} catch (e) {
  // ODBC driver on z/OS does not have deferred prepare enabled
  // by default. As a result, we expect the error to be thrown
  // on the prepare.
  assert.deepEqual(os.type(), "OS/390", "With deferred prepare in Db2Connect on distributed platforms, bad sql should not throw error.");
  process.exit(0);
}
assert.equal(stmt.constructor.name, "ODBCStatement");
  
stmt.bindSync(["hello world", 1, null]);
    
stmt.execute(function (err, result) {
  assert.ok(err);

  stmt.executeNonQuery(function (err, count) {
    assert.ok(err);

    db.close(function () {});
  });
});


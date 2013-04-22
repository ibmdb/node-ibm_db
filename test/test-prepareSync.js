var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  assert.equal(db.connected, true);
  
  var stmt = db.prepareSync("select ? as col1, ? as col2, ? as col3");
  assert.equal(stmt.constructor.name, "ODBCStatement");
    
  stmt.bindSync(["hello world", 1, null]);
      
  stmt.execute(function (err, result) {
    assert.equal(err, null);
    assert.equal(result.constructor.name, "ODBCResult");
    
    result.fetchAll(function (err, data) {
      assert.equal(err, null);
      console.log(data);
      
      result.closeSync();
      
      db.close(function () {
        assert.deepEqual(data, [{ col1: "hello world", col2 : 1, col3 : null }]);
      });
    });
  });
});

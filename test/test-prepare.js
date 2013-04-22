var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  assert.equal(db.connected, true);
  
  db.prepare("select ? as col1", function (err, stmt) {
    assert.equal(err, null);
    assert.equal(stmt.constructor.name, "ODBCStatement");
    
    stmt.bind(["hello world"], function (err) {
      assert.equal(err, null);
      
      stmt.execute(function (err, result) {
        assert.equal(err, null);
        assert.equal(result.constructor.name, "ODBCResult");
        
        result.fetchAll(function (err, data) {
          assert.equal(err, null);
          console.log(data);
          
          result.closeSync();
          
          db.close(function () {
            assert.deepEqual(data, [{ col1: "hello world" }]);
          });
        });
      });
    });
  });
});

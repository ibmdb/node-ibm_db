var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);

assert.equal(db.connected, true);
  
db.prepare("select 'abc' as col1 from SYSIBM.SYSDUMMY1", function (err, stmt) {
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
        
        db.closeSync();
        assert.deepEqual(data, [{ col1: "hello world" }]);
      });
    });
  });
});


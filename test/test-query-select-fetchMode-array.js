var common = require("./common")
  , odbc = require("../")
  , db = odbc({ fetchMode : odbc.FETCH_ARRAY })
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  assert.equal(db.connected, true);
  
  db.query("select 1 as COLINT, 'some test' as COLTEXT ", function (err, data) {
    assert.equal(err, null);
    
    db.close(function () {
      assert.deepEqual(data, [[1,"some test"]]);
    });
  });
});

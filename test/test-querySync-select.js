var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  assert.equal(db.connected, true);
  
  var data = db.querySync("select 1 as \"COLINT\", 'some test' as \"COLTEXT\"");
  
  db.close(function () {
    assert.deepEqual(data, [{ COLINT: 1, COLTEXT: 'some test' }]);
  });
});

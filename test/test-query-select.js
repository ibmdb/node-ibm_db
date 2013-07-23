var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  db.query("select 1 as \"COLINT\", 'some test' as \"COLTEXT\"", function (err, data) {
    db.close(function () {
      assert.equal(err, null);
      assert.deepEqual(data, [{ COLINT: '1', COLTEXT: 'some test' }]);
    });
  });
});

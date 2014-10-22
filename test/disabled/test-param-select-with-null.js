var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert");


db.open(common.connectionString, function (err) {
  assert.equal(err, null);
  
  db.query("select ? as \"NULLCOL1\" "
    , [null]
    , function (err, data, more) {
    	if (err) { console.error(err) }
        db.close(function () {
          assert.equal(err, null);
          assert.deepEqual(data, [{
            NULLCOL1: null 
          }]);
        });
    });
});

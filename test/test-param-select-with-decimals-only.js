var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert");


db.open(common.connectionString, function (err) {
  assert.equal(err, null);
  
  db.query("select ? as \"DECCOL1\" "
    , [5.5]
    , function (err, data, more) {
        db.close(function () {
          assert.equal(err, null);
          assert.deepEqual(data, [{
            DECCOL1: 5.5
          }]);
        });
    });
});

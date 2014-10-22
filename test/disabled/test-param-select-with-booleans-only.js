var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert");


db.open(common.testConnectionStrings[0].connectionString, function (err) {
  assert.equal(err, null);
  
  db.query("select ? as \"TRUECOL\", ? as \"FALSECOL\" FROM SYSIBM.SYSDUMMY1"
    , [true, false]
    , function (err, data, more) {
        db.close(function () {
          assert.equal(err, null);
          assert.deepEqual(data, [{
            TRUECOL: true,
            FALSECOL: false
          }]);
        });
    });
});

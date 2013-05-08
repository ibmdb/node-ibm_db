var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert");


db.open(common.connectionString, function (err) {
  assert.equal(err, null);
  
  db.query("select ? as INTCOL1, ? as INTCOL2, ? as INTCOL3, ? as FLOATCOL4, ? as FLOATYINT"
    , [5, 3, 1, 1.23456789012345, 12345.000]
    , function (err, data, more) {
        db.close(function () {
          assert.equal(err, null);
          assert.deepEqual(data, [{
            INTCOL1: 5,
            INTCOL2: 3,
            INTCOL3: 1,
            FLOATCOL4 : 1.23456789012345,
            FLOATYINT : 12345
          }]);
        });
    });
});

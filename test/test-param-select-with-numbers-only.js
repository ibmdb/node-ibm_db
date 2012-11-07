var common = require("./common")
  , odbc = require("../odbc.js")
  , db = new odbc.Database()
  , assert = require("assert");


db.open(common.connectionString, function (err) {
  assert.equal(err, null);
  
  db.query("select ? as INTCOL1, ? as INTCOL2, ? as INTCOL3 "
    , [5, 3, 1]
    , function (err, data, more) {
        db.close(function () {
          assert.equal(err, null);
          assert.deepEqual(data, [{
            INTCOL1: 5,
            INTCOL2: 3,
            INTCOL3: 1 
          }]);
        });
    });
});

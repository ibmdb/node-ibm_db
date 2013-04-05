var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert");


db.open(common.connectionString, function (err) {
  assert.equal(err, null);
  
  db.query("select ? as TEXTCOL, ? as TEXTCOL2, ? as TEXTCOL3"
    , ["fish", "asdf", "1234"]
    , function (err, data, more) {
        db.close(function () {
          assert.equal(err, null);
          assert.deepEqual(data, [{
            TEXTCOL: 'fish',
            TEXTCOL2: 'asdf',
            TEXTCOL3: '1234'
          }]);
        });
    });
});

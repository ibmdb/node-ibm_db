var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert");


db.open(common.connectionString, function (err) {
  assert.equal(err, null);
  
  db.query("select ? as \"TEXTCOL1\", ? as \"TEXTCOL2\", ? as \"NULLCOL1\" "
    , ["something", "something", null]
    , function (err, data, more) {
    	if (err) { console.error(err) }
        db.close(function () {
          assert.equal(err, null);
          assert.deepEqual(data, [{
            TEXTCOL1: "something",
            TEXTCOL2: "something",
            NULLCOL1: null 
          }]);
        });
    });
});

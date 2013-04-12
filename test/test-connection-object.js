var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionObject, function(err){
  assert.equal(err, null);

  db.close(function () {
    assert.equal(db.connected, false);
  });
});

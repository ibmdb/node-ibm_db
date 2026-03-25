var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionObject, function(err){
  assert.equal(err, null);
  console.log("Connection opened successfully");
  let result = db.querySync("select 1 as COL1 from sysibm.sysdummy1");
  assert.equal(result.length, 1);
  console.log("Selected Data = ", result);
  db.close(function () {
    assert.equal(db.connected, false);
  });
});

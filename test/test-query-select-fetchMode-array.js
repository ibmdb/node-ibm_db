var common = require("./common")
  , odbc = require("../")
  , db = odbc({ fetchMode : odbc.FETCH_ARRAY })
  , assert = require("assert")
  ;

db.openSync(common.connectionString);

assert.equal(db.connected, true);

db.query("select 1 as COLINT, 'some test' as COLTEXT ", function (err, data) {
  assert.equal(err, null);
  
  db.closeSync();
  assert.deepEqual(data, [[1,"some test"]]);
});


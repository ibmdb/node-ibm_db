var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);

assert.equal(db.connected, true);

db.queryResult("select 1 as COLINT, 'some test' as COLTEXT union select 2, 'something else'  FROM SYSIBM.SYSDUMMY1", function (err, result) {
  assert.equal(err, null);
  assert.equal(result.constructor.name, "ODBCResult");
  
  var data = result.fetchAllSync();
  
  db.closeSync();
  assert.deepEqual(data, [
      {"COLINT":1,"COLTEXT":"some test"}
    ,{"COLINT":2,"COLTEXT":"something else"}
  ]);
});


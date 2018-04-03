var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
assert.equal(db.connected, true);

var data = db.querySync("select 1 as \"COLINT\", 'some test' as \"COLTEXT\" FROM SYSIBM.SYSDUMMY1");
assert.deepEqual(data, [{ COLINT: 1, COLTEXT: 'some test' }]);

// Test multiple result set returned by querySync.
db.fetchMode = 3; // Fetch in array mode.
data[0] = db.querySync("select 1, 2, 3 from sysibm.sysdummy1");
data[1] = db.querySync("select 4, 5, 6 from sysibm.sysdummy1");
console.log(data);
assert.deepEqual(data, [ [ [ 1, 2, 3 ] ], [ [ 4, 5, 6 ] ] ]);

db.fetchMode = 4; // Fetch in object mode. It is default mode too.
data[0] = db.querySync("select 1 from sysibm.sysdummy1");
data[1] = db.querySync("select 2 from sysibm.sysdummy1");
data[2] = db.querySync("select 3 from sysibm.sysdummy1");
console.log(data);
assert.deepEqual(data[2], [ { '1': 3 } ]);
db.closeSync();



var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
var data;

try {
  data = db.querySync("select 'ꜨꜢ' as UNICODETEXT FROM SYSIBM.SYSDUMMY1");
}
catch (e) {
  console.log(e); 
}

db.closeSync();
console.log(data);
assert.deepEqual(data, [{ UNICODETEXT: 'ꜨꜢ' }]);


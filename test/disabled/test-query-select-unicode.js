var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);

db.query("select '☯ąčęėįšųūž☎áäàéêèóöòüßÄÖÜ€ шчябы Ⅲ ❤' as UNICODETEXT FROM SYSIBM.SYSDUMMY1", function (err, data) {
  db.closeSync();
  console.log(data);
  assert.equal(err, null);
  assert.deepEqual(data, [{ UNICODETEXT: '☯ąčęėįšųūž☎áäàéêèóöòüßÄÖÜ€ шчябы Ⅲ ❤' }]);
});


var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  db.query("select '☯ąčęėįšųūž☎áäàéêèóöòüßÄÖÜ€ шчябы Ⅲ ❤' as UNICODETEXT", function (err, data) {
    db.close(function () {
      console.log(data);
      assert.equal(err, null);
      assert.deepEqual(data, [{ UNICODETEXT: '☯ąčęėįšųūž☎áäàéêèóöòüßÄÖÜ€ шчябы Ⅲ ❤' }]);
    });
  });
});

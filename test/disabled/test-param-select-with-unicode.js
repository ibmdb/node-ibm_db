var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  db.query("select ? as UNICODETEXT", ['ף צ ץ ק ר ש תכ ך ל מ ם נ ן ס ע פ 電电電買买買開开開東东東車车車'], function (err, data) {
    db.close(function () {
      console.log(data);
      assert.equal(err, null);
      assert.deepEqual(data, [{ UNICODETEXT: 'ף צ ץ ק ר ש תכ ך ל מ ם נ ן ס ע פ 電电電買买買開开開東东東車车車' }]);
    });
  });
});

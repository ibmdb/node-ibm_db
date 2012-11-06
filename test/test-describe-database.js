var common = require("./common")
  , odbc = require("../odbc.js")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  
  common.dropTables(db, function () {
    common.createTables(db, function () {
      db.describe({
        database : 'MAIN'
      }, function (err, data) {
        db.close(function () {
          assert.deepEqual(data, [{
            TABLE_QUALIFIER: null,
            TABLE_OWNER: null,
            TABLE_NAME: 'TEST',
            TABLE_TYPE: 'TABLE',
            REMARKS: null 
          }]);
        });
      });
    });
  });
});
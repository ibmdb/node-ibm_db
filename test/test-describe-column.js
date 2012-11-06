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
        database : 'MAIN',
        table : 'TEST',
        column : 'COLDATETIME'
      }, function (err, data) {
        db.close(function () {
          assert.deepEqual(data, [{ 
            TABLE_QUALIFIER: '',
            TABLE_OWNER: '',
            TABLE_NAME: 'TEST',
            COLUMN_NAME: 'COLDATETIME',
            DATA_TYPE: 11,
            TYPE_NAME: 'DATETIME',
            PRECISION: 3,
            LENGTH: 32,
            SCALE: '10',
            RADIX: '0',
            NULLABLE: 1,
            REMARKS: null,
            COLUMN_DEF: 'NULL',
            SQL_DATA_TYPE: '11',
            SQL_DATETIME_SUB: null,
            CHAR_OCTET_LENGTH: 16384,
            ORDINAL_POSITION: 2,
            IS_NULLABLE: 'YES'
          }]);
        });
      });
    });
  });
});

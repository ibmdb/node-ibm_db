var ibmdb = require("../")
    , common = require("./common")
    , assert = require("assert")
    , cn = common.connectionString;

var createSQL =  "create table issue253 (name int)";
var selectSQL =  "select * from issue253 WHERE CURRENT DATE = '2017-04-17 0'";
var dropSQL   =  "drop table issue253";

ibmdb.open(cn, function (err,conn) {
  if (err) console.log(err);
  assert.equal(err, null);
  
  conn.querySync(createSQL);
  
  conn.query(selectSQL, function (err, data) {
      conn.querySync(dropSQL);
      conn.closeSync();
     
      if (err) {
          assert.equal(err.message, "[IBM][CLI Driver][DB2/NT64] SQL0180N  The syntax of the string representation of a datetime value is incorrect.  SQLSTATE=22007\r\n");
      }
  });
});
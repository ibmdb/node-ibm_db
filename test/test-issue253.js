var ibmdb = require("../")
    , common = require("./common")
    , assert = require("assert")
    , cn = common.connectionString;

const os = require("os");

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
      var errorFound = false;
     
      if (err.message) {
        if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
          // zOS Db2 returns SQLCode -181 and SQLState 22007 for
          // THE STRING REPRESENTATION OF A DATETIME VALUE IS NOT
          // A VALID DATETIME VALUE
          if (os.type() === "OS/390") {
            var errorFound = err.message.includes("SQLCODE = -181");
          } else {
            var errorFound = err.message.includes("SQL0181N");
          }
        } else {
          var errorFound = err.message.includes("SQL0180N");
        }
      }
      assert.equal(errorFound, true);
  });
});

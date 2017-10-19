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
        var workingCondition1 = err.message.includes("SQL0180N");
        var workingCondition2 = err.message.includes("SQLSTATE=22007");
        if(workingCondition1 && workingCondition2){
          console.log("fix is working fine.");
        } else {
          console.log("fix is not working fine.");
        }
      }
  });
});

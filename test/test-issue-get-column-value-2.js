var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  , util = require('util')
  , count = 0
  ;
 
var getSchema = function () {
  var db = new odbc.Database();
 
  console.log(util.format('Count %s, time %s', count, new Date()));
  //console.log(db);
 
  db.open(common.connectionString, function(err) {
    if (err) {
      console.error("connection error: ", err.message);
      db.close(function(){});
      return;
    }
 
    db.describe({database: 'main', schema: 'RETAIL', table: common.tableName }, function (err, rows) {
//    db.query("select * from " + common.tableName, function (err, rows) {
      if (err) {
        console.error("describe error: ", err.message);
        db.close(function(){});
        return;
      }
      
      db.close(function() {
        console.log("Connection Closed");
        db = null;
        count += 1;
        if (count < 100) {
          setImmediate(getSchema);
        }
        else {
          process.exit(0);
        }
      });
    });
  });
};
 
getSchema();
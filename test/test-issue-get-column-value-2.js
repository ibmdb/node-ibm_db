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
 
  db.open(common.connectionString, function(err) {
    if (err) {
      console.error("connection error: ", err.message);
      db.close(function(){});
      assert(err.message, null);
    }
 
    db.describe({database: 'SAMPLE', schema: 'NEWTON', table: "PRODUCT" }, function (err, rows) {
      if (err) {
        console.error("describe error: ", err.message);
        db.close(function(){});
        return;
      }
      console.log(rows);
      
      db.close(function() {
        console.log("Connection Closed");
        db = null;
        count += 1;
        if (count < 10) {
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

var common = require("./common")
  , odbc = require("../odbc.js")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  common.createTables(db, function (err, data, morefollowing) {
    console.log(arguments);
    db.close(function () { 
      
    });
  });
});

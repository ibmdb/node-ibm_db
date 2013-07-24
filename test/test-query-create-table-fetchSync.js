var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);

db.queryResult("create table " + common.tableName + " (COLINT INTEGER, COLDATETIME DATETIME, COLTEXT TEXT)", function (err, result) {
  console.log(arguments);
  
  try {
    //this should throw because there was no result to be had?
    var data = result.fetchAllSync();
    console.log(data);
  }
  catch (e) {
    console.log(e);
  }
  
  db.closeSync();
});


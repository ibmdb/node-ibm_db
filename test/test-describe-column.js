var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);

console.log("connected");

common.dropTables(db, function (err) {
  if (err) console.log(err.message);
  
  console.log("tables dropped");
  
  common.createTables(db, function (err) {
    if (err) console.log(err.message);
    
    console.log("tables created");
    
    db.describe({
      database : common.databaseName,
      table : common.tableName,
      column : 'COLDATETIME'
    }, function (err, data) {
      if (err) console.log(err.message);
      
      console.log(data);
      assert.ok(data.length, "No records returned when attempting to describe the column COLDATETIME");
      test2();
    });
  });
});

async function test2() {
    let result = "";
    result = await db.columns(common.databaseName, '%', common.tableName, 'COLDATETIME');
    console.log("db.columns result = ", result);
    result = await db.columns(common.databaseName, common.tableName, 'COLDATETIME').catch((e) => { console.log(e); });
    result = await db.describe({
      database : common.databaseName,
      table : common.tableName,
      column : 'COLDATETIME'
    });
    console.log("db.describe result = ", result);
    result = await db.describe({
      table : common.tableName,
      column : 'COLDATETIME'
    }).catch((e) => { console.log(e); });
    console.log("db.describe result = ", result);
    db.closeSync();
}

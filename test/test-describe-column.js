var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

var expectColumnsArgsRejection = false;

process.on('unhandledRejection', function (reason) {
  if (expectColumnsArgsRejection && reason && reason.error === "[node-ibm_db] Missing Arguments" &&
      /catalog, schema, table and column/.test(reason.message)) {
    console.log(reason);
    return;
  }

  console.error(reason);
  process.exitCode = 1;
});

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
      test2().catch(function (error) {
        console.error(error);
        process.exitCode = 1;
        try {
          db.closeSync();
        } catch (e) {}
      });
    });
  });
});

async function test2() {
    let result = "";
    let negativeDb = null;

    result = await db.columns(common.databaseName, '%', common.tableName, 'COLDATETIME');
    console.log("db.columns result = ", result);
    assert.ok(result.length, "db.columns() did not return column metadata for COLDATETIME");

    expectColumnsArgsRejection = true;
    try {
      negativeDb = new odbc.Database();
      negativeDb.openSync(common.connectionString);
      result = await negativeDb.columns(common.databaseName, common.tableName, 'COLDATETIME');
      assert.fail("Expected invalid db.columns() call to throw");
    } catch (e) {
      console.log(e);
      assert.ok(/Argument 3 must be a string or null/.test(e.message));
    } finally {
      await new Promise(function (resolve) {
        setImmediate(resolve);
      });
      expectColumnsArgsRejection = false;
      if (negativeDb) {
        try {
          negativeDb.closeSync();
        } catch (e) {}
      }
    }

    result = await db.describe({
      database : common.databaseName,
      table : common.tableName,
      column : 'COLDATETIME'
    });
    console.log("db.describe result = ", result);
    assert.ok(result.length, "db.describe() did not return column metadata for COLDATETIME");

    db.closeSync();
}

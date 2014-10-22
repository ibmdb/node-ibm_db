var common = require("./common")
  , odbc = require("../")
  , db = new odbc.ODBC()
  , assert = require("assert")
  , exitCode = 0
  ;

/*
 * The purpose of this test is to test binding an array and then
 * changing the values of the array before an execute[Sync]
 * call and have the new array values be used.
 */
  
db.createConnection(function (err, conn) {
	
	console.log(common.connectionString);
	conn.openSync(common.connectionString);
  
  conn.createStatement(function (err, stmt) {
    var r, result, caughtError;
    
    var a = ['hello', 'world'];
    
    stmt.prepareSync('select ? as col1, ? as col2');
    
    stmt.bindSync(a);
    
    result = stmt.executeSync();
    
    console.log(result.fetchAllSync());
    result.closeSync();
    
    a[0] = 'goodbye';
    a[1] = 'steven';
    
    result = stmt.executeSync();
    
    r = result.fetchAllSync();
    
    try {
      assert.deepEqual(r, [ { col1: 'goodbye', col2: 'steven' } ]);
    }
    catch (e) {
      console.log(e);
      exitCode = 1;
    }
    
    conn.closeSync();
    
    if (exitCode) {
      console.log("failed");
    }
    else {
      console.log("success");
    }
    
    process.exit(exitCode);
  });
});

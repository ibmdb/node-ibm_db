var common = require("./common")
	, odbc = require("../")
	, assert = require("assert")
	, openCallback = 0
	, closeCallback = 0
	, openCount = 10
	, connections = []
	, errorCount = 0;
	;

for (var x = 0; x < openCount; x++ ) {
  var db = new odbc.Database();
  connections.push(db);

  try {
    var connected = db.openSync(common.connectionString);
    if(connected === true) {
      console.log("Connection ", x + 1, " opened.");
    }
    else {
      console.log("Connection ", x + 1, " error = ", connected);
    }
  }
  catch (e) {
    console.log(common.connectionString);
    console.log(e);
    errorCount += 1;
    break;
  }
}

connections.forEach(function (db) {
  db.closeSync();
  console.log("Closed connection ", openCount--);
});

/* Test case to test the fix of issue #557 */
odbc.open(common.connectionString, function(err, conn) {
    conn.query("select * from mytab; select 1 from sysibm.sysdummy1;",
      function(err, result) {
        if(err) {
          console.log(err);
        }
        else {
          conn.closeSync();
        }
        console.log("result = ", result);
        assert(err['message'].search("SQL0204N|42704") > 0);
        console.log('Done');
        process.exit(errorCount);
    });
});

var common = require("./common")
	, odbc = require("../")
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
    db.openSync(common.connectionString);
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
});

console.log('Done');
process.exit(errorCount);
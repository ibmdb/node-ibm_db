var common = require("./common")
	, odbc = require("../")
	, openCallback = 0
	, closeCallback = 0
	, openCount = 1000
	, connections = []
	;

for (var x = 0; x < openCount; x++ ) {
  var db = new odbc.Database();
  connections.push(db);

  db.openSync(common.connectionString);
}

connections.forEach(function (db) {
  db.closeSync();
});

console.log('Done');

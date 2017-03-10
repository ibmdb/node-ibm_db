var common = require("./common")
	, odbc = require("../")
    , assert = require("assert")
	, openCallback = 0
	, closeCallback = 0
	, openCount = 10
	, connections = []
	;

for (var x = 0; x < openCount; x++ ) {
	(function () {
		var db = new odbc.Database();
		connections.push(db);

		db.open(common.connectionString, function(err) {
            assert.equal(err, null);
			openCallback += 1;
			maybeClose();
		});
	})();
}

function maybeClose() {

	if (openCount == openCallback) {
		doClose();
	}
}


function doClose() {
	connections.forEach(function (db) {
		db.close(function () {
			closeCallback += 1;
			
			maybeFinish();
		});
	});
}

function maybeFinish() {
	if (openCount == closeCallback) {
		console.log('Done');
	}
}

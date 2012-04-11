var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();

db.open(common.connectionString, function(err)
{
	if (err) {
		console.error(err.message);
		return;
	}

    require('http').createServer(function (req, res) {
		if (req.url == "/close") {
			db.close(function () {});
			db = null;
			res.end();
			return false;
		}

		var query = "select 1234 union select 5345";

		db.query(query, function(err, rows, moreResultSets)
		{
			if (err) {
				console.error(err.message);
			}

			err = null;
			rows = null;
			moreResultSets = null;
			res.end();
		});
	}).listen(8082, "127.0.0.1");
});

process.on('uncaughtException', function (err) {
	console.error('uncaughtException:' + err);
	console.error(err.stack);
});

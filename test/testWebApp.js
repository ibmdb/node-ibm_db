var common = require("./common")
	, odbc = require("../")
	, db = new odbc.Database();

db.open(common.connectionString, function(err)
{
	if (err) {
		console.error(err.message);
		return;
	}

    require('http').createServer(function (req, res) {
		if (req.url == "/close") {
			db.closeSync();
			db = null;
			res.end();
			process.exit(1);
		}

		var query = "select 1234 from sysibm.sysdummy1";

		db && db.query(query, function(err, rows)
		{
			if (err) {
				console.error(err.message);
			}
			console.log(rows);
            res.writeHead(200, {'Content-Type': 'text/plain'});
            res.write(JSON.stringify(rows));
			res.end();
		});
	}).listen(8082, "127.0.0.1");
});

process.on('uncaughtException', function (err) {
	console.error('uncaughtException:' + err);
	console.error(err.stack);
});
console.log("App is listening on 127.0.0.1:8082");

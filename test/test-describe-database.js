var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();

db.open(common.connectionString, function(err)
{
	db.describe({
		database : 'main'
	}, function (err, data) {
		if (err) {
			console.error(err);
			process.exit(1);
		}
		
		console.error(data);
	});
});

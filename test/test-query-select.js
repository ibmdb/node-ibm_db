var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();

db.open(common.connectionString, function(err)
{
	db.query('select \'a\' as test, datetime(\'now\') as nowish, * from test', function (err, data) {
		if (err) {
			console.error(err);
			process.exit(1);
		}
		
		console.error(data);
		db.close(function () {});
	});
});

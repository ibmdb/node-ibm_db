var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();

db.query("select * from test", function (err, rs, moreResultSets) {
	console.error(arguments);
});

db.open(common.connectionString, function(err)
{
	console.error('db.open callback');
	
	db.close(function () {
		console.error('db.close callback');
		
		db.query("select * from test", function (err, rs, moreResultSets) {
			console.error('db.query callback');
			console.error(arguments);
		});
	});
});

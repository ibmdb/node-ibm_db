var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();

db.open(common.connectionString, function(err)
{
	console.error('db.open callback');
	
	db.close(function () {
		console.error('db.close callback');
	});
});

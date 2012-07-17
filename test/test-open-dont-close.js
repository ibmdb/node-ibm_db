var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();

db.open(common.connectionString, function(err)
{
	console.error('db.open callback');
	console.error('node should just sit and wait');	
});

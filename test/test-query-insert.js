var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();

db.open(common.connectionString, function(err)
{
	db.query("insert into test (col1) values ('sandwich')", function (err, data) {
		if (err) {
			console.error(err);
			process.exit(1);
		}
		
		console.error(data);
	});
	
	db.query("insert into test (col1) values ('fish')", function (err, data) {
		if (err) {
			console.error(err);
			process.exit(1);
		}
		
		console.error(data);
	});
	
	db.query("insert into test (col1) values ('scarf')", function (err, data) {
		if (err) {
			console.error(err);
			process.exit(1);
		}
		
		console.error(data);
	});
});

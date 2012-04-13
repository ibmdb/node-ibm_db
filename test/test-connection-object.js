var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();

db.open(common.connectionObject, function(err){ 
	if (err) {
		console.error(err);
		process.exit(1);
	}
	
	console.error("Open success");
	
	db.close(function () {
		console.error("Close Success");
	});
});

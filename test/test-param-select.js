var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();


db.open(common.connectionString, function (err) {
	if (err) {
		console.error(err);
		return;
	}

	db.query("select * from test where col1 = ? "
		, ["fish"]
		, function (err, data, more) {
			if (err) {
				console.error(err);
				process.exit(1);
			}
			
			console.error(data);
		}
	);
});

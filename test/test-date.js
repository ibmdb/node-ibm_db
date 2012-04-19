var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database();

db.open(common.connectionString, function(err)
{
	if (err) {
		console.error(err);
		process.exit(1);
	}

	createTable();
});

function createTable() {
	db.query("CREATE TABLE IF NOT EXISTS date_test ( dt1 datetime )", function (err) {
		if (err) {
			console.error(err);
			return cleanup(1);
		}

		insertData();
	});
}

function dropTable(exitCode) {
	db.query("DROP TABLE IF EXISTS date_test", function (err) {
		if (err) {
			console.error(err);
			return cleanup(1);
		}
		
		cleanup(exitCode || 0);
	});
}


function insertData() {
	db.query("INSERT INTO date_test (dt1) values ('2012-04-17')", function (err) {
		if (err) {
			console.error(err);
			return dropTable(1);
		}
		
		selectData();
	});
}

function selectData() {
	db.query("SELECT * FROM date_test", function (err, data) {
		if (err) {
			console.error(err);
			return cleanup(1);
		}

		//test selected data
		if (data[0].dt1 instanceof Date) {
			dropTable(0);
		}
		else {
			console.error("dt1 is instance of: %s", data[0].dt1.constructor.name);
			dropTable(1);
		}
	});
}

function cleanup(exitCode) {
	db.close(function (err) {
		if (exitCode == 0) {
			console.error("success");
		}
		else {
			console.error("failure");
		}

		if (err) {
			process.error(err);
			process.exit(1);
		}
		
		process.exit(exitCode);
	});
}

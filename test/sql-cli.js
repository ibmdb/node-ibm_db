var common = require("./common")
	, odbc = require("../")
	, db = new odbc.Database();

db.open(common.connectionString, function(err)
{
	if (err) {
		console.log(err);
		process.exit(1);
	}	
	
	process.stdout.write('> ');
	process.stdin.resume();
	
	process.stdin.on('data',function (strInput) {
		db.query(strInput.toString(), function (err, rs, moreResultSets) {
			if (err) {
				console.log(err);
				console.log(rs);
			}
			else {
				console.log(rs);
			}
			
			process.stdout.write('> ');
		});
	});
});

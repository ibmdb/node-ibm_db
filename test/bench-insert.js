var common = require("./common")
	, odbc = require("../")
	, db = new odbc.Database();

db.open(common.connectionString, function(err){ 
	if (err) {
		console.log(err);
		process.exit(1);
	}
	dropTable();	
	createTable();
});

function createTable() {
	db.query("create table bench_insert (str varchar(50))", function (err) {
		if (err) {
			console.log(err);
			return finish();
		}
		
		return insertData();
	});
}

function dropTable() {
    try { 
        db.querySync("drop table bench_insert")
    }catch(e){
    //    console.log(e);
    // do nothing if the table doesn't exist
    }
}

function insertData() {
	var count = 0
		, iterations = 100
		, time = new Date().getTime();
	
	for (var x = 0; x < iterations; x++) {
		db.query("insert into bench_insert (str) values ('testing')", cb);
		
	}

	function cb (err) {
		if (err) {
			console.log(err);
			return finish();
		}
		
		if (++count == iterations) {
			var elapsed = (new Date().getTime() - time)/1000;
			process.stdout.write("(" + iterations + " records inserted in " + elapsed + " seconds, " + (iterations/elapsed).toFixed(4) + " records/sec)");
			return dropTable();
		}
	}
}

function finish() {
	db.close(function () {
		console.log("connection closed");
	});
}

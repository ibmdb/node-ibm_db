var common = require("./common")
	, odbc = require("../")
	, db = new odbc.Database();

var insertString = "";
var batchSize = 100;

db.open(common.connectionString, function(err){ 
	if (err) {
		console.log(err);
		process.exit(1);
	}
        createInsertString(batchSize);
	dropTable();	
	createTable();
});

function createInsertString(batchSize) {

    insertString = "insert into bench_insert (str) values ('testing')";
    for (var i = 0; i < batchSize; i++) {
        insertString += ", ('testing')";
    }
}
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
		//, iterations = 10000
		, time = new Date().getTime();
	
	for (var x = 0; x < iterations; x++) {
		db.query(insertString, cb);
		
	}

	function cb (err) {
		if (err) {
			console.log(err);
			return finish();
		}
		
		if (++count == iterations) {
			var elapsed = (new Date().getTime() - time)/1000;
            process.stdout.write("(" + batchSize * iterations + " records inserted in " + elapsed + " seconds, " +
                                 (batchSize * iterations/elapsed).toFixed(4) + " records/sec)");
			return dropTable();
		}
	}
}

function finish() {
	db.close(function () {
		console.log("connection closed");
	});
}

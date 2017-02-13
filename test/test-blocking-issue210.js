var common = require("./common")
	, ibmdb = require("../")
	, pool = new ibmdb.Pool()
	, connectionString = common.connectionString
    , assert = require("assert")
	, connections = []
	, connectCount = 2;

var startTime1, startTime2, totalTime1, totalTime2, dropTable = 0;
var elapsedTime = ibmdb.getElapsedTime;
var ret = pool.init(connectCount, connectionString); 

//moment().format("YYYY-MM-DD HH:mm:ss.SSS"));
console.log(elapsedTime(), "Started pool.open, populate a table mtab1 of 100K rows.");
pool.open(connectionString, function( err, conn) {
    try { conn.querySync("drop table mtab1");
          conn.querySync("drop table mtab2"); } catch(e) {};
    conn.querySync("create table mtab1(c1 int, c2 varchar(20))");    
    conn.querySync("create table mtab2(c1 int, c2 varchar(20))");    
    conn.querySync("Insert into mtab1 values (1, 'bimal'),(2,'kumar'),(3, 'jha'), (4, 'kamal'), (5, 'ibm')");
    conn.querySync("Insert into mtab1 values (1, 'bimal'),(2,'kumar'),(3, 'jha'), (4, 'kamal'), (5, 'ibm')");
    for(var i = 0; i < 8 ; i++) {
      conn.querySync("insert into mtab2 (select * from mtab1)");
      conn.querySync("insert into mtab1 (select * from mtab2)");
      }
    conn.querySync("drop table mtab2");
    console.log(elapsedTime(), "Inserted rows in mtab1 = ", conn.querySync("select count(*) from mtab1")[0]['1']);
    conn.close(function(err){});
});

ibmdb.debug(true);
var q1time, q2time;
console.log(elapsedTime(), "Opening connection #1");
pool.open(connectionString, function (err, connection) {
        console.log(elapsedTime(), "Connection 1 opened. Start execution of Query1");
        startTime1 = new Date();
        connection.query("select * from mtab1", function(err, data) {
                if(err) console.log(err);
                totalTime = (new Date() - startTime1)/1000;
                q1time = parseInt(totalTime%60);
                console.log(elapsedTime(), "Total execution time for Query1 = ", q1time, "sec."); 
                dropTable++;
                if(dropTable == 2) {
                  connection.querySync("drop table mtab1");
                  ibmdb.debug(false);
                  pool.close();
                  assert.equal(q1time > 5, false);
                }
        });
});
pool.open(connectionString, function (err, connection) {
        console.log(elapsedTime(), "Connection 2 opened. Start execution of Query2");
        startTime1 = new Date();
        connection.query("select c1, c2 from mtab1", function(err, data) {
                if(err) console.log(err);
                totalTime = (new Date() - startTime1)/1000;
                q2time = parseInt(totalTime%60);
                console.log(elapsedTime(), "Total execution time for Query2 = ", q2time, "sec."); 
                dropTable++;
                if(dropTable == 2) {
                  connection.querySync("drop table mtab1");
                  ibmdb.debug(false);
                  pool.close();
                  assert.equal(q2time > 5, false);
                }
        });
});
console.log(elapsedTime(), " **** All lines of test file executed, waiting for callback functions to finish. ");


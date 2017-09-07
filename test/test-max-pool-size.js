var common = require("./common")
	, ibmdb = require("../")
	, pool = new ibmdb.Pool()
	, connectionString = common.connectionString
    , assert = require("assert")
	, connectCount = 12;

pool.setMaxPoolSize(5); // Max no of active connections.
//pool.setConnectTimeout(8); // No timeout. connection request will remain in queue.

var elapsedTime = ibmdb.getElapsedTime;
var ret = pool.init(3, connectionString); 
if(typeof ret === 'object') assert.equal(ret.message, undefined);

//moment().format("YYYY-MM-DD HH:mm:ss.SSS"));
console.log(elapsedTime(), "Started pool.open, populate a table MTAB1 of 130 rows.");
pool.open(connectionString, function( err, conn) {
    try { conn.querySync("drop table mtab1");
          conn.querySync("drop table mtab2"); } catch(e) {};
    conn.querySync("create table mtab1(c1 varchar(30), c2 varchar(20))");
    conn.querySync("create table mtab2(c1 varchar(30), c2 varchar(20))");
    conn.querySync("Insert into mtab1 values ('1', 'bimal'),('2','kumar'),('3', 'jha'), ('4', 'kamal'), ('5', 'ibm')");
    conn.querySync("Insert into mtab1 values ('1', 'bimal'),('2','kumar'),('3', 'jha'), ('4', 'kamal'), ('5', 'ibm')");
    for(var i = 0; i < 3 ; i++) {  // 4 is 340 rows and 6 is 2330 rows
      conn.querySync("insert into mtab2 (select * from mtab1)");
      conn.querySync("insert into mtab1 (select * from mtab2)");
      }
    conn.querySync("drop table mtab2");
    console.log(elapsedTime(), "Inserted rows in mtab1 = ", conn.querySync("select count(*) from mtab1")[0]['1']);
    conn.close(function(err){});
});

ibmdb.debug(true);
openConnectionsUsingPool();

function openConnectionsUsingPool() {
  for (var x = 1; x <= connectCount; x++) {
   (function (connectionIndex) {
      console.log("Opening connection #", connectionIndex);
      pool.open(connectionString, function (err, connection) {
        if (err) {
          console.error("error for connection %d: %s",
              connectionIndex, err.message);
          if (connectionIndex == connectCount) {
            console.log("Going to close connections.. \n");
            closeConnections();
          }
          return false;
        }

        console.log(elapsedTime(), "Connection " + connectionIndex + " opened. Start execution of Query" + connectionIndex + "\n");
        var startTime = new Date();
        connection.query("select c1, c2 from mtab1", function(err, data) {
            if(err) console.log(err);
            var totalTime = (new Date() - startTime)/1000;
            //var qtime = parseInt(totalTime%60);
            console.log(elapsedTime(), "Total execution time for Query"+connectionIndex+" = ", totalTime, "sec."); 

            if (connectionIndex == connectCount) {
              console.log("Going to close connections.. \n");
              connection.querySync("drop table mtab1");
              closeConnections();
            }
            else
            {
              connection.close();
            }
        });
      });
    })(x);
  }
}

function closeConnections () {
  pool.close(function () {
    console.error("pool closed");
  });
}


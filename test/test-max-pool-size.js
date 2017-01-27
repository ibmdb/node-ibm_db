var common = require("./common")
	, ibmdb = require("../")
	, pool = new ibmdb.Pool()
	, connectionString = common.connectionString
    , assert = require("assert")
	, connections = []
	, connectCount = 6;

ibmdb.debug(true);
pool.setMaxPoolSize(9); // Max no of active connections.
pool.setConnectTimeout(10); // Connection timeout in seconds.
//var connectionString = "DATABASE=STLEC1;HOSTNAME=INEC007.vmec.svl.ibm.com;PORT=446;PROTOCOL=TCPIP;UID=newton;PWD=iNn0v8on;";
var ret = pool.init(3, connectionString); // Initialize pool with n no of connections.
assert.deepEqual(true, ret);

pool.open(connectionString, function( err, conn) {
    try { conn.querySync("drop table mtab1");
          conn.querySync("drop table mtab2"); } catch(e) {};
    conn.querySync("create table mtab1(c1 int, c2 varchar(20))");    
    conn.querySync("create table mtab2(c1 int, c2 varchar(20))");    
    conn.querySync("Insert into mtab1 values (1, 'bimal'),(2,'kumar'),(3, 'jha'), (4, 'kamal'), (5, 'ibm')");
    conn.querySync("Insert into mtab1 values (1, 'bimal'),(2,'kumar'),(3, 'jha'), (4, 'kamal'), (5, 'ibm')");
    for(var i = 0; i < 10 ; i++) {
      conn.querySync("insert into mtab2 (select * from mtab1)");
      conn.querySync("insert into mtab1 (select * from mtab2)");
      }
    console.log("Inserted rows in mtab1 = " , conn.querySync("select count(*) from mtab1"));
    conn.close(function(err){});
    });
openConnectionsUsingPool(connections);

function openConnectionsUsingPool(connections) {
  for (var x = 1; x <= connectCount; x++) {
   (function (connectionIndex) {
      console.error("Opening connection #", connectionIndex);
      pool.open(connectionString, function (err, connection) {
        if (err) {
          console.error("error for connection %d: %s", 
              connectionIndex, err.message);
          if (connectionIndex == connectCount) {
            console.log("Going to close connections.. \n");
            closeConnections(connections);
          }
          return false;
        }

        connections.push(connection);
        console.log("connection " + connectionIndex + " opened.");
        connection.querySync("select * from mtab1");

        if (0) { //connectionIndex == connectCount) {
          console.log("Going to close connections.. \n");
          closeConnections(connections);
        }
      });
    })(x);
  }
  console.log("For loop is done. ", connectCount, " connections opened.");
}

console.log("going to open a new pooled connection #7 .... ");
pool.open(connectionString, function( err, conn) {
    console.log(" connection #7 opened. run query now. ====>");
    conn.query("select * from mtab2", function(err, result) {
        console.log("Done!");
    });
});
function closeConnections (connections) {
  pool.close(function () {
    console.error("pool closed");
  });
}



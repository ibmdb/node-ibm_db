var common = require("./common")
	, ibmdb = require("../")
	, pool = new ibmdb.Pool()
    , assert = require("assert")
	, connectionString = common.connectionString
	, connections = []
	, connectCount = 10;

openConnectionsUsingPool(connections);

function openConnectionsUsingPool(connections) 
{
  console.log("\n Test 1:\n", "=======");
  console.log("Open", connectCount, "connections from pool.\n");
  for (var x = 0; x <= connectCount; x++) 
  {
    (function (connectionIndex) {
      console.error("Opening connection #", connectionIndex + 1);
      pool.open(connectionString, function (err, connection) {
        if (err) {
          console.error("error: ", err.message);
          assert.equal(err.message, null);
        }

        connections.push(connection);

        if (connectionIndex == connectCount) {
          ibmdb.debug(true);
          closeConnections(connections);
        }
      });
    })(x);
  }
}

function closeConnections (connections) 
{
  console.log("\n Test 2:\n", "=======");
  console.log("Close all connections using pool.close().");
  pool.close(function () {
    console.error("pool closed.");
    testMultipleCloseConnections();
  });
}

function testMultipleCloseConnections()
{
    console.log("\n Test 3:\n", "=======");
    console.log("Test Multiple Close Connections.\n");
    const pool = new ibmdb.Pool();
    pool.open(connectionString, (err, conn) => {
        if (err) { console.log("err", err); return; }
        console.log("Connection opened successfully.");
        conn.close(function (err2) {
            if (err2) { console.log("err2", err2); return; }
            console.log("Close 1 Success!");
            // We should still able to exec query as conn is returned to pool,
            // but connection to server is terminated, it is still active.
            console.log("Data = ", conn.querySync("select 1 as c1 from sysibm.sysdummy1"));
            conn.close(function (err3) {
                if (err3) { console.log("err3", err3); return; }
                console.log("Close 2 Success!");
                console.log("Data = ", conn.querySync("select 2 as c2 from sysibm.sysdummy1"));
                // Get the connection again from pool to actually close it.
                pool.open(connectionString, (err, conn) => {
                  conn.realClose(function (err) {
                    //Now connection to server is terminated.
                    console.log("Trying to execute a query on closed connection...");
                    try {conn.querySync("select 3 as c3 from sysibm.sysdummy1");
                    } catch(e) {console.log("Error =", e);}
                    console.log("Going to Close the Pool.");
                    pool.close(function (err4) {
                      if (err4) { console.log("err4", err4); return; }
                      console.log("Pool Closed successfully. Test3 PASSED!");
                      testSyncConnections();
                    });
                  });
                });
            });
        });
    });
}

function testSyncConnections()
{
    console.log("\n Test 4:\n", "=======");
    console.log("Testing synchronous version of connection pool APIs.\n");

    const pool2 = new ibmdb.Pool();
    try {
        var conn = pool2.openSync(connectionString);
    } catch(error) {
        console.log("error = ", error); return;
    }
    console.log("Connection opened successfully.");
    console.log("Data = ", conn.querySync("select 1 as c1 from sysibm.sysdummy1"));
    var err = conn.closeSync();
    if (err) { console.log("Error in first closeSync,", err); return; }
    console.log("CloseSync 1 Success!");
    console.log("Data = ", conn.querySync("select 2 as c2 from sysibm.sysdummy1"));
    err = conn.closeSync();
    if (err) { console.log("Error in second closeSync,", err); return; }
    console.log("CloseSync 2 Success!");
    try {
      // Get the connection again from pool to actually close it.
      conn = pool2.openSync(connectionString);
      err = conn.realCloseSync();
      if (err != true) { console.log("Error in real closeSync,", err); }
      console.log("RealCloseSync Success!");
      console.log("Trying to execute a query on closed connection...");
      conn.querySync("select 3 as c3 from sysibm.sysdummy1");
    } catch(error) {
        console.log("Error = ", error);
    }
    console.log("Going to Close the Pool.");
    err = pool2.closeSync();
    if (err) { console.log("err", err); return; }
    console.log("Pool Closed successfully. Test4 PASSED!");
}


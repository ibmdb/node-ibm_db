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
  for (var x = 0; x <= connectCount; x++) 
  {
    (function (connectionIndex) {
      console.error("Opening connection #", connectionIndex);
      pool.open(connectionString, function (err, connection) {
        //console.error("Opened connection #", connectionIndex);
        if (err) {
          console.error("error: ", err.message);
          assert.equal(err.message, null);
        }

        connections.push(connection);

        if (connectionIndex == connectCount) {
          closeConnections(connections);
        }
      });
    })(x);
  }
}

function closeConnections (connections) 
{
  pool.close(function () {
    console.error("pool closed. calling testMultipleCloseConnections().");
    testMultipleCloseConnections();
  });
}

function testMultipleCloseConnections()
{
    const pool = new ibmdb.Pool();
    pool.open(connectionString, (err, conn) => {
        if (err) { console.log("err", err); return; }
        console.log("Connection opened successfully.");
        conn.close(function (err2) {
            if (err2) { console.log("err2", err2); return; }
            console.log("Close 1 Success!");
            conn.close(function (err3) {
                if (err3) { console.log("err3", err3); return; }
                console.log("Close 2 Success!");
                console.log("Going to Close the Pool.");
                pool.close(function (err4) { 
                    if (err4) { console.log("err4", err4); return; }
                    console.log("Pool Closed successfully. Test PASSED!");
                });
            });
        });
    });
}


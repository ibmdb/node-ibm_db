var common = require("./common")
	, ibmdb = require("../")
	, pool = new ibmdb.Pool()
	, connectionString = common.connectionString
	, connections = []
	, connectCount = 12;

//ibmdb.debug(true);
pool.setMaxPoolSize(8); // Max no of active connections.
pool.setConnectTimeout(5); // Connection timeout in seconds.
pool.init(10, connectionString); // Initialize pool with n no of connections.

openConnectionsUsingPool(connections);

function openConnectionsUsingPool(connections) {
  for (var x = 0; x < connectCount; x++) {
   (function (connectionIndex) {
      console.error("Opening connection #", connectionIndex);
      pool.open(connectionString, function (err, connection) {
        if (err) {
          console.error("error for connection %d: %s", 
              connectionIndex, err.message);
          return false;
        }

        connections.push(connection);
        console.log("connection " + connectionIndex + " opened.\n");

        if (connectionIndex == connectCount) {
          console.log("Going to close connections.. \n");
          //console.log(connections);
          closeConnections(connections);
        }
      });
    })(x);
  }
}

function closeConnections (connections) {
  pool.close(function () {
    console.error("pool closed");
  });
}



var common = require("./common")
	, odbc = require("../")
	, pool = new odbc.Pool()
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
    console.error("pool closed");
  });
}


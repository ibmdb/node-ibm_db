var common = require("./common")
	, odbc = require("../odbc.js")
	, db = new odbc.Database()
	, connectionString = common.connectionString
	, connections = []
	, connectCount = 500;

openConnectionsUsingPool(connections);

function openConnectionsUsingPool(connections) {
	for (var x = 0; x <= connectCount; x++) {
		
		(function (connectionIndex) {
			//setTimeout(function () {
			//console.error("Opening connection #", connectionIndex);
			
			pool.open(connectionString, function (err, connection) {
				//console.error("Opened connection #", connectionIndex);
				
				if (err) {
					console.error("error: ", err.message);
					return false;
				}
				
				connections.push(connection);
				
				if (connectionIndex == connectCount) {
					//closeConnections(connections);
				}
			});
			
			//}, x * 50);
		})(x);
	}
}

function openConnectionsUsingDB(connections) {
	for (var x = 0; x <= connectCount; x++) {
		
		(function (connectionIndex) {
			//console.error("Opening connection #", connectionIndex);
			var db = new Database();
			
			db.open(connectionString, function (err, connection) {
				//console.error("Opened connection #", connectionIndex);
				
				if (err) {
					console.error("error: ", err.message);
					return false;
				}
				
				connections.push(db);
				//connections.push(connection);
				
				if (connectionIndex == connectCount) {
					closeConnections(connections);
				}
			});
		})(x);
	}
}

function closeConnections (connections) {
	connections.forEach(function (connection, idx) {
		//console.error("Closing connection #", idx);
		connection.close(function () {
			//console.error("Closed connection #", idx);
		});
	});
}
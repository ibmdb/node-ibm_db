/*require the ibm_db module*/
var common = require("./common")
  , assert = require("assert")
  , ibmdb = require("../");

var connString = 'DRIVER={DB2 ODBC Driver};DATABASE=SAMPLE;UID=db2admin;PWD=db2admin;HOSTNAME=localhost;port=50000;PROTOCOL=TCPIP';

console.log("Test program to access DB2 sample database");
var testTable = 'BIGINTTEST';
var testValues = [10205152031467301, 10205152031467303];

/*Connect to the database server
  param 1: The DSN string which has the details of database name to connect to, user id, password, hostname, portnumber 
  param 2: The Callback function to execute when connection attempt to the specified database is completed
*/
ibmdb.open(common.connectionString, function(err, conn)
{
        if(err) {
          	console.error("error: ", err.message);
            assert.equal(err.message, null);
        } else {

		console.log('Connection to DB2 machine successful');
		
        try {
		    conn.querySync("create table " + testTable + " (COLINT BIGINT)");
        } catch (e) {}
		for(var i=0;i<testValues.length;i++) {
			conn.querySync("insert into " + testTable + " values (" + testValues[i] + ")");
		}
		
		/*
			On successful connection issue the SQL query by calling the query() function on Database
			param 1: The SQL query to be issued
			param 2: The callback function to execute when the database server responds
		*/
		conn.query("insert into " + testTable + " values (" + testValues[0] + ")", function(err, nodetest, moreResultSets) {
		
			if(err) {
				console.log('Error: '+err);
				process.exit(0);
			}
            conn.querySync("drop table " + testTable);

			/*
				Close the connection to the database
				param 1: The callback function to execute on completion of close function.
			*/
			conn.close(function(){
				console.log("Connection Closed");
			});
		});
	}
});

/*require the ibm_db module*/
var common = require("./common")
  , ibmdb = require("../")
  , db = new ibmdb.Database();

var connString = 'DRIVER={DB2 ODBC Driver};DATABASE=SAMPLE;UID=db2admin;PWD=db2admin;HOSTNAME=localhost;port=50000;PROTOCOL=TCPIP';

console.log("Test program to access DB2 sample database");

/*Connect to the database server
  param 1: The DSN string which has the details of database name to connect to, user id, password, hostname, portnumber 
  param 2: The Callback function to execute when connection attempt to the specified database is completed
*/
ibmdb.open(common.connectionString, function(err, conn)
{
        if(err) {
          	console.error("error: ", err.message);
        } else {

		console.log('Connection to DB2 machine successful');

		/*
			On successful connection issue the SQL query by calling the query() function on Database
			param 1: The SQL query to be issued
			param 2: The callback function to execute when the database server responds
		*/
		conn.query("SELECT TIMESTAMP_FORMAT('1999-12-31 23:59:59.123', 'YYYY-MM-DD HH24:MI:SS.FF') AS TD FROM SYSIBM.SYSDUMMY1;", function(err, nodetest, moreResultSets) {
		
			if(err) {
				console.log('Error: '+err);
				process.exit(0);
			}
			console.log('Fetched Row Count: '+nodetest.length);
			console.log("TIME");
			console.log("-----------------------");

			for (var i=0;i<nodetest.length;i++)
			{
				console.log(nodetest[i].TD);
			}
			console.log("-----------------------");

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
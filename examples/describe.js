/* 
 * This example discusses the use of the odbc describe method.
 * 
 */

var odbc = require("../odbc.js"),
      db = new odbc.Database();

      
//open a connection to the database
db.open("DSN=myDsnName;UID=myUserName;PWD=mySuperSecretPassword;DATABASE=myAwesomeDatabase", function(err)
{
	
	if (err) {
		//Something went bad
		console.log(err);
		
		//Let's not go any further
		return;
	}
	
	/*
	 * In its most basic form you must call describe passing it an object which 
	 * contains the database name and a callback function.
	 * 
	 * This will return a list of tables in the database for all schemas.
	 */
	
	db.describe({
		database : 'myAwesomeDatabase'
	}, function (error, result) {
		if (error) {
			console.log(error);
			return false;
		}
		
		console.log(result);
	});
	
	/*
	 * Sometimes there may be schemas that you don't want to see, on MSSQL, sys comes to mind
	 * 
	 * So you can specify which schema you are looking for by specifying the schema property.
	 */
	
	db.describe({
		database : 'myAwesomeDatabase',
		schema : 'dbo'
	}, function (error, result) {
		if (error) {
			console.log(error);
			return false;
		}
		
		console.log(result);
	});
	
	/*
	 * Or you can get a list of views by specifying the type property
	 */
	
	db.describe({
		database : 'myAwesomeDatabase',
		schema : 'dbo',
		type : 'view'
	}, function (error, result) {
		if (error) {
			console.log(error);
			return false;
		}
		
		console.log(result);
	});
	
	/*
	 * You can get a list of columns in a table by specifying the table property
	 */
	
	db.describe({
		database : 'myAwesomeDatabase',
		schema : 'dbo',
		table : 'customers'
	}, function (error, result) {
		if (error) {
			console.log(error);
			return false;
		}
		
		console.log(result);
	});
	
	/*
	 * Or you can get information on a specific column in a specific table by also specifying the column property
	 */
	
	db.describe({
		database : 'myAwesomeDatabase',
		schema : 'dbo',
		table : 'customers',
		column : 'name'
	}, function (error, result) {
		if (error) {
			console.log(error);
			return false;
		}
		
		console.log(result);
	});
});
var common = require("./common"), 
	odbc = require("../"),
	maxConnections = 5,
	connections = [],
	assert = require("assert"),
	util = require('util'),
	dropCallback = 0,
	initialMemHeap = 0,
	maxDiff = 1000000,
	closedConnectionCount = 0;
	try
	{
		global.gc();
		initialMemHeap = util.inspect(process.memoryUsage().heapUsed);
		for (var i = 0; i < maxConnections; i++)
		{ (function (i) {
			var db = new odbc.Database();
			connections.push(db);
			
			db.open(common.connectionString, function(err) {
				runQueries(db, "T1Leak" + i);
			});
		})(i);
		}
	}
	catch(e)
	{
		console.log(e);
		db.close(function () {
					console.log("Connection closed on error");
					});
	}
	
	function runQueries(db, tableName)
	{
		db.query("create table "+ tableName + " (PID INTEGER, C1 VARCHAR(255), C2 VARCHAR(255), C3 VARCHAR(255))", function(err, data){
			if (err == null)
			{
				console.log("Table "+ tableName + " created");
			}
					
			else
			{
				console.log(err);	
			}
		});
				
		db.query("INSERT into " + tableName + " values (1, 'PersonA', 'LastNameA', 'QA')", icback);
		db.query("INSERT into " + tableName + " values (2, 'PersonB', 'LastNameB', 'Dev')", icback);
		db.query("INSERT into " + tableName + " values (3, 'PersonC', 'LastNameC', 'QA')", icback);
		db.query("INSERT into " + tableName + " values (4, 'PersonD', 'LastNameD', 'QA')", icback);
		db.query("INSERT into " + tableName + " values (5, 'PersonE', 'LastNameE', 'QA')", icback);
		
		db.query("SELECT * from " + tableName, scback);
		
		db.query("UPDATE " + tableName + " SET C3 = 'QA Intern' where C2 = 'LastNameD'", ucback);
		db.query("SELECT * from " + tableName + " where C3 = 'QA Intern'", scback);
		db.query("SELECT count(*) from " + tableName + " where PID = 7", scback);
		db.query("DELETE from " + tableName + " where PID = 5", dcback);
		db.query("INSERT into " + tableName + " values (6, 'PersonF', 'LastNameF', 'QA Lead')", icback);
		db.query("SELECT * from " + tableName + " where PID = 5", scback);
		db.query("DROP table " + tableName, drcback.bind({tableName: tableName}));
	}

	function icback(err, data)
	{
		if (err == null)
		{
		}		
		else
		{
			console.log(err);	
		}
	}
			
	function scback(err, data)
	{
		if (err == null)
		{
			console.log("Select statement successful");
		}	
		else
		{
			console.log(err);	
		}
	}
	
	function ucback(err, data)
	{
		if (err == null)
		{
			console.log("Update statement successful");
		}	
		else
		{
			console.log(err);	
		}
	}

	function dcback(err, data)
	{
		if (err == null)
		{
			console.log("Delete row(s) successful");
		}	
		else
		{
			console.log(err);	
		}
	}

	function drcback(err, data)
	{
		if (err == null)
		{
			console.log("Drop table " + this.tableName + " successful");
		}		
		else
		{
			console.log(err);	
		}

		dropCallback++;
		if(dropCallback == maxConnections)
		{
			closedbConnections();
		}
	}

	function closedbConnections()
	{
		connections.forEach(function(db, ix){
			db.close(function () {
				console.log("Database Connection " + ix + " Closed");
				closedConnectionCount++
				if(closedConnectionCount == maxConnections)
				{
					checkMemory();
				}
			});
		});
	}
	function checkMemory()
	{
		global.gc();
		diffHeapUse =  util.inspect(process.memoryUsage().heapUsed) - initialMemHeap;
		console.log("DIFFERENCE ", diffHeapUse);
		assert(diffHeapUse < maxDiff);
	}
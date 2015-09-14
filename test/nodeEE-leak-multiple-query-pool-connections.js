var common = require("./common"), 
	odbc = require("../"),
	pool = new odbc.Pool(),
	maxConnections = 10,
	assert = require("assert"),
	util = require('util'),
	initialMemHeap = 0,
	maxDiff = 1000000;

	try
	{
		global.gc();
		initialMemHeap = util.inspect(process.memoryUsage().heapUsed);
		for (var i = 0; i <= maxConnections; i++)
		{ (function (i) {
				if (i == maxConnections) 
				{
	          		closeConnections();
	        	}
	        	else
	        	{
					console.log("Opening connection #", i);
	        		pool.open(common.connectionString, function (err, connection) {
						if (err) 
						{
							console.error("error: ", err.message);
					    }
						
		        		else
		        		{
		        			runQueries(connection, "T1Leak" + i);
		        		}
					});
	        	}
			})(i);
		}
	}
	catch(e)
	{
		console.log(e);
		pool.close(function () 
		{
    		console.log("pool closed on error");
    	});
	}
	
	function runQueries(connection, tableName)
	{
		connection.query("create table "+ tableName + " (PID INTEGER, C1 VARCHAR(255), C2 VARCHAR(255), C3 VARCHAR(255))", function(err, data){
			if (err == null)
			{
				console.log("Table "+ tableName + " created");
			}
					
			else
			{
				console.log(err);	
			}
		});
		connection.query("INSERT into " + tableName + " values (1, 'PersonA', 'LastNameA', 'QA')", icback);
		connection.query("INSERT into " + tableName + " values (2, 'PersonB', 'LastNameB', 'Dev')", icback);
		connection.query("INSERT into " + tableName + " values (3, 'PersonC', 'LastNameC', 'QA')", icback);
		connection.query("INSERT into " + tableName + " values (4, 'PersonD', 'LastNameD', 'QA')", icback);
		connection.query("INSERT into " + tableName + " values (5, 'PersonE', 'LastNameE', 'QA')", icback);
				
		connection.query("SELECT * from " + tableName, scback);
				
		connection.query("UPDATE " + tableName + " SET C3 = 'QA Intern' where C2 = 'LastNameD'", ucback);
		connection.query("SELECT * from " + tableName + " where C3 = 'QA Intern'", scback);
		connection.query("SELECT count(*) from " + tableName + " where PID = 7", scback);
		connection.query("DELETE from " + tableName + " where PID = 5", dcback);
		connection.query("INSERT into " + tableName + " values (6, 'PersonF', 'LastNameF', 'QA Lead')", icback);
		connection.query("SELECT * from " + tableName + " where PID = 5", scback);
		connection.query("DROP table " + tableName, drcback.bind({tableName: tableName}));
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
	}

	function closeConnections() 
	{
		pool.close(function () {
    		console.log("Database connection pool closed");
    		checkMemory();
    	});
	}
	
	function checkMemory()
	{
		global.gc();
		diffHeapUse =  util.inspect(process.memoryUsage().heapUsed) - initialMemHeap;
		console.log("DIFFERENCE ", diffHeapUse);
		assert(diffHeapUse < maxDiff);
	}
var common = require("./common"), 
	odbc = require("../"), 
	db = new odbc.Database(), 
	assert = require("assert"),
	util = require('util'),
	initialMemHeap = 0,
	maxDiff = 1000000,
	insertCount = 0;
	try
	{
		global.gc();
		initialMemHeap = util.inspect(process.memoryUsage().heapUsed);
		db.open(common.connectionString, function(err) {
			runQueries();
		});

	}
	catch(e)
	{
		console.log(e);
		db.close(function () {
			console.log("Connection closed on error");
		});
		
	}
	
	function runQueries()
	{
		db.query("create table T1Leak (PID INTEGER, C1 VARCHAR(255), C2 VARCHAR(255), C3 VARCHAR(255))", function(err, data){
			if (err == null)
			{
				console.log("Table T1Leak created");
			}
			
			else
			{
				console.log(err);	
			}
		});
			
		db.query("INSERT into T1Leak values (1, 'PersonA', 'LastNameA', 'QA')", icback);
		db.query("INSERT into T1Leak values (2, 'PersonB', 'LastNameB', 'Dev')", icback);
		db.query("INSERT into T1Leak values (3, 'PersonC', 'LastNameC', 'QA')", icback);
		db.query("INSERT into T1Leak values (4, 'PersonD', 'LastNameD', 'QA')", icback);
		db.query("INSERT into T1Leak values (5, 'PersonE', 'LastNameE', 'QA')", icback);
		
		db.query("SELECT * from T1Leak", scback);
		
		db.query("UPDATE T1Leak SET C3 = 'QA Intern' where C2 = 'LastNameD'", ucback);
		db.query("SELECT * from T1Leak where C3 = 'QA Intern'", scback);
		db.query("SELECT count(*) from T1Leak where PID = 7", scback);
		db.query("DELETE from T1Leak where PID = 5", dcback);
		db.query("INSERT into T1Leak values (6, 'PersonF', 'LastNameF', 'QA Lead')", icback);
		db.query("SELECT * from T1Leak where PID = 5", scback);
		db.query("DROP table T1Leak", drcback);
	}

	function icback(err, data)
	{
		if (err == null)
		{
			insertCount++;
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
			console.log("Drop table T1Leak successful");
			
		}		
		else
		{
			console.log(err);	
		}

		closedbConnection();
	}
	function closedbConnection()
	{
		db.close(function () {
			console.log("Database Connection Closed");
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
var common = require("./common"), 
	odbc = require("../"), 
	db = new odbc.Database(), 
	assert = require("assert"),
	async = require('async'),
	util = require('util'),
	tableOne = "T1",
	tableTwo = "T2",
	insertCount = 0,
	expected1 = [{ PID: '1', C1: 'PersonA', C2: 'LastNameA', C3: 'QA' }, { PID: '2', C1: 'PersonB', C2: 'LastNameB', C3: 'Dev' }, { PID: '3', C1: 'PersonC', C2: 'LastNameC', C3: 'QA' }, { PID: '4', C1: 'PersonD', C2: 'LastNameD', C3: 'QA' }, { PID: '5', C1: 'PersonE', C2: 'LastNameE', C3: 'QA' }],
	expected2 = [{ PID: '6', C1: 'PersonF', C2: 'LastNameF', C3: 'QA Lead' }, { PID: '7', C1: 'PersonG', C2: 'LastNameG', C3: 'Dev Lead' }],
	expected3 = [{ PID: '4', C1: 'PersonD', C2: 'LastNameD', C3: 'QA Intern' }];
	

	var funcArr = [];
    funcArr.push(main);
    async.waterfall(funcArr, function (err, result) {
    	if (err == null)
    	{
    		consol.log('Waterfall successful');
    	}	
    	else
    	{
    		console.error(err);
    	}
    });

	function main()
	{
		try
		{
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
	}
	
	
	function runQueries()
	{
		db.query("create table " + tableOne + " (PID INTEGER, C1 VARCHAR(255), C2 VARCHAR(255), C3 VARCHAR(255))", function(err, data){
			if (err == null)
			{
				console.log("Table " + tableOne + " created");
			}
			
			else
			{
				console.log(err);	
			}
		});
			
		db.query("create table " + tableTwo + " (PID INTEGER, C1 VARCHAR(255), C2 VARCHAR(255), C3 VARCHAR(255))", function(err, data){
			if (err == null)
			{
				console.log("Table " + tableTwo + " created");
			}
			
			else
			{
				console.log(err);	
			}
		});
			
		db.query("INSERT into " + tableOne + " values (1, 'PersonA', 'LastNameA', 'QA')", icback);
		db.query("INSERT into " + tableOne + " values (2, 'PersonB', 'LastNameB', 'Dev')", icback);
		db.query("INSERT into " + tableOne + " values (3, 'PersonC', 'LastNameC', 'QA')", icback);
		db.query("INSERT into " + tableOne + " values (4, 'PersonD', 'LastNameD', 'QA')", icback);
		db.query("INSERT into " + tableOne + " values (5, 'PersonE', 'LastNameE', 'QA')", icback);
		
		db.query("INSERT into " + tableTwo + " values (6, 'PersonF', 'LastNameF', 'QA Lead')", icback);
		db.query("INSERT into " + tableTwo + " values (7, 'PersonG', 'LastNameG', 'Dev Lead')", icback);
		
		db.query("SELECT * from " + tableOne, scback.bind({expected: expected1}));
		db.query("SELECT * from " + tableTwo, scback.bind({expected: expected2}));
		
		db.query("UPDATE " + tableOne + " SET C3 = 'QA Intern' where C2 = 'LastNameD'", ucback);
		db.query("SELECT * from " + tableOne + " where C3 = 'QA Intern'", scback.bind({expected: expected3}));
		
		db.query("DELETE from " + tableTwo + " where PID = 7", dcback);
		db.query("SELECT * from " + tableTwo + " where PID = 7", scback.bind({expected: []}));
		db.query("DROP table " + tableTwo, drcback); 
		db.query("SELECT tabname from syscat.tables where tabschema='ELKOREHP' and TABNAME='" + tableTwo + "'", scback.bind({expected: []}));
		db.query("DELETE from " + tableOne + " where PID = 5", dcback);
		db.query("SELECT * from " + tableOne + " where PID = 5", scback.bind({expected: []}));
		db.query("DROP table " + tableOne, drcback);
		db.query("SELECT tabname from syscat.tables where tabschema='ELKOREHP' and TABNAME='" + tableOne + "'", scback.bind({expected: []}));
		closeConnection();
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
		assert.deepEqual(data, this.expected);
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
			console.log("Drop table successful");
			
		}
					
		else
		{
			console.log(err);	
		}
	}
	function closeConnection()
	{
		db.close(function () 
		{
			console.log("Database Connection Closed");
		});
	}
	
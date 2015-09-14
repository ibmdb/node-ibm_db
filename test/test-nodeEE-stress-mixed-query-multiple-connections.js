var common = require("./common"), 
	odbc = require("../"),
	maxConnections = 5,
	connections = [],
	assert = require("assert"),
	util = require('util'),
	queryCallback = 0,
	insertCount = 0,
	expected1 = [{ PID: '1', C1: 'PersonA', C2: 'LastNameA', C3: 'QA' }, { PID: '2', C1: 'PersonB', C2: 'LastNameB', C3: 'Dev' }, { PID: '3', C1: 'PersonC', C2: 'LastNameC', C3: 'QA' }, { PID: '4', C1: 'PersonD', C2: 'LastNameD', C3: 'QA' }, { PID: '5', C1: 'PersonE', C2: 'LastNameE', C3: 'QA' }],
	expected2 = [{ PID: '6', C1: 'PersonF', C2: 'LastNameF', C3: 'QA Lead' }, { PID: '7', C1: 'PersonG', C2: 'LastNameG', C3: 'Dev Lead' }],
	expected3 = [{ PID: '4', C1: 'PersonD', C2: 'LastNameD', C3: 'QA Intern' }];
	try
	{

		for (var i = 0; i < maxConnections; i++)
		{ (function (i) {
			var db = new odbc.Database();
			connections.push(db);
			db.open(common.connectionString, function(err) {
				runQueries(db, "T1" + i, "T2" + i);
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
	
	function runQueries(db, tableOne, tableTwo)
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
		
		db.query("SELECT * from " + tableOne, scback.bind({done: false, expected: expected1}));
		db.query("SELECT * from " + tableTwo , scback.bind({done: false, expected: expected2}));
		
		db.query("UPDATE " + tableOne + " SET C3 = 'QA Intern' where C2 = 'LastNameD'", ucback);
		db.query("SELECT * from " + tableOne + " where C3 = 'QA Intern'", scback.bind({done: false, expected: expected3}));
		
		db.query("DELETE from " + tableTwo + " where PID = 7", dcback);
		db.query("SELECT * from " + tableTwo + " where PID = 7", scback.bind({done: false, expected: []}));
		db.query("DROP table " + tableTwo, drcback.bind({tableName: tableTwo})); 
		db.query("SELECT tabname from syscat.tables where tabschema='ELKOREHP' and TABNAME='" + tableTwo + "'", scback.bind({done: false, expected: []}));
		db.query("DELETE from " + tableOne + " where PID = 5", dcback);
		db.query("SELECT * from " + tableOne + " where PID = 5", scback.bind({done: false, expected: []}));
		db.query("DROP table " + tableOne , drcback.bind({tableName: tableTwo}));
		db.query("SELECT tabname from syscat.tables where tabschema='ELKOREHP' and TABNAME='" + tableOne + "'", scback.bind({done: true, expected: []}));
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

		if(this.done)
		{
			queryCallback++;
			if(queryCallback == maxConnections)
			{
				closedbConnections();
			}
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
	function drcback(err, data){
		if (err == null)
		{
			console.log("Drop table " + this.tableName + " successful");
			
		}
		else
		{
			console.log(err);	
		}
	}
	function closedbConnections()
	{
		connections.forEach(function(db, ix){
			db.close(function () {
				console.log("Database Connection " + ix + " Closed");
			});
		});
	}
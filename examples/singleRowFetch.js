/*
 * This example demonstrates how to fetch a single row from a large table
 * one by one and get processed by application. It avoids allocation of 
 * large memory by the ibm_db and data processing becomes easier.
 *
 * This example has used Sync methods, but async methods also exist.
 */

var ibmdb = require("../") //require("ibm_db")
  , conn = new ibmdb.Database()
  , cn = "database=sample;hostname=hotel.torolab.ibm.com;port=21169;uid=newton;pwd=xxxx"
  ;

// open a connection to the database
conn.openSync(cn);

// create table and insert some rows to it.
conn.querySync("create table mytab (c1 int, c2 varchar(20))");
conn.querySync("insert into mytab values (1, 'bimal'),(2, 'kamal'),(3,'mohan'),(4,'ram')");

// Select data from table
conn.queryResult("select * from mytab", function (err, result) {
  if(err) {
    console.log(err);
    return;
  }
  
  // Fetch single row at once and process it.
  // Note that queryResult will bring only 64k data from server and result.fetchSync
  // will return each row from this 64k client buffer. Once all data is read from
  // buffer, ibm_db driver will bring another 64k chunk of data from server.
  var data;
  while( data = result.fetchSync() )
  {
    console.log(data);
  }

  // drop the table and close connection.
  conn.querySync("drop table mytab");
  conn.closeSync();
});


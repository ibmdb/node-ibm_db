
// Test file to demonstrate use of fetch, fetchSynch, fetchAll and fetchAllSync APIs.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn=common.connectionString
  ;

ibmdb.open(cn,function(err,conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int, col3 decimal(11,0))");
  conn.querySync("insert into hits values ('something', 42, 343434)");
  conn.querySync("insert into hits values ('für', 43, 35353535)");

  // Fetch data Synchronously using fetchSync() API.
  var stmt = conn.prepareSync("select * from hits");
  stmt.setAttrSync(3, 2); //SQL_ATTR_MAX_LENGTH = 3
  var result = stmt.executeSync();
  var data = 0;
  console.log("-------------------------------------------------------------");
  console.log("result = ", result);
  console.log("Selected data using fetchSync() API = ");
  while( data = result.fetchSync({fetchMode:3}) ) {
    console.log(data);
  }
  result.closeSync();

  // Get column data in chunks using getDataSync API
  stmt = conn.prepareSync("select * from hits");
  result = stmt.executeSync();
  console.log("Retrieve column data using getDataSync API:");
  console.log(result.fetchSync({fetchMode:0}));
  console.log("First Row Data = ");
  console.log(result.getDataSync(1, 4));
  console.log(result.getDataSync(1, 5));
  console.log(result.getDataSync(2, 5));
  console.log(result.getDataSync(3, 5));
  result.fetchSync({fetchMode:0});
  console.log("Second Row Data = ");
  console.log(result.getDataSync(1, 4));
  console.log(result.getDataSync(1, 5));
  console.log(result.getDataSync(2, 5));
  console.log(result.getDataSync(3, 5));
  result.closeSync();
  console.log("------------------");

  // Fetch data Synchronously using fetchAllSync() API.
  //stmt = conn.prepareSync("select * from hits");
  result = stmt.executeSync();
  data = result.fetchAllSync();
  console.log("Selected rows using fetchAllSync() API = ", data);
  result.closeSync();

  // Fetch data Asynchronously using fetchAll() API.
  result = stmt.executeSync();
  result.fetchAll(function (err, data, colcount) {
    if(err) { console.log(err); }
    else {
      console.log("Selected data using fetchAll() API = ", data);
      console.log("No of columns in selected data = ", colcount);
      assert.deepEqual(colcount, 3);
    }
    result.closeSync();

    // Fetch data Asynchronously using Promisified fetch() API.
    stmt.execute(function (err, result) {
      if( err ) console.log(err);
      result.fetch().then(row => {
        console.log("\n\n--------------------------------------------");
        console.log("Selected data using Promisified fetch() API: ");
        console.log("--------------------------------------------");
        console.log("Row1 = ", row);
        return result.fetch({fetchMode:ibmdb.FETCH_ARRAY});
      }).then(row => {
        console.log("Row2 = ", row);
        result.closeSync();

        // Get column data in chunks using getData API
        stmt = conn.prepareSync("select * from hits");
        result = stmt.executeSync();
        console.log("Retrieve column data using getData API:");
        return result.fetch({fetchMode:0});
      }).then(() => {
        console.log("First Row Data = ");
        return result.getData(1, 4);
      }).then(data => {
        console.log(data);
        return result.getData(1, 5);
      }).then(data => {
        console.log(data);
        return result.getData(2, 5);
      }).then(data => {
        console.log(data);
        return result.getData(3, 5);
      }).then(data => {
        console.log(data);
        result.fetchSync({fetchMode:0});
        console.log("Second Row Data = ");
        return result.getData(1, 4);
      }).then( data => {
        console.log(data);
        return result.getData(1, 5);
      }).then(data => {
        console.log(data);
        console.log(result.getDataSync(2, 2));
        console.log(result.getDataSync(3, 3));
        result.closeSync();
        console.log("------------------");
        return testAsync(conn);
      }).then(() => {
        conn.querySync("drop table hits");
        //Close the connection
        conn.close(function(err){
          console.log("Connection Closed.");
          console.log("---------------------------------------------------");
        });
      }).catch(err => console.log(err));
    });
  });
});

// Select data asynchronously using async-await
async function testAsync(conn) {
  let stmt = conn.prepareSync("select * from hits");
  let result = stmt.executeSync();
  console.log("\n\n---------------------------------------");
  console.log("Retrieve column data using async-await:");
  console.log("---------------------------------------");
  await result.fetch({fetchMode:0});
  process.stdout.write("First Row Data = ");
  let data = await result.getData(1, 4);
  process.stdout.write(data);
  data = await result.getData(1, 5);
  process.stdout.write(data);
  process.stdout.write(", " + await result.getData(2, 5));
  console.log(", ", await result.getData(3, 5));
  await result.fetch({fetchMode:ibmdb.FETCH_NODATA});
  process.stdout.write("Second Row Data = ");
  process.stdout.write(await result.getData(1, 4));
  process.stdout.write(result.getDataSync(1, 5));
  process.stdout.write(", " + await result.getData(2, 5));
  console.log(", ", result.getDataSync(3, 5));
  await result.close();
  console.log("------ return from testAsync --------\n");
}


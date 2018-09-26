
// Test file to demonstrate use of fetch, fetchSynch, fetchAll and fetchAllSync APIs.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn=common.connectionString
  ;

ibmdb.open(cn,function(err,conn){
  conn.querySync("create table hits (col1 varchar(40), col2 int)");
  conn.querySync("insert into hits values ('something', 42)");
  conn.querySync("insert into hits values ('für', 43)");

  // Fetch data Synchronously using fetchSync() API.
  var stmt = conn.prepareSync("select * from hits");
  var result = stmt.executeSync();
  var data = 0;
  console.log("-------------------------------------------------------------");
  console.log("result = ", result);
  console.log("Selected data using fetchSync() API = ");
  while( data = result.fetchSync({fetchMode:3}) ) {
    console.log(data);
  }
  result.closeSync();

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
      assert.deepEqual(colcount, 2);
    }
    result.closeSync();

    // Fetch data Asynchronously using fetch() API.
    stmt.execute(function (err, result) {
      if( err ) console.log(err);
      result.fetch(function (err, row) {
          if(err) { console.log(err); }
          else {
            console.log("Selected data using fetch() API = ");
            console.log("Row1 = ", row);
            result.fetch({fetchMode:3},function (err, row) {
              if(err) { console.log(err); }
              console.log("Row2 = ", row);
              result.closeSync();
              conn.querySync("drop table hits");
              //Close the connection
              conn.close(function(err){
                console.log("Connection Closed.");
                console.log("-------------------------------------------" +
                            "------------------");
              });
            });
          }
      });
    });
  });
});


// Test case to test the OUT parameters and Result set returned by 
// Stored Procedure when Async and Sync forms of Prepare and Execute
// APIs are used.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , schema = common.connectionObject.CURRENTSCHEMA;

if(schema == undefined) schema = "NEWTON";
   
var proc1 = "create or replace procedure " + schema + ".PROC2 ( IN v1 int, INOUT v2 varchar(30) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from " + schema + ".mytab1; declare cr2  cursor with return for select c2 from " + schema + ".mytab1; open cr1; open cr2; set v2 = 'success'; end";
var proc2 = "create or replace procedure " + schema + ".PROC2 ( IN v1 int, INOUT v2 varchar(30) )  language sql begin  set v2 = 'success'; end";
var proc3 = "create or replace procedure " + schema + ".PROC2 ( IN v1 int, IN v2 varchar(30) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from " + schema + ".mytab1; declare cr2  cursor with return for select c2 from " + schema + ".mytab1; open cr1; open cr2; end";
var query = "call " + schema + ".PROC2(?, ?)";
var result;

if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
  // Update the queries for Db2 on z/OS compatiability.
  // - Does not support CREATE OR REPLACE syntax.
 var dropProc = "drop procedure " + schema + ".PROC2";
  proc1 = proc1.replace(" or replace", "");
  proc2 = proc2.replace(" or replace", "");
  proc3 = proc3.replace(" or replace", "");

  // - When creating an SQL native procedure on z/OS, you will need to have a WLM environment
  // defined for your system if you want to run the procedure in debugging mode. Adding
  // "disable debug mode" to bypass this requirement.
  proc1 = proc1.replace(" begin", " disable debug mode begin");
  proc2 = proc2.replace(" begin", " disable debug mode begin");
  proc3 = proc3.replace(" begin", " disable debug mode begin");
}


//ibmdb.debug(true);
ibmdb.open(common.connectionString, {fetchMode : 3}, function (err, conn) {
    if(err) { 
      console.log(err);
      process.exit(-1);
    }
    try {
      conn.querySync("drop table " + schema + ".mytab1");
    } catch (e) {};
    conn.querySync({"sql":"create table " + schema + ".mytab1 (c1 int, c2 varchar(20))", "noResults":true});
    if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
      // Db2 on z/OS does not support multi-row inserts
      conn.querySync("insert into " + schema + ".mytab1 values (2, 'bimal')");
      conn.querySync("insert into " + schema + ".mytab1 values (3, 'kumar')");
    } else {
      conn.querySync("insert into " + schema + ".mytab1 values (2, 'bimal'), (3, 'kumar')");
    }

    param2 = {ParamType:"INOUT", DataType:1, Data:"abc", Length:50};

    // Create SP with INOUT param and 2 Result Set.
    if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
      // CREATE OR REPLACE is not supported on z/OS.  Explicitly drop procedure.
      try {
         conn.querySync(dropProc);
      } catch(e) { };
    }
    conn.querySync(proc1);
    // Call SP Synchronously.
    stmt = conn.prepareSync(query);
    //console.log(stmt.bindSync(['1', param2]));
    result = stmt.executeSync(['1', param2]);
    console.log("Result for Sync call of PROC1 (1 OUT param and 2 Result Sets) ==>");
    if(Array.isArray(result))
    {
        // Print INOUT and OUT param values for SP.
        console.log(result[1]);
        assert.deepEqual(result[1], ['success']);
        result = result[0];
    }
    data = result.fetchAllSync();
    console.log(data);
    assert.equal(data.length, 2);
    while(result.moreResultsSync()) {
      data = result.fetchAllSync();
      console.log(data);
    }
    result.closeSync();
    stmt.closeSync();
    // Call SP Asynchronously.
    conn.prepare(query, function (err, stmt) {
      if (err) console.log(err);
      stmt.execute(['1', param2], function(err, result, outparams) {
        if( err ) console.log(err);  
        else {
          data = result.fetchAllSync();
          console.log("Result for Async call of PROC1 (1 OUT param and 2 Result Sets) ==>");
          console.log(outparams);
          assert.deepEqual(outparams, ['success']);
          console.log(data);
          assert.equal(data.length, 2);
          while(result.moreResultsSync()) {
            data = result.fetchAllSync();
            console.log(data);
          }
          result.closeSync();
        }
        stmt.closeSync();

        // Create SP with only OUT param and no resultset.
        if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
          // CREATE OR REPLACE is not supported on z/OS.  Explicitly drop procedure.
          try {
            conn.querySync(dropProc);
          } catch(e) { };
        }
        conn.querySync(proc2);
        // Call SP Synchronously.
        stmt = conn.prepareSync(query);
        result = stmt.executeSync(['1', param2]);
        console.log("Result for Sync call of PROC2 (1 OUT param and no Result Set) ==>");
        if(Array.isArray(result))
        {
          // Print INOUT and OUT param values for SP.
          console.log(result[1]);
          assert.deepEqual(result[1], ['success']);
          result = result[0];
        }
        data = result.fetchAllSync();
        if(data.length) console.log(data);
        result.closeSync();
        stmt.closeSync();
        // Call SP Asynchronously.
        conn.prepare(query, function (err, stmt) {
          if (err) console.log(err);
          stmt.execute(['1', param2], function(err, result, outparams) {
            if( err ) console.log(err);  
            else {
              result.closeSync();
              console.log("Result for Async call of PROC2 (1 OUT param and no Result Set) ==>");
              console.log(outparams);
              assert.deepEqual(outparams, ['success']);
            }
            stmt.closeSync();

            // Create SP with only Result Set and no OUT or INPUT param.
            if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
              // CREATE OR REPLACE is not supported on z/OS.  Explicitly drop procedure.
              try {
                conn.querySync(dropProc);
              } catch(e) { };
            }
            conn.querySync(proc3);
            // Call SP Synchronously.
            stmt = conn.prepareSync(query);
            result = stmt.executeSync(['1', 'abc']);
            console.log("Result for Sync call of PROC3 (only 2 Result Sets) ==>");
            if(Array.isArray(result))
            {
              // Print INOUT and OUT param values for SP.
              console.log(result[1]);
              assert.deepEqual(result[1], null);
              result = result[0];
            }
            data = result.fetchAllSync();
            console.log(data);
            assert.equal(data.length, 2);
            while(result.moreResultsSync()) {
              data = result.fetchAllSync();
              console.log(data);
            }
            result.closeSync();
            stmt.closeSync();
            // Call SP Asynchronously.
            conn.prepare(query, function (err, stmt) {
              if (err) console.log(err);
              stmt.execute(['1', 'abc'], function(err, result, outparams) {
                if( err ) console.log(err);  
                else {
                  data = result.fetchAllSync();
                  console.log("Result for Async call of PROC3 (only 2 Result Sets) ==>");
                  console.log(data);
                  assert.equal(data.length, 2);
                  while(result.moreResultsSync()) {
                    data = result.fetchAllSync();
                    console.log(data);
                  }
                  result.closeSync();
                }
                stmt.closeSync();

                // Do Cleanup.
		if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
                  conn.querySync("drop procedure " + schema + ".PROC2");
		} else {
                  conn.querySync("drop procedure " + schema + ".PROC2 ( INT, VARCHAR(30) )");
                }
                conn.querySync("drop table " + schema + ".mytab1");
                // Close connection in last only.
                conn.close(function (err) { console.log("done.");});
              });
            });
          });
        });
      });
    });
});


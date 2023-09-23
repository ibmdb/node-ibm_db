// Test case to test the OUT parameters and Result set returned by 
// Stored Procedure when Async and Sync forms of Prepare and Execute
// APIs are used.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , isZOS = common.isZOS
  , schema = common.connectionObject.CURRENTSCHEMA;

var proc1 = "create or replace procedure " + schema + ".PROC1 ( IN v1 int, INOUT v2 varchar(30) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from " + schema + ".mytab1; declare cr2  cursor with return for select c2 from " + schema + ".mytab1; open cr1; open cr2; set v2 = 'success'; end";
var proc2 = "create or replace procedure " + schema + ".PROC2 ( IN v1 int, INOUT v2 varchar(30) )  language sql begin  set v2 = 'success'; end";
var proc3 = "create or replace procedure " + schema + ".PROC3 ( IN v1 int, IN v2 varchar(30) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from " + schema + ".mytab1; declare cr2  cursor with return for select c2 from " + schema + ".mytab1; open cr1; open cr2; end";
var query1 = "call " + schema + ".PROC1(?, ?)";
var query2 = "call " + schema + ".PROC2(?, ?)";
var query3 = "call " + schema + ".PROC3(?, ?)";
var dropProc1 = "drop procedure " + schema + ".PROC1";
var dropProc2 = "drop procedure " + schema + ".PROC2";
var dropProc3 = "drop procedure " + schema + ".PROC3";

if (isZOS) {
  proc1 = common.sanitizeSP(proc1);
  proc2 = common.sanitizeSP(proc2);
  proc3 = common.sanitizeSP(proc3);
} else {
  dropProc1 = "drop procedure " + schema + ".PROC1 ( INT, VARCHAR(30) )";
  dropProc2 = "drop procedure " + schema + ".PROC2 ( INT, VARCHAR(30) )";
  dropProc3 = "drop procedure " + schema + ".PROC3 ( INT, VARCHAR(30) )";
}

var param2 = {ParamType:"INOUT", DataType:1, Data:"abc", Length:50};
var conn, stmt, result, data, err;

async function setupTestEnv() {
    try {
      conn = ibmdb.openSync(common.connectionString, {fetchMode : 3});
    } catch (err) {
      console.log(err);
      process.exit(-1);
    }
    await conn.query("drop table " + schema + ".mytab1").catch((e) => {});
    await conn.query(dropProc1).catch((e) => {});
    await conn.query(dropProc2).catch((e) => {});
    await conn.query(dropProc3).catch((e) => {});
    try {
      conn.querySync({"sql":"create table " + schema + ".mytab1 (c1 int, c2 varchar(20))", "noResults":true});
      conn.querySync("insert into " + schema + ".mytab1 values (2, 'bimal')");
      conn.querySync("insert into " + schema + ".mytab1 values (3, 'kumar')");
    } catch(err) {
      console.log(err);
      process.exit(-1);
    }

    // Create SP with INOUT param and 2 Result Set.
    await conn.query(proc1).catch((e) => {console.log(e);});
    // Create SP with only OUT param and no resultset.
    await conn.query(proc2).catch((e) => {console.log(e);});
    // Create SP with only Result Set and no OUT or INPUT param.
    await conn.query(proc3).catch((e) => {console.log(e);});
}

function closeStmt() {
    result.closeSync();
    stmt.closeSync();
    result = "";
    stmt = "";
    data = "";
}

// Call SP Synchronously.
function syncTestSP() {
    //==> Test 1 <== *******************************************************
    stmt = conn.prepareSync(query1);
    //console.log(stmt.bindSync(['1', param2]));
    // Passing '1' throws SQL0420N on windows, so use 1.
    result = stmt.executeSync([1, param2]);
    console.log("\nTest 1: Result for Sync call of PROC1 (1 OUT param and 2 Result Sets) ==>");
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
    closeStmt();

    //==> Test 2 <== *******************************************************
    stmt = conn.prepareSync(query2);
    result = stmt.executeSync([1, param2]);
    console.log("\nTest 2: Result for Sync call of PROC2 (1 OUT param and no Result Set) ==>");
    if(Array.isArray(result))
    {
      // Print INOUT and OUT param values for SP.
      console.log(result[1]);
      assert.deepEqual(result[1], ['success']);
      result = result[0];
    }
    data = result.fetchAllSync();
    if(data.length) console.log(data);
    closeStmt();

    //==> Test 3 <== *******************************************************
    stmt = conn.prepareSync(query3);
    result = stmt.executeSync([1, 'abc']);
    console.log("\nTest 3: Result for Sync call of PROC3 (only 2 Result Sets) ==>");
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
    closeStmt();
}

// Call SP Asynchronously using async await.
async function awaitTestSP() {
    //==> Test 4 <== *******************************************************
    stmt = await conn.prepare(query1);
    result = await stmt.execute([1, param2]);
    console.log("\nTest 4: Result for AWAIT call of PROC1 (1 OUT param and 2 Result Sets) ==>");
    if(Array.isArray(result))
    {
        // Print INOUT and OUT param values for SP.
        console.log(result[1]);
        assert.deepEqual(result[1], ['success']);
        result = result[0];
    }
    data = await result.fetchAll();
    console.log(data);
    assert.equal(data.length, 2);
    while(result.moreResultsSync()) {
      data = await result.fetchAll();
      console.log(data);
    }
    closeStmt();

    //==> Test 5 <== *******************************************************
    stmt = await conn.prepare(query2);
    result = await stmt.execute([1, param2]);
    console.log("\nTest 5: Result for Sync call of PROC2 (1 OUT param and no Result Set) ==>");
    if(Array.isArray(result))
    {
      // Print INOUT and OUT param values for SP.
      console.log(result[1]);
      assert.deepEqual(result[1], ['success']);
      result = result[0];
    }
    data = await result.fetchAll();
    if(data.length) console.log(data);
    closeStmt();

    //==> Test 6 <== *******************************************************
    stmt = await conn.prepare(query3);
    result = await stmt.execute([1, 'abc']);
    console.log("\nTest 6: Result for Sync call of PROC3 (only 2 Result Sets) ==>");
    if(Array.isArray(result))
    {
      // Print INOUT and OUT param values for SP.
      console.log(result[1]);
      assert.deepEqual(result[1], null);
      result = result[0];
    }
    data = await result.fetchAll();
    console.log(data);
    assert.equal(data.length, 2);
    while(result.moreResultsSync()) {
      data = await result.fetchAll();
      console.log(data);
    }
    closeStmt();
}

// Call SP Asynchronously using callback function.
function callbackTestSP() {
    //==> Test 7 <== *******************************************************
    conn.prepare(query1, function (err, stmt) {
      if (err) console.log(err);
      stmt.execute([1, param2], function(err, result, outparams) {
        if( err ) console.log(err);  
        else {
          data = result.fetchAllSync();
          console.log("\nTest 7: Result for Async call of PROC1 (1 OUT param and 2 Result Sets) ==>");
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

        //==> Test 8 <== *******************************************************
        conn.prepare(query2, function (err, stmt) {
          if (err) console.log(err);
          stmt.execute([1, param2], function(err, result, outparams) {
            if( err ) console.log(err);  
            else {
              result.closeSync();
              console.log("\nTest 8: Result for Async call of PROC2 (1 OUT param and no Result Set) ==>");
              console.log(outparams);
              assert.deepEqual(outparams, ['success']);
            }
            stmt.closeSync();

            //==> Test 9 <== *******************************************************
            conn.prepare(query3, function (err, stmt) {
              if (err) console.log(err);
              stmt.execute([1, 'abc'], function(err, result, outparams) {
                if( err ) console.log(err);  
                else {
                  data = result.fetchAllSync();
                  console.log("\nTest 9: Result for Async call of PROC3 (only 2 Result Sets) ==>");
                  console.log(data);
                  assert.equal(data.length, 2);
                  while(result.moreResultsSync()) {
                    data = result.fetchAllSync();
                    console.log(data);
                  }
                  result.closeSync();
                }
                stmt.closeSync();
                cleanupTestEnv();
              });
            });
          });
        });
      });
    });
}

function cleanupTestEnv() {
    // Do Cleanup.
    try {
      conn.querySync(dropProc1);
      conn.querySync(dropProc2);
      conn.querySync(dropProc3);
      conn.querySync("drop table " + schema + ".mytab1");
    } catch(e) {};
    // Close connection in last only.
    conn.close(function (err) { console.log("done.");});
}

async function main() {
    await setupTestEnv();
    syncTestSP();
    await awaitTestSP();
    callbackTestSP();
}

main();


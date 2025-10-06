var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , isZOS = common.isZOS
  , schema = common.connectionObject.CURRENTSCHEMA;

var singleTestMode = false;
console.log("=> Run this script using \"node --expose-gc test-sp-memleak.js\" command.");
console.log("==================== TEST 1 =============================\n");

var proc1 = `create or replace procedure "${schema}".PROC1 ( IN v1 BLOB(200), IN v2 varchar(200), IN v3 BLOB(200), IN v4 varchar(200), INOUT V5 BLOB(200), INOUT v6 varchar(200), INOUT V7 BLOB(200), INOUT v8 varchar(200), OUT V9 BLOB(200), OUT v10 varchar(200), OUT V11 BLOB(200), OUT v12 varchar(200) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from ${schema}.mytab1; declare cr2  cursor with return for select c2 from ${schema}.mytab1; open cr1; open cr2; set v5 = BLOB(x'64656667'); set v6 = 'successs10'; set v7 = BLOB(x'68697071'); set v8 = 'successss11'; set v9 = BLOB(x'64656667'); set v10 = 'successs10'; set v11 = BLOB(x'68697071'); set v12 = 'successss11'; end`;
var proc2 = `create or replace procedure "${schema}".PROC2 ( IN v1 BLOB(200), IN v2 varchar(200), IN v3 BLOB(200), IN v4 varchar(200), INOUT V5 BLOB(200), INOUT v6 varchar(200), INOUT V7 BLOB(200), INOUT v8 varchar(200), OUT V9 BLOB(200), OUT v10 varchar(200), OUT V11 BLOB(200), OUT v12 varchar(200) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from ${schema}.mytab1; declare cr2  cursor with return for select c2 from ${schema}.mytab1; open cr1; open cr2; set v5 = BLOB(x'64656667'); set v7 = BLOB(x'68697071'); set v6 = 'successs10'; set v8 = 'successss11'; set v9 = v1; set v10 = v2; set v11 = v3; set v12 = v4; end`;

proc1 = common.sanitizeSP(proc1);
proc2 = common.sanitizeSP(proc2);

var j = 1;
var innerLoopCount = 5; //Change to 500 to run individually using --expose_gc
var mainLoopCount = 2;  //Change to 20 to run individually.
var timeout = 300;
if(singleTestMode) {
    innerLoopCount = 500;
    mainLoopCount = 20;
    timeout = 30000;
}

gc();
var start = process.memoryUsage().heapUsed;
console.log("Memory Usage at Start = ", process.memoryUsage());
main();

async function main()
{
  for(j = 1; j <= mainLoopCount; j++)
  {
      await callSPinLoop();
  }
  gc();
  setTimeout( function() {
    gc();
    console.log("Memory Usage = ", process.memoryUsage());
    console.log("Heap Memory used =", process.memoryUsage().heapUsed - start);
  }, timeout);
}

async function callSPinLoop()
{
    var conn;
    try {
      conn = ibmdb.openSync(common.connectionString, {fetchMode : 3});
    } catch (err) {
      console.log(err);
      process.exit(-1);
    }
    try {
      conn.querySync({"sql":"create table " + schema + ".mytab1 (c1 int, c2 varchar(200))", "noResults":true});
    } catch (e) {};

    if (isZOS) {
      // Db2 on z/OS does not support multi-row inserts
      conn.querySync("insert into " + schema + ".mytab1 values (2, 'bimal')");
      conn.querySync("insert into " + schema + ".mytab1 values (3, 'kumar')");
      // Explicitly drop procedure on z/OS.
      try {
        conn.querySync("drop procedure " + schema + ".PROC1");
      } catch(e) { };
      try {
        conn.querySync("drop procedure " + schema + ".PROC2");
      } catch(e) { };
    }
    else {
      conn.querySync("insert into " + schema + ".mytab1 values (2, 'bimal'), (3, 'kumar')");
    }

    // Create Stored Procedures.
    let createProcResult = conn.querySync(proc1);
    if(createProcResult.length) console.log(createProcResult);
    createProcResult = conn.querySync(proc2);
    if(createProcResult.length) console.log(createProcResult);

    await testSP(conn);
    gc();
    for(let i = 1; i <= innerLoopCount; i++) {
      await testSP(conn);

      if(i%100 == 0) {
        gc();
        console.log("Heap Memory used(i :", i, ", j :", j, ") =", process.memoryUsage().heapUsed - start);
        console.log("Memory Usage = ", process.memoryUsage());
      }
    }

    // Do Cleanup.
    if (isZOS) {
      conn.querySync("drop procedure " + schema + ".PROC1");
      conn.querySync("drop procedure " + schema + ".PROC2");
    } else {
      conn.querySync("drop procedure " + schema + ".PROC1 ( BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200) )");
      conn.querySync("drop procedure " + schema + ".PROC2 ( BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200), BLOB(200), VARCHAR(200) )");
    }
    await conn.query("drop table " + schema + ".mytab1");
    await conn.close();
    conn = undefined;
    console.log('Done');
}

async function testSP(conn)
{
    let result;
    let query1 = "CALL proc1(?,?,?,?,?,?,?,?,?,?,?,?);";
    let query2 = "CALL proc1(?,?,?,?,?,?,?,?,?,?,?,?);";
    let param1 = {ParamType:"INPUT", DataType:"BLOB", Data:Buffer.from("abcd")};
    let param2 = {ParamType:"INPUT", DataType:1, Data:Buffer.from("xyz")};
    let param3 = {ParamType:"INPUT", DataType:"BLOB", Data:"efgh"};
    let param4 = {ParamType:"INPUT", DataType:1, Data:"pqrst"};
    let param5 = {ParamType:"INOUT", DataType:"BLOB", Data:Buffer.from("ijklm"), Length:100};
    let param6 = {ParamType:"INOUT", DataType:1, Data:Buffer.from("nop"), Length:120};
    let param7 = {ParamType:"INOUT", DataType:"BLOB", Data:"bcde", Length:200};
    let param8 = {ParamType:"INOUT", DataType:1, Data:"xyz", Length:200};
    let param9 = {ParamType:"OUTPUT", DataType:"BLOB", Data:Buffer.from("ijk"), Length:200};
    let param10 = {ParamType:"OUTPUT", DataType:1, Data:Buffer.from("mnop"), Length:200};
    let param11 = {ParamType:"OUTPUT", DataType:"BLOB", Data:"", Length:200};
    let param12 = {ParamType:"OUTPUT", DataType:1, Data:"", Length:200};
    let params = [param1, param2, param3, param4,
                  param5, param6, param7, param8,
                  param9, param10, param11, param12];

    // Call SP Synchronously.
    result = conn.querySync(query1, params);
    if(!singleTestMode) console.log("Result for Sync call of proc1 ==>", result);
    assert.equal(result.length, 10);
    result = await conn.query(query2, params);
    if(!singleTestMode) console.log("Result for Sync call of proc2 ==>", result);
    assert.equal(result.length, 10);
}


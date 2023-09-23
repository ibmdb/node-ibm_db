var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , isZOS = common.isZOS
  , schema = common.connectionObject.CURRENTSCHEMA;

//==================== TEST 1 =============================
console.log("==================== TEST 1 =============================\n");

var proc1 = "create or replace procedure " + schema + ".PROC2 ( IN v1 int, INOUT v2 varchar(30) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from " + schema + ".mytab1; declare cr2  cursor with return for select c2 from " + schema + ".mytab1; open cr1; open cr2; set v2 = 'success'; end";
var proc2 = "create or replace procedure " + schema + ".PROC2 ( IN v1 int, INOUT v2 varchar(30) )  language sql begin  set v2 = 'success'; end";
var proc3 = "create or replace procedure " + schema + ".PROC2 ( IN v1 int, IN v2 varchar(30) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from " + schema + ".mytab1; declare cr2  cursor with return for select c2 from " + schema + ".mytab1; open cr1; open cr2; end";
var query = "call " + schema + ".PROC2(?, ?)";
var dropProc = "drop procedure " + schema + ".PROC2";
var result;

proc1 = common.sanitizeSP(proc1);
proc2 = common.sanitizeSP(proc2);
proc3 = common.sanitizeSP(proc3);

ibmdb.open(common.connectionString, {fetchMode : 3}, function (err, conn) {
    if(err) {
      console.log(err);
      process.exit(-1);
    }
    try {
      conn.querySync({"sql":"create table " + schema + ".mytab1 (c1 int, c2 varchar(20))", "noResults":true});
    } catch (e) {};

    conn.querySync("insert into " + schema + ".mytab1 values (2, 'bimal')");
    conn.querySync("insert into " + schema + ".mytab1 values (3, 'kumar')");

    param2 = {ParamType:"INOUT", DataType:1, Data:"abc", Length:50};

    // Create SP with INOUT param and 2 Result Set.
    if (isZOS) {
      // CREATE OR REPLACE is not supported on z/OS.  Explicitly drop procedure.
      try {
         conn.querySync(dropProc);
      } catch(e) { };
    }
    conn.querySync(proc1);

    // Call SP Synchronously.
    result = conn.querySync(query, [1, param2]);
    console.log("Result for Sync call of proc1 ==>");
    console.log(result);
    assert.equal(result.length, 3);
    // Call SP Asynchronously.
    conn.query(query, [1, param2], function (err, result) {
        if (err) console.log(err);
        else {
          console.log("Result for Async call of proc1 ==>");
          console.log(result);
          assert.equal(result.length, 3);
        }

        // Create SP with only OUT param and no resultset.
        if (isZOS) {
          // CREATE OR REPLACE is not supported on z/OS.  Explicitly drop procedure.
          try {
            conn.querySync(dropProc);
          } catch(e) { };
        }
        conn.querySync(proc2);
        // Call SP Synchronously.
        result = conn.querySync(query, [1, param2]);
        console.log("Result for Sync call of proc2 ==>");
        console.log(result);
        assert.equal(result.length, 1);
        // Call SP Asynchronously.
        conn.query(query, [1, param2], function (err, result) {
            if (err) console.log(err);
            else {
              console.log("Result for Async call of proc2 ==>");
              console.log(result);
              assert.equal(result.length, 1);
            }

            // Create SP with only Result Set and no OUT or INPUT param.
            if (isZOS) {
              // CREATE OR REPLACE is not supported on z/OS.  Explicitly drop procedure.
              try {
                conn.querySync(dropProc);
              } catch(e) { };
            }
            conn.querySync(proc3);
            // Call SP Synchronously.
            result = conn.querySync(query, [1, 'abc']);
            console.log("Result for Sync call of proc3 ==>");
            console.log(result);
            assert.equal(result.length, 2);
            // Call SP Asynchronously.
            conn.query(query, [1, 'abc'], function (err, result) {
                if (err) console.log(err);
                else {
                  console.log("Result for Async call of proc3 ==>");
                  console.log(result);
                  assert.equal(result.length, 2);
                }

                // Do Cleanup.
                if (isZOS) {
                  conn.querySync("drop procedure " + schema + ".PROC2");
                } else {
                  conn.querySync("drop procedure " + schema + ".PROC2 ( INT, VARCHAR(30) )");
                }
                conn.querySync("drop table " + schema + ".mytab1");
                // Call second test
                testDate(conn);
            });
        });
    });
});

//==================== TEST 2 =============================

function testDate(conn)
{
    console.log("\n==================== TEST 2 ===========================\n");
    var proc1 = "create procedure " + schema + ".PROC4(IN v1 int, OUT v2 DATE)"+
        " dynamic result sets 1 language sql begin  declare cr1  cursor with " +
        "return for select c1, c2 from " + schema + ".mytab1; open cr1; " +
        "set v2 = CURRENT DATE; end";
    var param2 = {ParamType:"OUTPUT", DataType:"DATE", Data:"2020-08-15", Length:30};
    var query = "call " + schema + ".PROC4(?, ?)";
    var dropProc = "drop procedure " + schema + ".PROC4";

    proc1 = common.sanitizeSP(proc1);
    try {
      conn.querySync("create table " + schema + ".mytab1 (c1 int, c2 DATE)");
    } catch (e) { console.log(e); }

    conn.querySync("insert into " +schema+ ".mytab1 values (5, '2020-04-22')");

    // Create Stored Procedure
    try {
       conn.querySync(dropProc);
    } catch(e) { };
    var err = conn.querySync(proc1);
    if(err.length) { console.log(err); }

    // Call SP Synchronously.
    var result = conn.querySync(query, [1, param2]);
    console.log("Result for Sync call of proc4 ==>");
    console.log(result);
    assert.equal(result.length, 2);
    // Call SP Asynchronously.
    conn.query(query, [1, param2], function (err, result) {
        if (err) console.log(err);
        else {
          console.log("Result for Async call of proc4 ==>");
          console.log(result);
          assert.equal(result.length, 2);
        }

        // Do Cleanup.
        if (isZOS) {
          conn.querySync("drop procedure " + schema + ".PROC4");
        } else {
          conn.querySync("drop procedure " + schema + ".PROC4 ( INT, DATE )");
        }
        conn.querySync("drop table " + schema + ".mytab1");
        testInteger(conn);
    });
}

//==================== TEST 3  - for issue #835 =============================

function testInteger(conn)
{
    console.log("\n==================== TEST 3 ===========================\n");
    var proc1 = "CREATE or replace PROCEDURE " + schema + ".TEST_PROC ( IN INPUT1 INTEGER, OUT OUTPUT1 INTEGER, OUT OUTPUT2 VARCHAR(500) ) " +
                "LANGUAGE SQL SPECIFIC " + schema + ".TEST_PROC NOT DETERMINISTIC MODIFIES SQL DATA BEGIN " +
                "SET OUTPUT1 = INPUT1 + 300; SET OUTPUT2 = 'Hello this a returned result'; END ;";
    var params = [454548, { ParamType: 'OUTPUT', SQLType: 'INTEGER', Data: 0 },{ ParamType: 'OUTPUT', DataType: 'VARCHAR', Data: '', Length: 500 }];
    var query = "call " + schema + ".TEST_PROC(?, ?, ?)";
    var dropProc = "drop procedure " + schema + ".TEST_PROC( INT, INT, VARCHAR(500) )";

    if (isZOS) {
      dropProc = "drop procedure " + schema + ".TEST_PROC";
      proc1 = common.sanitizeSP(proc1);
    }
    var err = conn.querySync(proc1);
    if(err.length) { console.log(err); }

    // Call SP Synchronously.
    result = conn.querySync(query, params);
    conn.querySync(dropProc);
    console.log("Result for Sync call of test_proc ==>");
    console.log(result);
    assert.equal(result.length, 2);
    assert.equal(result[0], 454848);
    testNullBigINT(conn);
}

//==================== TEST 4  - for issue #940 =============================

function testNullBigINT(conn)
{
    console.log("\n==================== TEST 4 ===========================\n");
    var proc1 = "CREATE or replace PROCEDURE " + schema + ".BIGINT_SP( IN c1 INTEGER, INOUT c2 BIGINT, OUT c3 BIGINT ) " +
                "LANGUAGE SQL SPECIFIC " + schema + ".BIGINT_SP NOT DETERMINISTIC MODIFIES SQL DATA BEGIN " +
                "SET c3 = c1 + 300; SET c2 = 8; END ;";
    var query = "call " + schema + ".BIGINT_SP(?, ?, ?)";
    var dropProc = "drop procedure " + schema + ".BIGINT_SP( INT, BIGINT, BIGINT )";
    if (isZOS) {
      dropProc = "drop procedure " + schema + ".BIGINT_SP";
      proc1 = common.sanitizeSP(proc1);
    }

    const inparam = { ParamType: 'INPUT', DataType: 'INT', Data: 5 };
    var mrf = null;
    const inoutparam = { ParamType: 'INOUT', DataType: 'BIGINT', Data: mrf };
    const outparam = { ParamType: 'OUTPUT', DataType: 'BIGINT', Data: 11, Length:8 };

    var err = conn.querySync(proc1);
    if(err.length) { console.log(err); }

    // Call SP Synchronously.
    result = conn.querySync(query, [inparam, inoutparam, outparam]);
    console.log("Result = ", result);
    assert.equal(result.length, 2);
    conn.querySync(dropProc);
    // Close connection in last only.
    conn.close((err) => {console.log('Done');});
}


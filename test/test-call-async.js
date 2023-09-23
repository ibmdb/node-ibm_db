// Test case to verify result of OUT and INOUT Parameters in a Strored Procedure.
// When SP is called using conn.query() assynchronously.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString
  , iszos = common.isZOS
  , schema = common.connectionObject.CURRENTSCHEMA;

ibmdb.open(cn, function (err, conn)
{
    if(err) console.log(err);
    assert.equal(err, null);
    var query = "CaLL " + schema + ".PROC1(?, ?, ?)";
    var proc1 = "create procedure " + schema + ".PROC1 " +
               "(IN v1 INTEGER, OUT v2 INTEGER, INOUT v3 VARCHAR(20)) " +
               "BEGIN set v2 = v1 + 1; set v3 = 'verygood'; END";
    var proc2 = "create or replace procedure " + schema + ".PROC2 (IN v1 INTEGER) BEGIN END";
    var dropProc1 = "drop procedure " + schema + ".PROC1(INT, INT, VARCHAR(20))";
    var dropProc2 = "drop procedure " + schema + ".PROC2(INT)";

    if (iszos) {
        // The DROP PROCEDURE syntax on z/OS can only specify the name of the procedure to drop.
        dropProc1 = "drop procedure " + schema + ".PROC1";
        dropProc2 = "drop procedure " + schema + ".PROC2";
        proc1 = common.sanitizeSP(proc1);
        proc2 = common.sanitizeSP(proc2);
    }
    try {
        conn.querySync(dropProc1);
        console.log("PROC1 dropped.\n");
    } catch(e) { }

    conn.querySync(proc1);
    console.log("created PROC1...\n");
    conn.commitTransactionSync();
    var param1 = {ParamType:"INPUT", DataType:1, Data:3};
    var param2 = {ParamType:"OUTPUT", DataType:1, Data:0};
    var param3 = {ParamType:"INOUT", DataType:1, Data:"abc", Length:30};

    conn.query(query, [param1, param2, param3], function(err, result){
        if(err) console.log(err);
        else {
            console.log("return value = ", result[0], result[1]);
        }
        conn.querySync(dropProc1);
        assert.deepEqual(result, [ 4, 'verygood' ]);
    });

    try {
        conn.querySync(dropProc2);
        console.log("PROC2 dropped.\n");
    } catch(e) { }
    conn.querySync(proc2);
    query = "call " + schema + ".PROC2(?)";
    conn.query({"sql":query, "params" : [param1]}, function(err, result){
        if(err) console.log(err);
        conn.querySync(dropProc2);
        conn.closeSync();
        assert.equal(result.length, 0);
        console.log('done');
    });
});

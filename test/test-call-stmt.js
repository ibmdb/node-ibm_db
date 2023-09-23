// Test case to verify result of OUT and INOUT Parameters in a Strored Procedure.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString
  , iszos = common.isZOS
  , schema = common.connectionObject.CURRENTSCHEMA;

var query = "CaLL " + schema + ".PROC1(?, ?, ?)";
ibmdb.open(cn, function (err, conn)
{
    if(err) console.log(err);
    assert.equal(err, null);
    let proc1 = "create or replace procedure " + schema + ".PROC1 " +
                "(IN v1 INTEGER, OUT v2 INTEGER, INOUT v3 VARCHAR(20)) " +
                "BEGIN set v2 = v1 + 1; set v3 = 'verygood'; END";
    let proc2 = "create or replace procedure " + schema + ".PROC2 (IN v1 INTEGER) BEGIN END";
    let dropProc1 = "drop procedure " + schema + ".PROC1(INT, INT, VARCHAR(20))";
    let dropProc2 = "drop procedure " + schema + ".PROC2(INT)";

    if (iszos) {
        proc1 = common.sanitizeSP(proc1);
        proc2 = common.sanitizeSP(proc2);
        dropProc1 = "drop procedure " + schema + ".PROC1";
        dropProc2 = "drop procedure " + schema + ".PROC2";
    }

    try {
        conn.querySync(dropProc1);
    } catch(e) {}
    conn.querySync(proc1);

    var param1 = {ParamType:"INPUT", DataType:1, Data:0};
    var param2 = {ParamType:"OUTPUT", DataType:1, Data:0};
    var param3 = {ParamType:"INOUT", DataType:1, Data:"abc", Length:30};

    result = conn.querySync(query, [param1, param2, param3]);
    assert.deepEqual(result, [ 1, 'verygood' ]);
    console.log("Output Parameters V2 = ", result[0], ", V3 = ", result[1]);
    conn.querySync(dropProc1);

    try {
        conn.querySync(dropProc2);
    } catch(e) {}
    conn.querySync(proc2);

    result = conn.querySync("call " + schema + ".PROC2(?)", [param1]);
    assert.deepEqual(result, []);
    conn.querySync(dropProc2);
    conn.closeSync();
    console.log('done');
});

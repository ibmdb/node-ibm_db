// Test case to verify result of OUT and INOUT Parameters in a Strored Procedure.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString
  , schema = common.connectionObject.CURRENTSCHEMA;

if(schema == undefined) schema = "NEWTON";
var query = "CaLL " + schema + ".proc1(?, ?, ?)";
ibmdb.open(cn, function (err, conn)
{
    if(err) console.log(err);
    assert.equal(err, null);
    try {
          conn.querySync("drop procedure " + schema + ".proc1(INT, INT, VARCHAR(20))");
    } catch(e) {}

    conn.querySync("create or replace procedure " + schema + ".proc1 " +
                   "(IN v1 INTEGER, OUT v2 INTEGER, INOUT v3 VARCHAR(20)) " +
                   "BEGIN set v2 = v1 + 1; set v3 = 'verygood'; END");
    var param1 = {ParamType:"INPUT", DataType:1, Data:0};
    var param2 = {ParamType:"OUTPUT", DataType:1, Data:0};
    var param3 = {ParamType:"INOUT", DataType:1, Data:"abc", Length:30};

    result = conn.querySync(query, [param1, param2, param3]);
    assert.deepEqual(result, [ 1, 'verygood' ]);
    console.log("Output Parameters V2 = ", result[0], ", V3 = ", result[1]);

    conn.querySync("drop procedure " + schema + ".proc1(INT, INT, VARCHAR(20))");
    conn.querySync("create or replace procedure " + schema + ".proc2 (IN v1 INTEGER) BEGIN END");
    result = conn.querySync("call " + schema + ".proc2(?)", [param1]);
    assert.deepEqual(result, []);
    conn.querySync("drop procedure " + schema + ".proc2(INT)");
    conn.closeSync();
    console.log('done');
});

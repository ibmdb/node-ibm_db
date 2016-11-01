// Test case to verify result of OUT and INOUT Parameters in a Strored Procedure.
// When SP is called using conn.query() assynchronously.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString
  , schema = common.connectionObject.CURRENTSCHEMA;

if(schema == undefined) schema = "NEWTON";
ibmdb.open(cn, function (err, conn)
{
    var query = "CaLL " + schema + ".proc1(?, ?, ?)";
    if(err) return console.log(err);
    try {
          conn.querySync("drop procedure " + schema + ".proc1");
          console.log("proc1 dropped.\n");
    } catch(e) {}
    conn.querySync("create procedure " + schema + ".proc1 " +
                   "(IN v1 INTEGER, OUT v2 INTEGER, INOUT v3 VARCHAR(20)) " +
                   "BEGIN set v2 = v1 + 1; set v3 = 'verygood'; END");
    console.log("created proc1...\n");
    conn.commitTransactionSync();
    var param1 = {ParamType:"INPUT", DataType:1, Data:3};
    var param2 = {ParamType:"OUTPUT", DataType:1, Data:0};
    var param3 = {ParamType:"INOUT", DataType:1, Data:"abc", Length:30};

    conn.query(query, [param1, param2, param3], function(err, result){
        if(err) console.log(err);
        else {
            console.log("return value = ", result[0], result[1]);
        }
        conn.querySync("drop procedure " + schema + ".proc1");
        conn.closeSync();
        assert.deepEqual(result, [ 4, 'verygood' ]);
    });
    conn.querySync("create or replace procedure " + schema + ".proc2 (IN v1 INTEGER) BEGIN END");
    conn.query("call " + schema + ".proc2(?)", [param1], function(err, result){
        if(err) console.log(err);
        conn.querySync("drop procedure " + schema + ".proc2");
        conn.closeSync();
        assert.equal(result, true);
        console.log('done');
    });
});

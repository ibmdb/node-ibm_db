// Test case to verify result of OUT and INOUT Parameters in a Strored Procedure.
// When SP is called using conn.query() assynchronously.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString;

var query = "CaLL newton.proc1(?, ?, ?)";
ibmdb.open(cn, function (err, conn) 
{
    if(err) return console.log(err);
    try {
          conn.querySync("drop procedure newton.proc1");
          console.log("newton.proc1 dropped.\n");
    } catch(e) {}

    conn.querySync("create procedure newton.proc1 " +
                   "(IN v1 INTEGER, OUT v2 INTEGER, INOUT v3 VARCHAR(20)) " +
                   "BEGIN set v2 = v1 + 1; set v3 = 'verygood'; END");
    console.log("created newton.proc1...\n");
    conn.commitTransactionSync();
    var param1 = {ParamType:"INPUT", DataType:1, Data:3};
    var param2 = {ParamType:"OUTPUT", DataType:1, Data:0};
    var param3 = {ParamType:"INOUT", DataType:1, Data:"abc", Length:30};

    conn.query(query, [param1, param2, param3], function(err, result){
        if(err) console.log(err);
        else {
            console.log("return value = ", result[0], result[1]);
        }
        conn.querySync("drop procedure newton.proc1");
        conn.closeSync();
        assert.deepEqual(result, [ 4, 'verygood' ]);
    });
    conn.querySync("create or replace procedure proc2 (IN v1 INTEGER) BEGIN END");
    conn.query("call proc2(?)", [param1], function(err, result){
        if(err) console.log(err);
        conn.querySync("drop procedure proc2");
        conn.closeSync();
        assert.equal(result, true);
        console.log('done');
    });
});


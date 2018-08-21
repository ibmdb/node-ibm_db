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
    var query = "CaLL " + schema + ".PROC1(?, ?, ?)";
    if(err) console.log(err);
    assert.equal(err, null);
    try {
      if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
        // The DROP PROCEDURE syntax on z/OS can only specify the name of the procedure to drop.
        conn.querySync("drop procedure " + schema + ".PROC1");
      } else {
        conn.querySync("drop procedure " + schema + ".PROC1(INT, INT, VARCHAR(20))");
      }
      console.log("PROC1 dropped.\n");
    } catch(e) { }

    if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
      // CREATE OR REPLACE syntax is not supported on z/OS.
      // When creating an SQL native procedure on z/OS, you will need to have a WLM environment
      // defined for your system if you want to run the procedure in debugging mode.
      // Adding "disable debug mode" to bypass this requirement.
      conn.querySync("create procedure " + schema + ".PROC1 " +
                     "(IN v1 INTEGER, OUT v2 INTEGER, INOUT v3 VARCHAR(20)) " +
                     "disable debug mode " +
                     "BEGIN set v2 = v1 + 1; set v3 = 'verygood'; END");
    } else {
      conn.querySync("create procedure " + schema + ".PROC1 " +
                     "(IN v1 INTEGER, OUT v2 INTEGER, INOUT v3 VARCHAR(20)) " +
                     "BEGIN set v2 = v1 + 1; set v3 = 'verygood'; END");
    }
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
        if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
          // The DROP PROCEDURE syntax on z/OS can only specify the name of the procedure to drop.
          conn.querySync("drop procedure " + schema + ".PROC1");
        } else {
          conn.querySync("drop procedure " + schema + ".PROC1(INT, INT, VARCHAR(20))");
        }
        assert.deepEqual(result, [ 4, 'verygood' ]);
    });
    if (process.env.IBM_DB_SERVER_TYPE === "ZOS") {
      // CREATE OR REPLACE syntax is not supported on z/OS.  Drop PROCEDURE first.
      // When creating an SQL native procedure on z/OS, you will need to have a WLM environment
      // defined for your system if you want to run the procedure in debugging mode.
      // Adding "disable debug mode" to bypass this requirement.
      try {
	conn.querySync("drop procedure " + schema + ".PROC2");
      } catch (e) { }
      conn.querySync("create procedure " + schema + ".PROC2 (IN v1 INTEGER) disable debug mode BEGIN END");
    } else {
      conn.querySync("create or replace procedure " + schema + ".PROC2 (IN v1 INTEGER) BEGIN END");
    }
    query = "call " + schema + ".PROC2(?)";
    conn.query({"sql":query, "params" : [param1]}, function(err, result){
        if(err) console.log(err);
        conn.querySync("drop procedure " + schema + ".PROC2(INT)");
        conn.closeSync();
        assert.equal(result.length, 0);
        console.log('done');
    });
});

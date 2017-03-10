var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , schema = common.connectionObject.CURRENTSCHEMA;

if(schema == undefined) schema = "NEWTON";

var proc1 = "create or replace procedure " + schema + ".proc2 ( IN v1 int, INOUT v2 varchar(30) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from " + schema + ".mytab1; declare cr2  cursor with return for select c2 from " + schema + ".mytab1; open cr1; open cr2; set v2 = 'success'; end";
var proc2 = "create or replace procedure " + schema + ".proc2 ( IN v1 int, INOUT v2 varchar(30) )  language sql begin  set v2 = 'success'; end";
var proc3 = "create or replace procedure " + schema + ".proc2 ( IN v1 int, IN v2 varchar(30) )  dynamic result sets 2 language sql begin  declare cr1  cursor with return for select c1, c2 from " + schema + ".mytab1; declare cr2  cursor with return for select c2 from " + schema + ".mytab1; open cr1; open cr2; end";
var query = "call " + schema + ".proc2(?, ?)";
var result;
ibmdb.open(common.connectionString, {fetchMode : 3}, function (err, conn) {
    if(err) {
      console.log(err);
      process.exit(-1);
    }
    try {
      conn.querySync({"sql":"create table " + schema + ".mytab1 (c1 int, c2 varchar(20))", "noResults":true});
    } catch (e) {};
    conn.querySync("insert into " + schema + ".mytab1 values (2, 'bimal'), (3, 'kumar')");
    param2 = {ParamType:"INOUT", DataType:1, Data:"abc", Length:50};

    // Create SP with INOUT param and 2 Result Set.
    conn.querySync(proc1);
    // Call SP Synchronously.
    result = conn.querySync(query, ['1', param2]);
    console.log("Result for Sync call of proc1 ==>");
    console.log(result);
    assert.equal(result.length, 3);
    // Call SP Asynchronously.
    conn.query(query, ['1', param2], function (err, result) {
        if (err) console.log(err);
        else {
          console.log("Result for Async call of proc1 ==>");
          console.log(result);
          assert.equal(result.length, 3);
        }

        // Create SP with only OUT param and no resultset.
        conn.querySync(proc2);
        // Call SP Synchronously.
        result = conn.querySync(query, ['1', param2]);
        console.log("Result for Sync call of proc2 ==>");
        console.log(result);
        assert.equal(result.length, 1);
        // Call SP Asynchronously.
        conn.query(query, ['1', param2], function (err, result) {
            if (err) console.log(err);
            else {
              console.log("Result for Async call of proc2 ==>");
              console.log(result);
              assert.equal(result.length, 1);
            }

            // Create SP with only Result Set and no OUT or INPUT param.
            conn.querySync(proc3);
            // Call SP Synchronously.
            result = conn.querySync(query, [1, 'abc']);
            console.log("Result for Sync call of proc3 ==>");
            console.log(result);
            assert.equal(result.length, 2);
            // Call SP Asynchronously.
            conn.query(query, ['1', 'abc'], function (err, result) {
                if (err) console.log(err);
                else {
                  console.log("Result for Async call of proc3 ==>");
                  console.log(result);
                  assert.equal(result.length, 2);
                }

                // Do Cleanup.
                conn.querySync("drop procedure " + schema + ".proc2 ( INT, VARCHAR(30) )");
                conn.querySync("drop table " + schema + ".mytab1");
                // Close connection in last only.
                conn.close(function (err) { console.log("done.");});
            });
        });
    });
});


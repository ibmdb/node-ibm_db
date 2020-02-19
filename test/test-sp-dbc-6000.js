// Test case to test the OUT parameters and Result set returned by 
// Stored Procedure when Async and Sync forms of Prepare and Execute
// APIs are used.

var ibmdb = require("../")
  , assert = require("assert")
  , common = require("./common")
  , connStr = common.connectionString;

//connStr = "database=RS22DC1A;hostname=rs22.rocketsoftware.com;port=3750;uid=" + common.connectionObject.UID + ";pwd=" + common.connectionObject.PWD;

if (process.env.IBM_DB_SERVER_TYPE !== "ZOS") {
    return;
}

var proc = "CREATE PROCEDURE SYSPROC.TBV8930 (in vlog_header varchar(200), out  return_code char(05), out sql_code integer, out result varchar(200)) language sql commit on return no query acceleration NONE asutime LIMIT 5002 reads sql data dynamic result sets 1 package owner DVLPP with explain MAIN: BEGIN declare sqlcode integer; declare sqlstate char(5); declare c1 cursor with return for select lower(vlog_header) as vlog_header from sysibm.sysdummy1; set result = upper(vlog_header) ; set return_code = '00000' , sql_code = 0 ; open c1; END MAIN"; 

var query = "call SYSPROC.TBV8930(?, ?, ?, ?)";
var result;
var dropProc = "drop procedure SYSPROC.TBV8930";

ibmdb.debug(true);
ibmdb.open(connStr, function (err, conn) {
    if(err) { 
      console.log(err);
      process.exit(-1);
    }
    param1 = {ParamType:"INPUT", DataType:1, Data:"aBcDeFgH"};
    param2 = {ParamType:"OUTPUT", DataType:1, Data:"", Length:5};
    param3 = {ParamType:"OUTPUT", Data:50};
    param4 = {ParamType:"OUTPUT", DataType:1, Data:"", Length:200};

    try {
       conn.querySync(dropProc);
    } catch(e) { };

    // Create SP
    conn.querySync(proc);

    // Call SP Synchronously.
    stmt = conn.prepareSync(query);
    //console.log(stmt.bindSync(['1', param2]));
    result = stmt.executeSync([param1, param2, param3, param4]);
    console.log("Result for Synchronous call of PROC ==>");
    if(Array.isArray(result))
    {
        // Print INOUT and OUT param values for SP.
        console.log("\tOut Params = ", result[1]);
        assert.deepEqual(result[1], [ '00000', 0, 'ABCDEFGH' ]);
        result = result[0];
    }
    data = result.fetchAllSync();
    console.log("\tResult Set = ", data);
    assert.equal(data.length, 1);
    while(result.moreResultsSync()) {
      data = result.fetchAllSync();
      console.log(data);
    }
    result.closeSync();
    stmt.closeSync();
    // Call SP Asynchronously.
    conn.prepare(query, function (err, stmt) {
      if (err) console.log(err);
      stmt.execute([param1, param2, param3, param4], function(err, result, outparams) {
        if( err ) console.log(err);  
        else {
          data = result.fetchAllSync();
          console.log("\nResult for Async call of PROC (3 OUT param and 1 Result Sets) ==>");
          console.log("\tOut Params = ", outparams);
          assert.deepEqual(outparams, [ '00000', 0, 'ABCDEFGH' ]);
          console.log("\tResult Set = ", data);
          assert.equal(data.length, 1);
          while(result.moreResultsSync()) {
            data = result.fetchAllSync();
            console.log(data);
          }
          result.closeSync();
        }
        stmt.closeSync();

        console.log("Dropping the procedure...");
        conn.querySync(dropProc);
        // Close connection in last only.
        conn.close(function (err) { console.log("done.");});
      });
    });
});


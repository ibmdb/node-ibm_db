var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;
var platform = require('os').platform();

ibmdb.open(cn, function(err, conn) { // 3 means FETCH_ARRARY
  if(err) console.log(err);
  assert.equal(err, null);

  var proc1 = "create or replace procedure PROC1 ( IN v1 decimal(11,0) ) language sql begin  insert into mytab1 values (7, 'rocket', v1); end";
  // Add proc2 to test the fix of issue #782 => Use integer as default value for decimal OUTPUT param
  var proc2 = "create or replace procedure PROC2 ( IN v1 decimal(15,3), OUT v2 decimal(15,3) ) language sql begin set v2 = v1 ; end";
  var dropProc = "drop procedure PROC1";
  var query = "CALL PROC1(?)";
  try{
    conn.querySync("drop table mytab1");
  } catch (e) {}
  try{
    conn.querySync(dropProc);
  } catch (e) {}
  try{
    conn.querySync("drop procedure PROC2");
  } catch (e) {}

  err = conn.querySync("create table mytab1 (c1 int, c2 varchar(10), c3 decimal(11, 0))");
  if(err.error) console.log(err);
  assert.equal(err.error, undefined);

  conn.querySync("insert into mytab1 values (5, 'rocket', 89874563789)");

  var stmt = conn.prepareSync("insert into mytab1 values ( ?, ?, ?)");
  var insertedRows = stmt.executeNonQuerySync([4, 'für', 2345.01]);
  console.log("insertedRows = ", insertedRows);
  stmt.closeSync();
  console.log("inserted data = ", conn.querySync("select * from mytab1"));
  var param1 = {ParamType:"INPUT", SQLType: "DECIMAL", Data: 45654378926};

  conn.prepare("insert into mytab1 VALUES (?, ?, ?)", function (err, stmt) {
    if (err) {
      console.log(err);
      stmt.closeSync();
      return conn.closeSync();
    }
    //Bind and Execute the statment asynchronously
    stmt.executeNonQuery([34245, 'IBM', param1], function (err, ret) {
      if (err) {
        console.log(err);
        stmt.closeSync();
      }
      else console.log("Inserted row count = " + ret);
      assert.equal(ret, 1);
      
      // Call SP Synchronously.
      var param2 = {ParamType:"INPUT", DataType: "DECIMAL", Data: 89.23365};
      var param3 = {ParamType: 'OUTPUT', CType: 8, DataType: 3, Data: 0, Length:15};
      conn.querySync(proc1);
      conn.querySync(proc2);
      result = conn.querySync(query, [param1]);
      console.log("Result for Sync call of proc1 ==>");
      console.log(result);
      result = conn.querySync("CALL PROC2(?, ?)", [param2, param3]);
      console.log("Result for Sync call of proc2 ==>");
      console.log(result);
      assert.deepEqual(result, [ 89.233 ]);
      // Call SP Asynchronously.
      conn.query(query, [param1], function (err, result) {
        if (err) console.log(err);
        else {
          console.log("Result for Async call of proc1 ==>");
          console.log(result);
        }
      
      conn.prepare("select * from mytab1", function (err, stmt) {
        if(err) {
          console.log(err);
          stmt.closeSync();
          return conn.closeSync();
        }
        stmt.execute([], function(err, result) {
          if (err) {
            console.log(err);
            stmt.closeSync();
          }
          else {
            data = result.fetchAllSync(); // Use fetchAllSync({fetchMode:3}) to get data as array.
            console.log("Column Names = ", result.getColumnNamesSync());
            console.log("Fetched Data = " );
            console.log(data);
            result.closeSync();
            stmt.closeSync();

            conn.querySync("drop table mytab1");
            conn.querySync(dropProc);
            testBigInt(conn);
            conn.close(function () { console.log('done'); });
            ibmdb.close();
          }
        });
      });  
    });
    });
  });
});

// Test for issue #816 - BigInt columns
function testBigInt(conn)
{
    console.log('\nTest BigInt value larger than MAX_SAFE_INTEGER value of JS');
    console.log('----------------------------------------------------------\n');
    console.log('JS Number max SAFE value : '+Number.MAX_SAFE_INTEGER);
    //9007199254740991 = Number.MAX_SAFE_INTEGER
    let maxSafeInt = Number.MAX_SAFE_INTEGER;
    let bigIntValue = 9007199254741997n;

    conn.querySync('create table mytab (c1 bigint, c2 varchar(20))');
    conn.querySync(`insert into mytab values (${maxSafeInt}, 'Max Safe Int'),(${bigIntValue}, 'Big Int Value')`);

    conn.queryResult("select * from mytab", (err, result) => {
        if(err) {
            console.log(err);
        } else {
            let data;
            while( data = result.fetchSync() )
            {
              console.log('Data Type returned : '+typeof data.C1);
              console.log(data);
              assert.deepEqual(typeof data.C1, 'string');
            }
            result.closeSync();
        }
        conn.querySync("drop table mytab;");
    });
}


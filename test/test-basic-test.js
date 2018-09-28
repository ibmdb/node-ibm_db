var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;

console.log("Trying to open a connection ... ");
ibmdb.open(cn, {"fetchMode": 3}, function(err, conn) { // 3 means FETCH_ARRARY
  if(err) console.log(err);
  assert.equal(err, null);
  console.log(" ... Got the connection.");

  try{
    conn.querySync("drop table mytab1");
  } catch (e) {}
  conn.querySync("create table mytab1 (c1 int, c2 varchar(10))");
  conn.querySync("insert into mytab1 values ( 4, 'für')");
  conn.query('select 1, 4, 5 from sysibm.sysdummy1;', function (err, data) {
    if (err) {
      console.log(err);
    } else {
      console.log(data);
      conn.query('select * from mytab1 where c1 = 2;', function (err, data) {
        if (err) {
          console.log(err);
        } else {
          console.log(data);
          conn.query('select 3,7,8 from sysibm.sysdummy1', [23], function (err, data) {
            if (err) {
              console.log(err);
            } else {
              console.log( data);
            }
          });
        }
  var d=conn.querySync("select * from mytab1 where (UCASE(C2) LIKE '%FUR%' OR UCASE(C2) LIKE '%FüR%') FOR READ ONLY WITH UR OPTIMIZE FOR 50 ROWS");
  console.log("Selected data with special character  = ", d);
  conn.prepare("insert into mytab1 VALUES (?, ?)", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }
    //Bind and Execute the statment asynchronously
    stmt.executeNonQuery([34245, 'bimal'], function (err, ret) {
      if( err ) console.log(err);  
      //else ret.closeSync(); // call closeSync() for execute().
      else console.log("Inserted row count = " + ret);
      assert.equal(ret, 1);
      
      conn.prepare("select * from mytab1", function (err, stmt) {
        if(err) {
          console.log(err);
          return conn.closeSync();
        }
        stmt.execute([], function(err, result) {
          if(err) console.log(err);
          else {
            data = result.fetchAllSync(); // Use fetchAllSync({fetchMode:3}) to get data as array.
            console.log("Column Names = ", result.getColumnNamesSync());
            console.log("Column Meta Data = ", result.getColumnMetadataSync());
            console.log("Fetched Data = " );
            console.log(data);
            result.closeSync();
            assert.deepEqual(data, [ { C1: 4, C2: 'für' }, { C1: 34245, C2: 'bimal' } ]);
            var valueArray = [];
            valueArray.push(34245);
            data = conn.querySync('select * from mytab1 where c1 = ?;', valueArray);
            console.log("conn.querySync('select * from mytab1 where c1 = ?;', valueArray)" );
            console.log(data);
            assert.deepEqual(data, [ [ 34245, 'bimal' ] ]);
            conn.querySync("drop table mytab1");
            conn.close(function () { console.log('done'); });
          }
        });
      });  
    });
  });
      });
    }
  });
});


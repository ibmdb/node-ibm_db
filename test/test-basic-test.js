var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;

ibmdb.open(cn, {"fetchMode": 3}, function(err, conn) { // 3 means FETCH_ARRARY
  if(err) console.log(err);
  assert.equal(err, null);

  try{
    conn.querySync("drop table mytab1");
    } catch (e) {}
  conn.querySync("create table mytab1 (c1 int, c2 varchar(10))");
  conn.query('select 1, 4, 5 from sysibm.sysdummy1;' +
             'select * from mytab1 where c1 = 2;'+
             'select 3,7,8 from sysibm.sysdummy1', [23], function (err, data) {
    if (err) console.log(err);
    else {
      console.log( data);
    }
  });
  conn.prepare("insert into mytab1 VALUES (?, ?)", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }
    //Bind and Execute the statment asynchronously
    stmt.executeNonQuery([42, 'bimal'], function (err, ret) {
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
            console.log("Fetched Data = " );
            console.log(data);
            result.closeSync();
            conn.querySync("drop table mytab1");
            assert.deepEqual(data, [ { C1: 42, C2: 'bimal' } ]);
            conn.close(function () { console.log('done'); });
          }
        });
      });  
    });
  });
});


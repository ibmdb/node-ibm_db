var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;

ibmdb.open(cn, function(err, conn) {
  if(err) return console.log(err);

  try{
    conn.querySync("drop table mytab1");
    } catch (e) {}
  conn.querySync("create table mytab1 (c1 int, c2 varchar(10))");
  conn.query('select 1 from sysibm.sysdummy1', [23], function (err, data) {
    if (err) console.log(err);
    else {
      console.log( data);
      assert(data, [ { '1': 1 } ]);
    }
  });
  conn.prepare("insert into mytab1 VALUES (?, ?)", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }
    //Bind and Execute the statment asynchronously
    stmt.execute([42, 'bimal'], function (err, result) {
      if( err ) console.log(err);  
      else result.closeSync();
      
      conn.prepare("select * from mytab1", function (err, stmt) {
        if(err) {
          console.log(err);
          return conn.closeSync();
        }
        stmt.execute([], function(err, result) {
          if(err) console.log(err);
          else {
            data = result.fetchAllSync();
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


var common = require("./common")
    , ibmdb = require("../")
    , db = new ibmdb.ODBC()
    , assert = require("assert")
    , connStr = common.connectionString;

var sql = "insert into testtab (col1,col2) values('一二三四', '五六七八')"; 

ibmdb.open(connStr,(err,conn)=>{
  if(err){ console.log(err); return; }
  try {
  conn.querySync('create table testtab (col1 VARCHAR(100), col2 VARCHAR(100))');
  } catch(err) {console.log(err);}

  // Insert Chinese characters in table using all possible ways.
  conn.querySync(sql);  // Insert Row1
  var stmt = conn.prepareSync(sql);
  stmt.executeSync();   // Insert Row2
  stmt.closeSync();
  conn.query(sql, function(err, data) { // Insert Row3
    if(err){ console.log(err); return; }
    conn.prepare(sql, function(err, stmt) {
     if(err){ console.log(err); return; }
     stmt.execute(function(err, data) { // Insert Row4
      if(err){ console.log(err); return; }
      stmt.closeSync();

      // Create another connection using alternate way.
      var conn2 = db.createConnectionSync();
      conn2.openSync(connStr);
      stmt = conn2.createStatementSync();
      stmt.executeDirectSync(sql);   // Insert Row5
      stmt.prepareSync(sql);
      console.log("Inserted row = ",stmt.executeNonQuerySync()); // Insert Row6
      stmt.closeSync();
      conn2.createStatement(function(err, stmt){
        if(err){ console.log(err); return; }
        stmt.executeDirect(sql, function(err, result){ // Insert Row7
          if(err){ console.log(err); return; }
          stmt.prepare(sql, function(err, result) { //dont use err, stmt here
            stmt.executeNonQuery(function(err, rowCount){ // Insert Row8
              if(err){ console.log(err); return; }
              console.log("Inserted rowcount = ", rowCount);
              stmt.closeSync();
              var data = conn.querySync("select * from testtab");
              console.log("Inserted rows = ", data);
              conn2.querySync("drop table testtab");
              conn2.closeSync();
              conn.closeSync();
              assert(data.length, 8);
            });
          });
        });
      });
     });
    });
  });
});


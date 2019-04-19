var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;

ibmdb.open(cn, function(err, conn) {
  if(err) console.log(err);

  try{
    conn.querySync("drop table mytab1");
  } catch (e) {}
  conn.querySync("create table mytab1 (c1 int, c2 varchar(10))");
  conn.querySync("insert into mytab1 values ( 4, 'bimal')");
  conn.prepare("insert into mytab1 VALUES (?, ?)", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      return conn.closeSync();
    }
    //Bind and Execute the statment asynchronously
    var result = stmt.executeSync([5, 'kumar']);
    console.log("No of Inserted rows = ", result.getAffectedRowsSync());
    result.closeSync();
    stmt.closeSync();
    conn.prepare("update mytab1 set c2 = 'jha' where c1 > 3", 
      function(err, stmt) {
        if(err) {
          console.log(err);
          return conn.closeSync();
        }
        result = stmt.executeSync();
        console.log("No of updated rows = ", result.getAffectedRowsSync());
        result.closeSync();
        stmt.closeSync();
        conn.prepare("delete from mytab1 where c1 = ?", function(err, stmt) {
            if(err) {
              console.log(err);
              return conn.closeSync();
            }
            stmt.execute([4], function(err, result) {
              if(err) { console.log(err); }
              else {
                console.log("No of deleted rows = ", 
                    result.getAffectedRowsSync());
                result.closeSync();
              }
              stmt.closeSync();
              conn.querySync("drop table mytab1");
              conn.closeSync();
            });
        });
    });
  });
});


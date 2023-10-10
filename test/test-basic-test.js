var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;
var platform = require('os').platform();

console.log("Trying to open a connection ... ");
console.log("Connection String =", cn);
ibmdb.open(cn, {"fetchMode": 3}, function(err, conn) { // 3 means FETCH_ARRARY
  if(err) console.log(err);
  assert.equal(err, null);
  console.log(" ... Got the connection.");

  try{
    conn.querySync("drop table mytab1");
  } catch (e) {}
  if (platform == 'os390') {
    conn.querySync("create table mytab1 (c1 int, c2 varchar(10)), c3 blob(100) ccsid UNICODE");
  } else {
    conn.querySync("create table mytab1 (c1 int, c2 varchar(10), c3 blob(100))");
  }
  var stmt = conn.prepareSync("insert into mytab1 values ( ?, ?, ?)");
  var param = { CType: 'BLOB', DataType: 'BLOB', Data: null };
  var insertedRows = stmt.executeNonQuerySync([4, 'für', param]);
  console.log("insertedRows = ", insertedRows);
  stmt.closeSync();

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
  var d=conn.querySync("select * from mytab1 where (UCASE(C2) LIKE '%FÜR%' OR UCASE(C2) LIKE '%FüR%') FOR READ ONLY WITH UR OPTIMIZE FOR 50 ROWS");
  console.log("Selected data with special character  = ", d);
  assert.equal(d.length, 1);

  conn.prepare("insert into mytab1 VALUES (?, ?, ?)", function (err, stmt) {
    if (err) {
      //could not prepare for some reason
      console.log(err);
      stmt.closeSync();
      return conn.closeSync();
    }
    //Bind and Execute the statment asynchronously
    stmt.executeNonQuery([34245, 'bimal', param], function (err, ret) {
      if (err) {
        console.log(err);
        console.log(err.sqlcode);
        stmt.closeSync();
      }
      //else ret.closeSync(); // call closeSync() for execute().
      else console.log("Inserted row count = " + ret);
      assert.equal(ret, 1);
      
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
            console.log("Column Meta Data = ", result.getColumnMetadataSync());
            console.log("Fetched Data = " );
            console.log(data);
            result.closeSync();
            stmt.closeSync();
            assert.deepEqual(data, [ { C1: 4, C2: 'für', C3: null }, { C1: 34245, C2: 'bimal', C3: null } ]);
            // Insert null value in BLOB colum C3, using querySync API
            conn.querySync("insert into mytab1 VALUES (?, ?, ?)", [345, 'bimal', param]);
            var valueArray = [];
            valueArray.push(345);
            data = conn.querySync('select * from mytab1 where c1 = ?;', valueArray);
            console.log("conn.querySync('select * from mytab1 where c1 = ?;', valueArray)" );
            console.log(data);
            assert.deepEqual(data, [ [ 345, 'bimal', null ] ]);

            /* Check to ensure query ignores sqlcode 100, issue #573 */
            conn.query("UPDATE mytab1 set c1 = 5 where c1 = 89; select 1 " +
                    "from sysibm.sysdummy1;", function(error, result, sqlca) {
              if(error) console.log("Error ====>> ", error);
              else {
                console.log("Result of UPDATE Query = ", result);
              }
              console.log("SQLCA = ", sqlca);
              assert.equal(error, null);

            /* Check for empty result set */
            console.log("conn.prepare('delete from mytab1 where c1 = 89')");
            conn.prepare("delete from mytab1 where c1 = 89",
              function (err, stmt2) {
              if(err) {
                console.log(err);
                stmt2.closeSync();
                return conn.closeSync();
              }
              stmt2.executeNonQuery(function(err, rows) {
                if (err) {
                  console.log(err);
                  assert.equal(rows, 1); //It should always assert.
                }
                console.log("Deleted row count = " + rows);
                stmt2.closeSync();
                conn.querySync("drop table mytab1");
                conn.close(function () { console.log('done'); });
                ibmdb.close();
              });
            });  // conn.prepare
            });  // conn.query
          }
        });
      });  
    });
  });
      });
    }
  });
});


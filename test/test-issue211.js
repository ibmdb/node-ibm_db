var ibmdb = require("../")
    , common = require("./common")
    , assert = require("assert")
    , cn = common.connectionString;

var stmt0 = {};
var stmt1 = {};
var stmt2 = {};
var queryExecuted = 0;

stmt1.sql = 'SELECT COUNT(C1) FROM FINAL TABLE(UPDATE T1 SET C2=?, C3=? WHERE C1=1)';
stmt1.params = ['Joe', 'Tim'];
stmt1.noResults = false;

stmt2.sql = 'SELECT COUNT(C1) FROM FINAL TABLE(UPDATE T1 SET C2=?, C3=? WHERE C1=1)';
stmt2.params = ['Jane', 'Joan'];
stmt2.noResults = false;

ibmdb.debug(true);
ibmdb.open(cn, function(err, conn1) {
  if (err) console.log(err);
  assert.equal(err, null);

  stmt0.sql = 'DROP TABLE T1';
  stmt0.noResults = true;
  try{ conn1.querySync(stmt0.sql); } catch(e) { }

  stmt0.sql = 'CREATE TABLE T1 (C1 INT PRIMARY KEY NOT NULL, C2 CHAR(50), C3 CHAR(50))';
  conn1.querySync(stmt0.sql);

  stmt0.sql = "INSERT INTO T1 VALUES (1, 'ABCDEF', 'GHIJK')";
  conn1.querySync(stmt0.sql);
  
  ibmdb.open(cn, function(err, conn2) {
      if (err) return console.log(err);

      conn1.beginTransaction(function(err) {
          if (err) return console.log(err);
          console.log('Transaction 1 opened');
          conn2.beginTransaction(function(err) {
              if (err) return console.log(err);
              console.log('Transaction 2 opened');

              conn1.query(stmt1, function(err, data) {
                console.log('Query 1 executed');
                if (err) {
                  conn1.rollbackTransactionSync();
                  console.log(err);
                } else {
                  conn1.commitTransactionSync();
                  console.log('<<< DATA >>>:', data);
                }
                queryExecuted++;
              });

              conn2.query(stmt2, function(err, data) {
                console.log('Query 2 executed');
                if (err) {
                  conn2.rollbackTransactionSync();
                  console.log(err);
                } else {
                  conn2.commitTransactionSync();
                  console.log('<<< DATA >>>:', data);
                  assert.deepEqual(data, [ { '1': 1 } ]);
                }
                conn2.closeSync();
                queryExecuted++;
              });
          });
      });
  });
  var interval = setInterval(function(){
                   if(queryExecuted === 2) {
                     conn1.querySync("drop table T1");
                     conn1.closeSync();
                     clearInterval(interval);
                   }
                 }, 1000);
});


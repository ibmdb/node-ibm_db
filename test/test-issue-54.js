//NOTE: this does not assert anything that it should, please fix.

var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  , util = require('util')
  , count = 0
  ;
 
var sql =
"declare @t table (x int); \
insert @t values (1); \
select 'You will get this message' \
raiserror('You will never get this error!', 16, 100); \
raiserror('Two errors in a row! WHAT?', 16, 100); \
select 'You will never get this message, either!' as msg; \
"

db.open(common.connectionString, function(err) {
  console.log(err || "Connected");
  assert.equal(err, null);

  if (!err) {
    db.query(sql, function (err, results, more) {
      console.log("q1 result: ", err, results, more);

      if (!more) {
        console.log("Running second query");

        db.query("select 1 as x", function(err, results, more) {
          console.log("q2 result: ", err, results, more);

          db.close(function(err) { console.log(err || "Closed"); })
        })
      }
    })
  }
});

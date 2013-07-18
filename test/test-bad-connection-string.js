var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

assert.throws(function () {
  db.openSync("this is wrong");
});

assert.equal(db.connected, false);
  
db.open("this is wrong", function(err) {
  console.log(err);
  
  assert.deepEqual(err, {
    error: '[node-odbc] SQL_ERROR',
    message: '[unixODBC][Driver Manager]Data source name not found, and no default driver specified',
    state: 'IM002'
  });
  
  assert.equal(db.connected, false);
});

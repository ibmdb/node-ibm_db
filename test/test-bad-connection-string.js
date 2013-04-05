var common = require("./common")
  , odbc = require("../odbc.js")
  , db = new odbc.Database()
  , assert = require("assert")
  ;


db.open("this is wrong", function(err) {
  assert.deepEqual(err, {
    error: '[node-odbc] SQL_ERROR',
    message: '[unixODBC][Driver Manager]Data source name not found, and no default driver specified',
    state: 'IM002'
  });
  
  assert.equal(db.connected, false);
});

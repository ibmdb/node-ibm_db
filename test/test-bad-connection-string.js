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
  
  if( /^win/.test(process.platform) )
    assert.deepEqual(err.message, '[IBM][CLI Driver] SQL1024N  A database connection does not exist.  SQLSTATE=08003\r\n');
  else
    assert.deepEqual(err.message, '[IBM][CLI Driver] SQL1024N  A database connection does not exist.  SQLSTATE=08003\n');
  
  assert.equal(db.connected, false);
});

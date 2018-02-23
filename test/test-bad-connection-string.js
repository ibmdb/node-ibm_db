var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

const os = require("os");

assert.throws(function () {
  db.openSync("this is wrong");
});

assert.equal(db.connected, false);
  
db.open("this is wrong", function(err) {
  console.log(err);
  
  if( /^win/.test(process.platform) ) {
    assert.deepEqual(err.message, '[IBM][CLI Driver] SQL1024N  A database connection does not exist.  SQLSTATE=08003\r\n');
  } else if (os.type() === "OS/390") {
    // Expected error message on z/OS is SQLCODE = -950 SQLSTATE = 42705
    // ERROR:  THE LOCATION NAME SPECIFIED IN THE CONNECT STATEMENT IS INVALID OR NOT LISTED IN THE COMMUNICATIONS DATABASE
    assert(/SQLCODE\s*=\s*-950/.test(err.message));
    assert.equal(err.state, 42705);
  } else {
    assert.deepEqual(err.message, '[IBM][CLI Driver] SQL1024N  A database connection does not exist.  SQLSTATE=08003\n');
  }
  
  assert.equal(db.connected, false);
});

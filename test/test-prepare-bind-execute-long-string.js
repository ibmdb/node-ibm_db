var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.openSync(common.connectionString);
issueQuery();

function issueQuery() {
  var count = 0
    , time = new Date().getTime()
    , stmt
    , result
    , data
    , str = ''
    ;
  
  var set = 'abcdefghijklmnopqrstuvwxyz';
  
  for (var x = 0; x < 1000001; x++) {
    str += set[x % set.length];
  }
  
  assert.doesNotThrow(function () {
    stmt = db.prepareSync('select ? as longString');
  });
  
  assert.doesNotThrow(function () {
    stmt.bindSync([str]);
  });
  
  assert.doesNotThrow(function () {
    result = stmt.executeSync();
  });
  
  assert.doesNotThrow(function () {
    data = result.fetchAllSync();
  });
  
//   console.log(data);
  console.log(str.length);
  console.log(data[0].longString.length);
  
  for (var x = 0; x < str.length; x++) {
    if (str[x] != data[0].longString[x]) {
      console.log(x, str[x], data[0].longString[x]);
      
      assert.equal(str[x], data[0].longString[x]);
    }
  }
  
  assert.equal(data[0].longString, str);
  
  finish(0);
}

function finish(exitCode) {
  db.closeSync();
  
  console.log("connection closed");
  process.exit(exitCode || 0);
}

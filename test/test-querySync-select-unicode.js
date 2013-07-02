var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  var data;
  try {
    data = db.querySync("select 'ꜨꜢ' as UNICODETEXT");
  }
  catch (e) {
   console.log(e); 
  }
  
  db.close(function () {
    console.log(data);
    assert.deepEqual(data, [{ UNICODETEXT: 'ꜨꜢ' }]);
  });
});

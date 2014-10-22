var common = require("./common")
  , odbc = require("../")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  assert.equal(db.connected, true);
  
  var dt = new Date();
  var dtString = dt.toISOString().replace('Z','').replace('T', ' ') + '000';
  var sql = "SELECT cast('" + dtString + "' as timestamp) as DT1 FROM SYSIBM.SYSDUMMY1";
  
  console.log(sql);
  
  db.query(sql, function (err, data) {
    assert.equal(err, null);
    assert.equal(data.length, 1);

    db.close(function () {
      assert.equal(db.connected, false);
      console.log(dt);
      console.log(data);
      
      //test selected data after the connection
      //is closed, in case the assertion fails
	  assert.equal(data[0].DT1.constructor.name, "String", "DT1 is not an instance of a String object");
      //assert.equal(data[0].DT1.getTime(), dt.getTime());
    });
  });
});

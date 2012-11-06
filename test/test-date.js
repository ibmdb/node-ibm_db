var common = require("./common")
  , odbc = require("../odbc.js")
  , db = new odbc.Database()
  , assert = require("assert")
  ;

db.open(common.connectionString, function(err) {
  assert.equal(err, null);
  assert.equal(db.connected, true);
  
  var dt = new Date();
  
  db.query("SELECT cast('" + dt.toISOString().replace('Z','') + "' as datetime) as DT1", function (err, data) {
    assert.equal(err, null);
    assert.equal(data.length, 1);

    db.close(function () {
      assert.equal(db.connected, false);
      
      //test selected data after the connection
      //is closed, in case the assertion fails
      assert.equal(data[0].DT1.constructor.name, "Date", "DT1 is not an instance of a Date object");
      assert.equal(data[0].DT1.getTime(), dt.getTime(), "The original time passed to the database is not the same as the time we got back");
    });
  });
});

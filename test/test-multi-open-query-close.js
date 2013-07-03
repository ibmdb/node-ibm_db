var common = require("./common")
, odbc = require("../")
, openCallback = 0
, closeCallback = 0
, queryCallback = 0
, openCount = 3
, connections = []
;

for (var x = 0; x < openCount; x++ ) {
  (function (x) {
    var db = new odbc.Database();
    connections.push(db);
    
    db.open(common.connectionString, function(err) {
      if (err) {
        throw err;
        process.exit();
      }
      
      //console.error("Open: %s %s %s", x, openCount, openCallback);
    
      openCallback += 1;
      
      maybeQuery();
    });
  })(x);
}

function maybeQuery() {
  if (openCount == openCallback) {
    doQuery();
  }
}

function doQuery() {
  connections.forEach(function (db, ix) {
    var seconds = connections.length - ix;
    
    var query = "WAITFOR DELAY '00:00:0" + seconds + "'; select " + seconds + " as result";
    
    db.query(query, function (err, rows, moreResultSets) {
             
      //console.error("Query: %s %s %s %s", ix, openCount, queryCallback, moreResultSets, rows, err);
    
      queryCallback += 1;
      
      maybeClose();
    });
  });
}

function maybeClose() {
  if (openCount == queryCallback) {
    doClose();
  }
}

function doClose() {
  connections.forEach(function (db, ix) {
    db.close(function () {
      //console.log("Close: %s %s %s", ix, openCount, closeCallback);
    
      closeCallback += 1;
      
      maybeFinish();
    });
  });
}

function maybeFinish() {
  if (openCount == closeCallback) {
    console.error('done');
  }
}

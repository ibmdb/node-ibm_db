var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;

ibmdb.open(cn, function (err,conn) {
    if (err) return console.log(err);
    var query = 'select creator, name from sysibm.systables where 1 = ?';
    var params = [1];
    conn.queryResult(query, params, function (err, data) {
        if(err) {
          console.log(err);
        }
        else {
           console.log("data = ", data.fetchAllSync());
           console.log("sqlerror = ", data.getSQLErrorSync());
           console.log("metadata = ", data.getColumnMetadataSync());
           data.closeSync();
           conn.close(function(){});
        }
    });
});

ibmdb.open(cn, function (err,conn) {
    if (err) return console.log(err);
    var query = 'select creator, name from sysibm.systables';
    var result = conn.queryResultSync(query);
    console.log("result = ", result);
    console.log("data = ", result.fetchAllSync());
    console.log("sqlerror = ", result.getSQLErrorSync());
    console.log("metadata = ", result.getColumnMetadataSync());
    result.closeSync();
    conn.closeSync();
});


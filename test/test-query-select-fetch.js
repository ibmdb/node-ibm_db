var common = require("./common")
    , odbc = require("../odbc.js")
    , db = new odbc.Database();

db.open(common.connectionString, function(err)
{
    db.queryResult('select \'a\' as test, datetime(\'now\') as nowish', function (err, result) {
        if (err) {
            console.error(err);
            process.exit(1);
        }
        
        result.fetch(function (err, data) {
          console.error(data);
          db.close(function () {});
        });
    });
});

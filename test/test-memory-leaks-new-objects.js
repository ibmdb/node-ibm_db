var odbc = require("../")
    , openCount = 100
    , start = process.memoryUsage().heapUsed
    , x = 100
    ;

gc();

start = process.memoryUsage().heapUsed;

for (x = 0; x < openCount; x++ ) {
    (function () {
        var db = new odbc.Database();
    })();
}

gc();

console.log(process.memoryUsage().heapUsed - start);

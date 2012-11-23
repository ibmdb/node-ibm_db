var odbc = require("../")
    , openCount = 100
    , start = process.memoryUsage()
    , stop = process.memoryUsage()
    , x = 100
    ;

gc();

start = process.memoryUsage();

for (x = 0; x < openCount; x++ ) {
    (function () {
        var db = new odbc.Database();
    })();
}

gc();

stop = process.memoryUsage();

console.log(start.heapUsed, stop.heapUsed);
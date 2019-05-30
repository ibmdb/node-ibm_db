var common = require("./common")
    , odbc = require("../")
    , pool = new odbc.Pool()
    , connectionString = common.connectionString
    , assert = require("assert")
    ;

var i=0;
console.log("---------------------------------------------------------------");
console.log("After first iteration and before start of second iteration,");
console.log("restart the server using 'db2stop force; db2start' command.");
console.log("---------------------------------------------------------------");
//odbc.debug(true);
pool.setMaxPoolSize(5);
var timer = setInterval(function() {
    var j = i;
    console.log('start'+j);
    if(i==3){
        pool.close();
        clearInterval(timer);
    }
    pool.open(connectionString, function (err, connection) {
        if (err) {
            console.log("Connection " + err.toString());
        }

        connection.query("select 1 from sysibm.sysdummy1", 
          function (err, results) {
            if (err) {
               console.log(err.toString());
               assert.equal(err['message'].search("SQL30081N"),18);
            }else{
                console.log(JSON.stringify(results));
                assert.equal(JSON.stringify(results), '[{"1":1}]');
            }
            connection.close(function () {
              console.log('Connection closed');
              console.log('done'+j);
              if(j == 0) {
                console.log("<=== Now restart the server using " + 
                            "'db2stop force; db2start' command. ====>");
              }
            });
        });
    });
    i++;
}, 3000);  // Change it to 30000 when running single test file for actual test.

pool.init(4, connectionString);

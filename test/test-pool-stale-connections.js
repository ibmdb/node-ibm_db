var common = require("./common")
    , ibmdb = require("../")
    , pool = new ibmdb.Pool()
    , connectionString = common.connectionString
    , assert = require("assert")
    , connectCount = 10
    ;

var i=0;
var timeout = 2000; // Change it to 20000 when running single test file for actual testing.
console.log("---------------------------------------------------------------");
console.log("After first iteration and before start of second iteration,");
console.log("restart the server using 'db2stop force; db2start' command.");
console.log("---------------------------------------------------------------");
//ibmdb.debug(true);
pool.setMaxPoolSize(connectCount);

var pool2 = new ibmdb.Pool();
var timer2;
var timer = setInterval(function() {
    var j = i;
    console.log('start'+j);
    if(i==3){
        pool2.close();
        clearInterval(timer);
    }
    pool2.open(connectionString, function (err, connection) {
        if (err) {
            console.log("Connection " + err.toString());
        }

        connection.beginTransactionSync();
        connection.query("select 1 from sysibm.sysdummy1", 
          function (err, results) {
            if (err) {
               console.log(err.toString());
               assert.equal(err['message'].search("SQL30081N"),18);
            }else{
                console.log(JSON.stringify(results));
                assert.equal(JSON.stringify(results), '[{"1":1}]');
                connection.commitTransactionSync();
            }
            connection.close(function () {
              console.log('Connection closed');
              console.log('done'+j);
              if(j == 0) {
                console.log("<=== Now restart the server using " + 
                            "'db2stop force; db2start' command. ====>");
              }
              if( j == 3 ) openMultipleConnections();
            });
        });
    });
    i++;
}, timeout); // Change timeout to 20000 for actual test.

function openMultipleConnections()
{
  console.log("\n Test 1:\n", "=======");
  console.log("Open", connectCount, "connections from pool.\n");
  var query = "select 'abc' as col from sysibm.sysdummy1";
  for (var x = 1; x <= connectCount; x++)
  {
    (function (connectionIndex) {
      console.error("Opening connection #", connectionIndex);
      pool.open(connectionString, function (err, connection) {
        if (err) {
          console.error("error: ", err.message);
          assert.equal(err.message, null);
        }

        console.log("Data for query" + connectionIndex + " = ",
            connection.querySync(query));
        connection.closeSync();

        if (connectionIndex == connectCount) {
          testStaleConnection();
        }
      });
    })(x);
  }
}

function testStaleConnection()
{
  console.log("\n Test 2:\n", "=======");
  console.log("Test Stale Connections...\n");
  console.log("<=== Restart the server using " + 
              "'db2stop force; db2start' command. ====>\n");
  var query = "select 1 from sysibm.sysdummy1";
  var result;
  setTimeout( function() {
    for (var x = 1; x <= connectCount; x++)
    {
        var connection = pool.openSync(connectionString);
        console.error("Got connection #", x, "from pool.");

        result = connection.querySync(query);
        if(result['message']) {
             console.log("error = ", result['message']);
             assert.equal(result['sqlcode'], -30081);
        } else {
          console.log("Data", x, "= ", JSON.stringify(result));
          assert.equal(JSON.stringify(result), '[{"1":1}]');
        }
        connection.closeSync();
        console.log('Connection', x, 'closed');
    }
    pool.closeSync();
  }, timeout); // Change timeout to 20000 for actual test.
}

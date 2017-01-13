var fs = require("fs")
  , common = require('./common.js')
  , spawn = require("child_process").spawn
  , path = require("path")
  , errorCount = 0
  , testCount = 0
  , testTimeout = 600000  //Let it be 10 min.
  , requestedTest = null
  , files
  , moment = require('moment')
  ;

// Check for async module, which is required to run nodeEE test files.
// Do not add async and moment in package.json as it is not required during installation.
// We do not want installation to be dependent on async and moment. Else, if async
// or moment fails to install, installation of ibm_db too will fail.
// Install async and moment only if it is required.
if(fs.accessSync(path.resolve("../node_modules/async")))
{
    console.log("Module async is requied to run nodeEE test cases. Please " +
                "execute 'npm install async' before running nodeEE test files.");
    return;
}

if (process.argv.length === 3) {
  requestedTest = process.argv[2];
}

var startTime = moment();
var connectionStrings = common.testConnectionStrings;

//check to see if the requested test is actually a driver to test
if (requestedTest) {
  connectionStrings.forEach(function (connectionString) {
    if (requestedTest == connectionString.title) {
      connectionStrings = [connectionString];
      requestedTest = null;
    }
  });
}

doNextConnectionString();


function doTest(file, connectionString) {
  fs.open('logs/' + file + '.log', 'wx', function(err, fd) {
    var test = spawn("node", ['--expose_gc',file, connectionString.connectionString], {stdio: ['pipe', fd, fd]})
      , timer = null
      , timedOut = false;
      ;
    
    process.stdout.write("Running test for [\033[01;29m" + connectionString.title + "\033[01;0m] : " + file.replace(/\.js$/, ""));
    process.stdout.write(" ... ");

    testCount += 1;
    
    test.on("exit", function (code, signal) {
      clearTimeout(timer);
      
      if (code != 0) {
        errorCount += 1;
        
        process.stdout.write("\033[01;31mfail \033[01;0m ");
        
        if (timedOut) {
          process.stdout.write("(Timed Out)");
        }
      }
      else {
        process.stdout.write("\033[01;32msuccess \033[01;0m ");
      }
      
      process.stdout.write("\n");
      
      doNextTest(connectionString);
    });
    
    var timer = setTimeout(function () {
      timedOut = true;
      test.kill();
    },testTimeout);
  });
}

function doNextTest(connectionString) {
  if (files.length) {
    var testFile = files.shift();
    
    doTest(testFile, connectionString);
  }
  else {
    //we're done with this connection string, display results and exit accordingly
    doNextConnectionString();
  }
}

function doNextConnectionString() {
  if (connectionStrings.length) {
    var connectionString = connectionStrings.shift();
    
    if (requestedTest) {
      files = [requestedTest];
    }
    else {
      //re-read files
      files = fs.readdirSync("./");

      files = files.filter(function (file) {
        return (/^nodeEE-/.test(file)) ? true : false;
      });

      files.sort();
    }
    
    doNextTest(connectionString);
  }
  else {
    var totalTime = (moment.duration(moment() - startTime))/1000;
    if (errorCount) {
      console.log("\nResults : %s of %s tests failed.\n", errorCount, testCount);
    }
    else {
      console.log("Results : All tests were successful. Total %s tests executed.", testCount);
    }
    console.log("Total execution time = %s min %s sec.", 
                parseInt(totalTime/60), parseInt(totalTime%60));

    if (errorCount) {
      process.exit(errorCount);
    }
  }
}

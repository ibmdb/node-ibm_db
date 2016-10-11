var fs = require("fs")
  , common = require('./common.js')
  , spawn = require("child_process").spawn
  , errorCount = 0
  , testCount = 0
  , testTimeout = 300000  //Let it be 5 min.
  , requestedTest = null
  , files
  , moment = require('moment')
  ;

var filesDisabled = fs.readdirSync("./disabled");

if (filesDisabled.length) {
  console.log("\n\033[01;31mWarning\033[01;0m : there are %s disabled tests\n", filesDisabled.length);
}

if (process.argv.length === 3) {
  requestedTest = process.argv[2];
}

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

var startTime = moment();

doNextConnectionString();


function doTest(file, connectionString) {
  var test = spawn("node", ['--expose_gc',file, connectionString.connectionString])
    , timer = null
    , timedOut = false;
    ;
  var testOut = '', testErr = '';

  test.stdout.on('data', function(data) {
    testOut += data.toString();
  });

  test.stderr.on('data', function(data) {
    testErr += data.toString();
  });

  process.stdout.write("Running test for [\033[01;29m" + connectionString.title + "\033[01;0m] : " + file.replace(/\.js$/, ""));
  process.stdout.write(" ... ");

  testCount += 1;

  test.on("exit", function (code, signal) {
    clearTimeout(timer);

    if (code && code != 0) {
      errorCount += 1;

      process.stdout.write("\033[01;31mfail \033[01;0m ");

      if (timedOut) {
        process.stdout.write("(Timed Out)");
      }
      process.stdout.write("\n \033[01;34mStdout: \033[01;0m \n");
      process.stdout.write(testOut);
      process.stdout.write("\n \033[01;31mStderr: \033[01;0m \n");
      process.stdout.write(testErr);
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
        return (/^test-/.test(file)) ? true : false;
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
      console.log("Results : All tests were successful. Total %s files executed.", testCount);
    }
    console.log("Total execution time = %s min %s sec.",
                parseInt(totalTime/60), parseInt(totalTime%60));
    if (errorCount) {
      process.exit(errorCount);
    }
  }
}

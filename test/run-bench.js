var fs = require("fs")
  , common = require('./common.js')
  , spawn = require("child_process").spawn
  , requestedBench = null
  , errorCount = 0
  , testCount = 0
  , files
  ;

if (process.argv.length === 3) {
  requestedBench = process.argv[2];
}

var connectionStrings = common.benchConnectionStrings;

//check to see if the requested test is actually a driver to benchmark
if (requestedBench) {
  connectionStrings.forEach(function (connectionString) {
    if (requestedBench == connectionString.title) {
      connectionStrings = [connectionString];
      requestedBench = null;
    }
  });
}

doNextConnectionString();

function doBench(file, connectionString) {
  var bench = spawn("node", ['--expose_gc',file, connectionString.connectionString]);
  
  process.stdout.write("Running test for [\033[01;29m" + connectionString.title + "\033[01;0m] : \033[01;33m" + 
                       file.replace(/\.js$/, "") + "\033[01;0m");
  process.stdout.write(" ... ");

  testCount += 1;
  
  bench.on("exit", function (code, signal) {
    process.stdout.write(" ... ");
    if (code != 0) {
        errorCount += 1;
        process.stdout.write("\033[01;31mfail \033[01;0m ");
    }
    else {
        process.stdout.write("\033[01;32msuccess \033[01;0m ");
    }
    process.stdout.write("\n");

    doNextBench(connectionString);
  });
  
  bench.stdout.on("data", function (data) {
    process.stdout.write(data);
  });
}

function doNextBench(connectionString) {
  if (files.length) {
    var benchFile = files.shift();
    
    doBench(benchFile, connectionString);
  }
  else {
    //we're done with this connection string, display results and exit accordingly
    doNextConnectionString();
  }
}

function doNextConnectionString() {
  if (connectionStrings.length) {
    var connectionString = connectionStrings.shift();
    
    if (requestedBench) {
      files = [requestedBench];
    }
    else {
      //re-read files
      files = fs.readdirSync("./");

      files = files.filter(function (file) {
        return (/^bench-/.test(file)) ? true : false;
      });

      files.sort();
    }
    
    doNextBench(connectionString);
  }
  else {
    if (errorCount) {
      console.log("\nResults : %s of %s tests failed.\n", errorCount, testCount);
    }
    else {
      console.log("Results : All tests were successful. Total %s files executed.", testCount);
    }
  }
}

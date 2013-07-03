var fs = require("fs")
  , common = require('./common.js')
  , spawn = require("child_process").spawn
  , requestedBench = null
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
  
  process.stdout.write("Running \033[01;33m" + file.replace(/\.js$/, "") + "\033[01;0m with [\033[01;29m" + connectionString.title + "\033[01;0m] : ");
  
  bench.on("exit", function (code, signal) {
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
    console.log("Done");
  }
}
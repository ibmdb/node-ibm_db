var fs = require("fs")
  , spawn = require("child_process").spawn
  , errorCount = 0
  , testCount = 0
  , testTimeout = 5000
  ;

var filesDisabled = fs.readdirSync("./disabled");

if (filesDisabled.length) {
  console.log("\n\033[01;31mWarning\033[01;0m : there are %s disabled tests\n", filesDisabled.length);
}

var files = fs.readdirSync("./");

files = files.filter(function (file) {
  return (/^test-/.test(file)) ? true : false;
});

files.sort();

doNextTest();

function doTest(file) {
  var test = spawn("node", ['--expose_gc',file])
    , timer = null
    , timedOut = false;
    ;
  
  process.stdout.write("Running test : " + file.replace(/\.js$/, ""));
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
    
    doNextTest();
  });
  
  var timer = setTimeout(function () {
    timedOut = true;
    test.kill();
  },testTimeout);
}

function doNextTest() {
  if (files.length) {
    var testFile = files.shift();
    
    doTest(testFile);
  }
  else {
    //we're done, display results and exit accordingly
    
    if (errorCount) {
      console.log("\nResults : %s of %s tests failed.\n", errorCount, testCount);
      process.exit(errorCount);
    }
    else {
      console.log("Results : All tests were successful.");
    }
  }
}

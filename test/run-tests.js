var fs = require("fs")
  , spawn = require("child_process").spawn
  , errorCount = 0
  , testCount = 0;
  ;

var filesDisabled = fs.readdirSync("./disabled");

if (filesDisabled.length) {
  console.log("\033[01;31mWarning\033[01;0m : there are %s disabled tests\n", filesDisabled.length);
}

var files = fs.readdirSync("./");

files = files.filter(function (file) {
  return (/^test-/.test(file)) ? true : false;
});

files.sort();

doNextTest();

function doTest(file) {
  var test = spawn("node", [file]);
  
  process.stdout.write("Running test : " + file.replace(/\.js$/, ""));
  process.stdout.write(" ... ");

  testCount += 1;
  
  test.on("exit", function (code, signal) {
    if (code != 0) {
      errorCount += 1;
      
      process.stdout.write("\033[01;31mfail \033[01;0m ");
    }
    else {
      process.stdout.write("\033[01;32msuccess \033[01;0m ");
    }
    
    process.stdout.write("\n");
    
    doNextTest();
  });
}

function doNextTest() {
  if (files.length) {
    var testFile = files.shift();
    
    doTest(testFile);
  }
  else {
    //we're done, display results and exit accordingly
    
    if (errorCount) {
      console.log("%s of %s tests failed", errorCount, testCount);
      process.exit(errorCount);
    }
    else {
      console.log("All tests were successful");
    }
  }
}
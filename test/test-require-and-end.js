var odbc = require("../")
    ;

//This test should just exit. This tests an issue where
//the C++ ODBC::Init function was causing the event loop to
//stay alive

console.log("done");
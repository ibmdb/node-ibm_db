var odbc = require("../")
    , db = new odbc.Database()
    ;

//This test should just exit. The only reason it should stay open is if a 
//connection has been established. But all we have done here is instantiate
//the object.

console.log("done");
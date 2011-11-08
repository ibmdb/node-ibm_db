/*
 * This example demonstrates odbc parameter binding.
 *
 */

var odbc = require("../odbc.js"),
      db = new odbc.Database();



//open a connection to the database
db.open("DSN=myDsnName;UID=myUserName;PWD=mySuperSecretPassword;DATABASE=myAwesomeDatabase;CHARSET=UTF8", function(err)
{

    if (err) {
        //Something went bad
        console.log(err);

        //Let's not go any further
        return;
    }

    /*
     * A quick example of passing some random parameters
     *
     */
    
    db.query("select ? as a, ? as b, ? as c, ? as d, ? as e", [null, 4711, true, -3.14, 'string'], function (error, result, info) {
        console.log("some random parameters");
        if (error) {console.log(error); return false;}
        console.log(result);
    });
    
    /*
     * Some non-ASCII-characters (note that db.open above is configured to use utf8 encoding) 
     *
     */
    
    db.query("select ?", ['áäàéêèóöòüßÄÖÜ€'], function (error, result) {
        console.log("some non-ASCII characters");
        if (error) {console.log(error); return false;} 
        console.log(result);
    });

});

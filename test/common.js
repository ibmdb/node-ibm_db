var odbc = require("../");
const os = require('os');

//connectionString = "DRIVER={DB2 ODBC Driver};DATABASE=SAMPLE;UID=db2admin;PWD=db2admin;HOSTNAME=localhost;port=50000;PROTOCOL=TCPIP";
var connectionString = "";
var connectionObject = "";
var isZOS = false;
if (os.type() === "OS/390") {
    isZOS = true;
}
// If env var IBM_DB_SERVER_TYPE is set to ZOS means, target DB is zOS
if(process.env.IBM_DB_SERVER_TYPE === "ZOS") {
    isZOS = true;
}

try {
  if (isZOS) {
    process.env.IBM_DB_SERVER_TYPE="ZOS";
    connectionObject = require('./config.zos.json');
  } else {
    connectionObject = require('./config.json');
  }
}
catch (e) {
  if (isZOS) {
    // On z/OS, ODBC driver uses a local connection that only requires DSN, UID and PWD.
    connectionObject = {
       DSN : "{Db2 ODBC Driver}",
       UID : "db2admin",
       PWD : ""
    };
  } else {
    connectionObject = {
      DRIVER : "{DB2 ODBC Driver}",
      DATABASE : "SAMPLE",
      HOSTNAME : "localhost",
      UID : "db2admin",
      PWD : "",
      PORT : "50000",
      PROTOCOL : "TCPIP"
    };
  }
}

//checks if schema is defined
if (process.env.IBM_DB_SCHEMA !== 'undefined') {
    connectionObject.CURRENTSCHEMA = process.env.IBM_DB_SCHEMA ||
                                     connectionObject.CURRENTSCHEMA;
}

connectionObject.UID = process.env.IBM_DB_UID || process.env.DB2_USER || connectionObject.UID;
connectionObject.PWD = process.env.IBM_DB_PWD || process.env.DB2_PASSWD || connectionObject.PWD;
if( !process.env.IBM_DB_PWD && !process.env.DB2_PASSWD ) {
    console.log("\n    ====================================================");
    console.log("    Warning: Environment variable DB2_PASSWD is not set.\n" +
                "    Please set it before running test file and avoid\n" +
                "    hardcoded password in config.json file.");
    console.log("    ====================================================\n");
}

// Construct the connection string
if (isZOS) {
  // On z/OS, ODBC driver uses a local connection that only requires DSN, UID and PWD.
  connectionObject.DSN = process.env.IBM_DB_DBNAME  || connectionObject.DSN;
  connectionString = "DSN=" + connectionObject["DSN"] + ";" +
                     "UID=" + connectionObject["UID"] + ";" +
                     "PWD=" + connectionObject["PWD"] + ";";
} else {
  connectionObject.DATABASE = process.env.IBM_DB_DBNAME  || connectionObject.DATABASE;
  connectionObject.HOSTNAME = process.env.IBM_DB_HOSTNAME || connectionObject.HOSTNAME;
  connectionObject.PORT    = process.env.IBM_DB_PORT     || connectionObject.PORT;
  connectionObject.PROTOCOL = process.env.IBM_DB_PROTOCOL || connectionObject.PROTOCOL;
  for(key in connectionObject) {
    if(connectionObject[key] != undefined)
      connectionString = connectionString + key + "=" +
                         connectionObject[key] + ";" ;
  }
}

exports.testConnectionStrings = [{ title : "DB2", 
                        connectionString : connectionString }];
exports.benchConnectionStrings = exports.testConnectionStrings;

if (process.argv.length === 3) {
  //look through the testConnectionStrings to see if there is a title that matches
  //what was requested.
  var lookup = process.argv[2];
  
  exports.testConnectionStrings.forEach(function (connectionString) {
    if (connectionString && connectionString.title &&
        connectionString.title == lookup) {
      connectionString = connectionString.connectionString
    }
  });
}

if(connectionObject.CURRENTSCHEMA == undefined) {
    connectionObject.CURRENTSCHEMA = "ZURBIE";
}
exports.connectionObject = connectionObject;
exports.connectionString = connectionString;
exports.isZOS = isZOS;
exports.databaseName = "SAMPLE";
exports.tableName = "NODE_ODBC_TEST_TABLE";

exports.dropTables = function (db, cb) {
  db.query("drop table " + exports.tableName, cb);
};

exports.createTables = function (db, cb) {
  db.query("create table " + exports.tableName + " (COLINT INTEGER, COLDATETIME TIMESTAMP, COLTEXT VARCHAR(255))", cb);
};

exports.sanitizeSP = function (proc) {
  if(isZOS) {
      // Update the queries for Db2 on z/OS compatiability.
      // Db2 for z/OS does not support CREATE OR REPLACE syntax.
      proc = proc.replace(" or replace", "");

      // When creating an SQL native procedure on z/OS, you will need
      // to have a WLM environment defined for your system if you want
      // to run the procedure in debugging mode.
      // Adding "disable debug mode" to bypass this requirement.
      proc = proc.replace(" begin", " disable debug mode begin");
  }
  return proc;
};

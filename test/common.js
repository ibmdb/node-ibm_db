var odbc = require("../");
const os = require('os');

//exports.connectionString = "DRIVER={DB2 ODBC Driver};DATABASE=SAMPLE;UID=db2admin;PWD=db2admin;HOSTNAME=localhost;port=50000;PROTOCOL=TCPIP";
exports.connectionString = "";

try {
  if (os.type() === "OS/390") {
    exports.connectionObject = require('./config.testConnectionStrings.zos.json');
  } else {
    exports.connectionObject = require('./config.testConnectionStrings.json');
  }
}
catch (e) {
  if (os.type() === "OS/390") {
    // On z/OS, ODBC driver uses a local connection that only requires DSN, UID and PWD.
    exports.connectionObject = {
       DSN : "{Db2 ODBC Driver}",
       UID : "db2admin",
       PWD : "db2admin"
    };
  } else {
    exports.connectionObject = {
      DRIVER : "{DB2 ODBC Driver}",
      DATABASE : "SAMPLE",
      HOSTNAME : "localhost",
      UID : "db2admin",
      PWD : "db2admin",
      PORT : "50000",
      PROTOCOL : "TCPIP"
    };
  }
}


//checks if schema is defined
if (process.env.IBM_DB_SCHEMA !== 'undefined') {
    exports.connectionObject.CURRENTSCHEMA = process.env.IBM_DB_SCHEMA || exports.connectionObject.CURRENTSCHEMA;
}

exports.connectionObject.UID     = process.env.IBM_DB_UID      || exports.connectionObject.UID;
exports.connectionObject.PWD     = process.env.IBM_DB_PWD      || exports.connectionObject.PWD;

// Construct the connection string
if (os.type() === "OS/390") {
  // On z/OS, ODBC driver uses a local connection that only requires DSN, UID and PWD.
  exports.connectionObject.DSN = process.env.IBM_DB_DBNAME  || exports.connectionObject.DSN;
  exports.connectionString = "DSN=" + exports.connectionObject["DSN"] + ";" +
                             "UID=" + exports.connectionObject["UID"] + ";" +
                             "PWD=" + exports.connectionObject["PWD"] + ";";
} else {
  exports.connectionObject.DATABASE = process.env.IBM_DB_DBNAME  || exports.connectionObject.DATABASE;
  exports.connectionObject.HOSTNAME = process.env.IBM_DB_HOSTNAME || exports.connectionObject.HOSTNAME;
  exports.connectionObject.PORT    = process.env.IBM_DB_PORT     || exports.connectionObject.PORT;
  exports.connectionObject.PROTOCOL = process.env.IBM_DB_PROTOCOL || exports.connectionObject.PROTOCOL;
  for(key in exports.connectionObject) {
    if(exports.connectionObject[key] != undefined)
      exports.connectionString = exports.connectionString + key + "=" +
                                 exports.connectionObject[key] + ";" ;
  }
}
//if (process.argv.length === 3) {
//  exports.connectionString = process.argv[2];
//}

exports.testConnectionStrings = [{ title : "DB2", 
                        connectionString : exports.connectionString }];
exports.benchConnectionStrings = exports.testConnectionStrings;

if (process.argv.length === 3) {
  //look through the testConnectionStrings to see if there is a title that matches
  //what was requested.
  var lookup = process.argv[2];
  
  exports.testConnectionStrings.forEach(function (connectionString) {
    if (connectionString && connectionString.title && connectionString.title == lookup) {
      exports.connectionString = connectionString.connectionString
    }
  });
}

exports.databaseName = "SAMPLE";
exports.tableName = "NODE_ODBC_TEST_TABLE";

exports.dropTables = function (db, cb) {
  db.query("drop table " + exports.tableName, cb);
};

exports.createTables = function (db, cb) {
  db.query("create table " + exports.tableName + " (COLINT INTEGER, COLDATETIME TIMESTAMP, COLTEXT VARCHAR(255))", cb);
};

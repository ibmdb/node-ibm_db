var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;
var platform = require('os').platform();

console.log("Trying to open a connection ... ");
ibmdb.open(cn, function(err, conn) {
  if(err) console.log(err);
  assert.equal(err, null);
  console.log(" ... Got the connection.");
  
  let dbms = conn.getInfoSync(ibmdb.SQL_DBMS_NAME);
  console.log("SQL_DBMS_NAME(Server Type) = ", dbms);
  console.log("SQL_DBMS_VER(Server Version) = ",  conn.getInfoSync(ibmdb.SQL_DBMS_VER));
  console.log("SQL_DRIVER_VER(client version) = ",  conn.getInfoSync(ibmdb.SQL_DRIVER_VER));
  console.log("SQL_MAX_SCHEMA_NAME_LEN = ",  conn.getInfoSync(ibmdb.SQL_MAX_SCHEMA_NAME_LEN));
  conn.getInfo(ibmdb.SQL_DBMS_NAME, function(error, data) {
    if(error) {
      console.log(error);
      conn.closeSync();
    } else {
      console.log("SQL_DBMS_NAME(Server Type) = ", data);
      conn.getInfo(ibmdb.SQL_DRIVER_BLDLEVEL, 20, function(error, data) {
        if(error) {
          console.log(error);
          conn.closeSync();
        } else {
          console.log("SQL_DRIVER_BLDLEVEL(client level) = ",  data);
          conn.getInfo(ibmdb.SQL_MAX_TABLE_NAME_LEN, function(error, data) {
            if(error) {
              console.log(error);
              conn.closeSync();
            }
            else {
              console.log("SQL_MAX_TABLE_NAME_LEN = ",  data);
              conn.getInfo(ibmdb.SQL_DBMS_FUNCTIONLVL, function(err, lvl) {
                if(err) console.log(err);
                else {
                  console.log("SQL_DBMS_FUNCTIONLVL(server level) = ", lvl);
                }
                conn.closeSync();
              });
            }
          });
        }
      });
    }
  });
});


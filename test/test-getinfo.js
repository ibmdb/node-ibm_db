var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;
var platform = require('os').platform();

main();
async function main()
{
    try {
        const conn = await ibmdb.open(cn);
        await testGetInfoSync(conn);
        await testGetTypeInfoSync(conn);
        await testGetFunctionsSync(conn);
        await testGetFunctionsAwait(conn);
        await testGetInfoAwait(conn);
        await testGetTypeInfoAwait(conn);
        await testGetInfoCallback(conn);
        //testGetTypeInfoCallback(conn);
    } catch (e) {
        console.log(e);
        assert.equal(true, false);
    }
}

async function testGetInfoSync(conn)
{
  console.log("\n====== testGetInfoSync ======");
  let dbms = conn.getInfoSync(ibmdb.SQL_DBMS_NAME);
  console.log("SQL_DBMS_NAME(Server Type) = ", dbms);
  console.log("SQL_DBMS_VER(Server Version) = ",  conn.getInfoSync(ibmdb.SQL_DBMS_VER));
  console.log("SQL_DRIVER_VER(client version) = ",  conn.getInfoSync(ibmdb.SQL_DRIVER_VER));
  console.log("SQL_MAX_SCHEMA_NAME_LEN = ",  conn.getInfoSync(ibmdb.SQL_MAX_SCHEMA_NAME_LEN));
}

async function testGetInfoCallback(conn)
{
  console.log("\n====== testGetInfoCallback ======");
  conn.getInfo(ibmdb.SQL_DBMS_NAME, function(error, data) {
    if(error) {
      console.log(error);
    } else {
      console.log("SQL_DBMS_NAME(Server Type) = ", data);
      conn.getInfo(ibmdb.SQL_DRIVER_BLDLEVEL, 20, function(error, data) {
        if(error) {
          console.log(error);
        } else {
          console.log("SQL_DRIVER_BLDLEVEL(client level) = ",  data);
          conn.getInfo(ibmdb.SQL_MAX_TABLE_NAME_LEN, 30, function(error, data) {
            if(error) {
              console.log(error);
            }
            else {
              console.log("SQL_MAX_TABLE_NAME_LEN = ",  data);
              conn.getInfo(ibmdb.SQL_DBMS_FUNCTIONLVL, function(err, lvl) {
                if(err) console.log(err);
                else {
                  console.log("SQL_DBMS_FUNCTIONLVL(server level) = ", lvl);
                  testGetTypeInfoCallback(conn);
                }
              });
            }
          });
        }
      });
    }
  });
}

async function testGetInfoAwait(conn)
{
    console.log("\n====== testGetInfoAwait ======");
    try {
      let data =  await conn.getInfo(ibmdb.SQL_INVALID_INFO2);
      console.log("SQL_INVALID_INFO = ",  data);
    } catch(e) {
      console.log("SQL_INVALID_INFO = ",  e);
    }

    try {
      let data = await conn.getInfo(ibmdb.SQL_DBMS_NAME);
      console.log("SQL_DBMS_NAME(Server Type) = ", data);
      console.log("SQL_APPLICATION_CODEPAGE = ",
          await conn.getInfo(ibmdb.SQL_APPLICATION_CODEPAGE));
      conn.getInfo(ibmdb.SQL_APPLICATION_CODEPAGE2)
            .then(data => console.log("SQL_APPLICATION_CODEPAGE2 = ", data))
            .catch(error => console.log("SQL_APPLICATION_CODEPAGE2 = ", error));
      data = await conn.getInfo(ibmdb.SQL_ASYNC_MODE)
             .catch(error => console.log("SQL_ASYNC_MODE = ", error));
      console.log("SQL_DB2_DRIVER_TYPE = ", await conn.getInfo(2567));
      data = await conn.getInfo(ibmdb.SQL_CONNECT_CODEPAGE);
      console.log("SQL_CONNECT_CODEPAGE = ", data);
    } catch (e) {
        console.log(e);
        assert(false);
    }
}

async function testGetTypeInfoSync(conn)
{
  console.log("SQL_BIGINT type info = ",  conn.getTypeInfoSync(ibmdb.BIGINT));
  //console.log("SQL_ALL_TYPES type info = ",  conn.getTypeInfoSync(ibmdb.ALLTYPES));
}

async function testGetTypeInfoAwait(conn)
{
    console.log("\n====== testGetTypeInfoAwait ======");
    try {
      let data =  await conn.getTypeInfo(ibmdb.SQL_INVALID_TYPE);
      console.log("SQL_INVALID_TYPE = ",  data);
    } catch(e) {
      console.log("SQL_INVALID_TYPE = ",  e);
    }
    console.log("SQL_BLOB type info = ", await conn.getTypeInfo(ibmdb.BLOB));

    try {
      let data = await conn.getTypeInfo(ibmdb.DBCLOB);
      console.log("SQL_DBCLOB = ", data);
      conn.getTypeInfo(ibmdb.NUMBER)
            .then(data => console.log("NUMBER = ", data))
            .catch(error => console.log("NUMBER = ", error));
      data = await conn.getTypeInfo(ibmdb.STRING)
             .catch(error => console.log("ibmdb.STRING = ", error));
      console.log("SQL_DOUBLE = ", await conn.getTypeInfo(8));
    } catch (e) {
        console.log(e);
        assert(false);
    }
}

async function testGetTypeInfoCallback(conn)
{
  console.log("\n====== testGetTypeInfoCallback ======");
  conn.getTypeInfo(ibmdb.NUMERIC, function(error, data) {
    if(error) { console.log(error); }
      else {
      console.log("SQL_NUMERIC Type Info = ", data);
      conn.getTypeInfo(ibmdb.LONGVARCHAR, function(error, data) {
        if(error) { console.log(error); } else {
          console.log("SQL_LONGVARCHAR Type Info = ", data);
          conn.getTypeInfo(ibmdb.FLOAT, function(error, data) {
            if(error) { console.log(error); }
            else {
              console.log("SQL_FLOAT Type Info = ",  data);
              conn.closeSync();
            }
          });
        }
      });
    }
  });
}

async function testGetFunctionsSync(conn)
{
  console.log("\n====== testGetFunctionsSync ======");
  let fExists = conn.getFunctionsSync(ibmdb.SQLCONNECT);
  console.log("Function SQLConnect Exist = ", fExists);
  console.log("Function SQLFreeConnect Exist = ", conn.getFunctionsSync(ibmdb.SQLFREECONNECT));
  console.log("Function SQLGetFunctions Supported = ", conn.getFunctionsSync(ibmdb.SQLGETFUNCTIONS));
  console.log("Function SQLDrivers supported = ", conn.getFunctionsSync(ibmdb.DRIVERS));
  console.log("Function SQLNumParams supported = ", conn.getFunctionsSync(63));
  console.log("Function Unknows supported = ", conn.getFunctionsSync(630));
  console.log("Function SQLDropDb supported = ", conn.getFunctionsSync(ibmdb.SQLDROPDB));
  console.log("All supported Functions = ", conn.getFunctionsSync(ibmdb.ALLFUNCTIONS));
  console.log("-------------------------------------\n");
}

async function testGetFunctionsAwait(conn)
{
    console.log("\n====== testGetFunctionsAwait ======");
    try {
      let data =  await conn.getFunctions(ibmdb.SQL_INVALID_FUNC);
      console.log("SQL_INVALID_FUNCTION = ",  data);
    } catch(e) {
      console.log("SQL_INVALID_FUNCTION = ",  e);
    }

    try {
      let data = await conn.getFunctions(ibmdb.SQLCONNECT);
      console.log("ibmdb.SQLCONNECT = ", data);
      console.log("Function SQLGetFunctions Supported = ",
          await conn.getFunctions(ibmdb.SQLGETFUNCTIONS));
      conn.getFunctions(ibmdb.DRIVERS)
            .then(data => console.log("Function DRIVERS supported = ", data))
            .catch(error => console.log("Function DRIVERS supported = ", error));
      data = await conn.getFunctions(63)
             .catch(error => console.log("Function SQLNumParams supported = ", error));
      console.log("SQLDROPDB = ", await conn.getFunctions(ibmdb.SQLDROPDB));
      console.log("All supported Functions = ", await conn.getFunctions(ibmdb.ALLFUNCTIONS));
    } catch (e) {
        console.log(e);
        assert(false);
    }
    console.log("-------------------------------------\n");
}


let common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  ;

let conn = null;
let data = null;

main();

async function main() {
  conn = await ibmdb.open(common.connectionString).catch((e) => {console.log(e);});
  await setConnectAttr();
  await conn.close();
  console.log("All tests executed.");
}

async function setConnectAttr() {
    console.log(conn.setAttrSync(21, 'invalidoption'));
    console.log(conn.setAttrSync(1281, 'bimal'));
    console.log(conn.setAttrSync(ibmdb.SQL_ATTR_INFO_APPLNAME, 'mynodeApp'));
    data = conn.querySync("select CURRENT CLIENT_USERID as userid, CURRENT CLIENT_APPLNAME as appname from sysibm.sysdummy1");
    console.log("Selected userid = ", data);
    assert.deepEqual(data, [{ APPNAME: 'mynodeApp', USERID: 'bimal' }]);

    await conn.setAttr(22, 22).catch((e) => {console.log(e);});
    console.log(await conn.setAttr("SQL_ATTR_INFO_WRKSTNNAME", 'mythinkpad'));
    data = await conn.query("select CURRENT CLIENT_WRKSTNNAME as host from sysibm.sysdummy1");
    console.log("Selected host = ", data);
    assert.deepEqual(data, [{ HOST: 'mythinkpad' }]);

    return;
}


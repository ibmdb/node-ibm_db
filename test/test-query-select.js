let common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  ;

let conn = null;
let data = null;

main();

async function main() {
  conn = await ibmdb.open(common.connectionString);
  await testColAlias();
  await testDupColNames();
  await conn.close();
  console.log("All tests executed.");
}

async function testColAlias() {
  try {
      data = await conn.query("select 1 as \"COLINT\", 'some test' as \"COLTEXT\" FROM SYSIBM.SYSDUMMY1");
  } catch (error) {
      assert.equal(error, null);
  }
  console.log("Result from testColAlias = ",data);
  assert.deepEqual(data, [{ COLINT: '1', COLTEXT: 'some test' }]);
  return;
}

async function testDupColNames() {
  let sql = "select 'text 1' as name, 'text 2' as name from SYSIBM.SYSDUMMY1";
  try {
      data = await conn.query(sql);
  } catch(error) {
      assert.equal(error, null);
  }
  console.log("Result from testDupColNames= ", data);
}


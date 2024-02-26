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
  await testColumnAlias();
  await testCharInsert();
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

async function testColumnAlias() {
  let sql = "select 'abc' as first, 'dcd' as second, 'efg', 'pqr', 'xyz' as last from SYSIBM.SYSDUMMY1";
  ibmdb.debug(2);
  try {
      data = await conn.querySync(sql);
  } catch(error) {
      assert.equal(error, null);
  }
  console.log("Result from testColumnAlias= ", data);
  ibmdb.debug(false);
}

async function testCharInsert() {
    await conn.query("drop table test1").catch((e) => {});
    await conn.query("create table test1(c1 int, c2 char(5))");
    await conn.query("insert into test1 values (1, 'abcde')");
    await conn.query("insert into test1 values (2, 'abcd')");
    await conn.query("insert into test1 values (?, ?)", [3, 'ABCDE']);
    await conn.query("insert into test1 values (?, ?)", [4, 'ABCD']);
    await conn.query("insert into test1 values (?, ?)", [5, { ParamType: 'INPUT', SQLType: 'CHAR', Data: 'DSWC' }]);
    console.log("Result from testCharInsert =", conn.querySync("select * from test1"));
    await conn.query("drop table test1");
}

var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;

main();
async function main() {
  try {
    let conn = await ibmdb.open(cn);
    await conn.query("create table mytab(c1 int, c2 varchar(10))");
    await conn.query("insert into mytab values (?, ?)", [3, 'ibm']);
    let stmt = await conn.prepare("select * from mytab");
    let result = await stmt.execute();
    data = await result.fetchAll();
    console.log("result = ", data);
    assert.deepEqual(data, [ { C1: 3, C2: 'ibm' } ]);
    await result.close();
    await stmt.close();
    await conn.query("drop table mytab").catch((e) => {console.log(e);});
    await conn.close();
  } catch(e) {
      console.log(e);
  }
}


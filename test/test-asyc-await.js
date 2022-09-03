var common = require("./common")
  , ibmdb = require("../")
  , pool = new ibmdb.Pool()
  , assert = require("assert")
  , cn = common.connectionString
  ;

main();
async function main() {
    await test1();
    await test2();
    await test3();
}

async function test1() {
    let invalidConnStr = "database=wrongdb;" + cn ;
    let conn = await ibmdb.open(invalidConnStr).catch((e) => {
                     console.log(e); });
    if(conn) console.log(conn);
    console.log("Test1 done.");
}

async function test2() {
  try {
    let conn = await ibmdb.open(cn);
    let data = await conn.query("select 1 from sysibm.sysdummy1");
    console.log("data = ", data);

    await conn.query("drop table mytab").catch((e) => {console.log(e);});
    await conn.query("create table mytab(c1 int, c2 varchar(10))");
    await conn.query("insert into mytab values (?, ?)", [3, 'rocket']);
    let stmt = await conn.prepare("select * from mytab");
    let result = await stmt.execute();
    data = await result.fetchAll();
    console.log("result = ", data);
    await result.close();
    await stmt.close();
    await conn.close();
  } catch(e) {
      console.log(e);
  }
  console.log("Test2 done.");
}

async function test3()
{
    pool.setMaxPoolSize(2); // Max no of active connections.
    let ret = pool.init(1, cn);
    if(typeof ret === 'object') assert.equal(ret.message, undefined);

    try {
      let conn = await pool.open(cn);
      let data = await conn.query("select 1 from sysibm.sysdummy1");
      console.log("data = ", data);

      await conn.query("drop table mytab");
      await conn.query("create table mytab(c1 int, c2 varchar(10))");
      await conn.query("insert into mytab values (?, ?)", [3, 'rocket']);
      let stmt = await conn.prepare("select * from mytab");
      let result = await stmt.execute();
      data = await result.fetchAll();
      console.log("result = ", data);
      await result.close();
      await stmt.close();
      await conn.query("drop table mytab");
      await conn.close();
    } catch(e) {
        console.log(e);
    }
    await pool.close();
    console.log("Test3 done.");
}

var common = require("./common")
  , ibmdb = require("../")
  , pool = new ibmdb.Pool()
  , assert = require("assert")
  , cn = common.connectionString
  ;

process.on('unhandledRejection', async (reason, p) => {
    console.debug('Unhandled Error: ', reason, p)
});

main();
async function main() {
    await test1();
    await test2();
    await test3();
    await test4();
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
    const query = 'update mytab set c2=? WHERE c1=?;'
    pool.setMaxPoolSize(2); // Max no of active connections.
    await pool.initAsync(1, cn).catch((err) => {
      console.log(err);
      if(typeof err === 'object') assert.equal(err.message, undefined);
    });

    try {
      let conn = await pool.open(cn);
      let data = await conn.query("select 1 from sysibm.sysdummy1");
      console.log("data = ", data);

      await conn.query("drop table mytab");
      await conn.query("create table mytab(c1 int, c2 varchar(10))");
      await conn.query("insert into mytab values (?, ?)", [1050, 'rocket']);

      // Test for issue #960
      let stmt = await conn.prepare(query)
      let result = await stmt.executeNonQuery(['canceled', 1050])
      console.log("No of updated row = ", result)
      assert.equal(1, result);

      stmt = await conn.prepare("select * from mytab");
      result = await stmt.execute();
      data = await result.fetchAll();
      console.log("result = ", data);
      assert.equal(data[0].C2, 'canceled');
      await result.close();
      await stmt.close();
      console.log("Test for issue #960 is done.");
      // Test for issue #996
      try {
        await conn.query("insert into mytab values (?, ?)", [[1050, 'rocket']]);
      } catch (err) { console.log("Got error, throwing it."); throw err; }
      data = await conn.query("select * from mytab");
      console.log("data = ", data);
      console.log("Test for issue #996 is done.");

      // Clean up
      await conn.query("drop table mytab");
      await conn.close();
    } catch(e) {
        console.log(e);
    }
    await pool.close();
    console.log("Test3 done.");
}

async function test4() {
    let invalidSQL = ["not an sql"] ;
    let conn = await ibmdb.open(cn);
    await conn.prepare(invalidSQL).catch((e) => { console.log(e); });
    console.log("Test4 done.");
}

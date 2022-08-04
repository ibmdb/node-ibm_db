var common = require("./common")
    , ibmdb = require("../")
    , crypt = require('crypto')
    , assert = require('assert')
    , cn = common.connectionString;

async function main() {	
    const BLOB_LENGTH = 1024 * 16;
    const conn = await ibmdb.open(cn)
    let results;

    results = await conn.querySync(`create table DBCLOB_TAB (id int, B1 DBCLOB(${BLOB_LENGTH}))`);
    console.log(results);

    const filler = 'X'.repeat(1000);

    const source = {
     "V1" : filler
   , "V2" : "Oops"
   }

   const data = JSON.stringify(source," ",2);
   console.log(data.length,Buffer.byteLength(data));

    try {
      results = await conn.query(`insert into DBCLOB_TAB (ID, B1) values (?, ?)`,[1,data]);
      console.log(results);
    } catch (e) {
      console.log(1,e);
    }

    results = await conn.query(`select ID, length(B1) L1, B1 from DBCLOB_TAB`);
    console.log("result = ", results);
    console.log("data =   ", data);

    let target;
    try {
      target = JSON.parse(results[0].B1);
      console.log(source.V1.length,target.V1.length);
    } catch (e) {
      console.log('Oops',e);
    }

    for (var i = 0; i < data.length; i++) {
      if (data.charAt(i) !== results[0].B1.charAt(i)) {
        console.log(i,data.charAt(i), results[0].B1.charAt(i));
        assert.equal(data.charAt(i), results[0].B1.charAt(i));
        break;
      }
    }
    await conn.query(`drop table DBCLOB_TAB`);
}

async function blobTest() {
    const BLOB_LENGTH = 256;
    const conn = await ibmdb.open(cn);
    let results;

    results = await conn.querySync(`create table BLOB_TAB (id int, B1 BLOB(${BLOB_LENGTH}))`);
    console.log(results);

    const values = Array.from(Array(BLOB_LENGTH).keys());
    console.log("values = ", values);
    const buf = Buffer.from(values);
    console.log("buffer = ", buf);
    console.log("buflen = ", buf.length);

    const blobParam = {DataType: "BLOB", Data:buf};

    try {
      const query = {
        sql   : `insert into BLOB_TAB (ID, B1) values (?, ?)`
      ,	params:  [1, blobParam]
      }

      results = await conn.query(query);
      console.log(results);
    } catch (e) {
      console.log(1,e);
    }
    results = await conn.query(`select ID, length(B1) L1, B1 from BLOB_TAB`);
    console.log("result = ", results);
    await conn.query(`drop table BLOB_TAB`);
}

main().then(() => {
    console.log('DBCLOB test success');
    console.log("==================\n");
    blobTest().then(() => {
        console.log('BLOB test success');
        console.log("==================\n");
        })
    }).catch((e) => { console.log(e); })

var common = require("./common")
    , ibmdb = require("../")
    , crypt = require('crypto')
    , assert = require('assert')
    , cn = common.connectionString;

async function main() {	
    const BLOB_LENGTH = 1024 * 16;
    const conn = await ibmdb.open(cn)
    let results;

	results = await conn.query(`BEGIN DECLARE V_STATEMENT VARCHAR(300) DEFAULT 'drop table BINARY_TAB'; DECLARE CONTINUE HANDLER FOR SQLSTATE '42704' BEGIN  END;  EXECUTE IMMEDIATE V_STATEMENT; END;`)
	console.log(results)

    results = await conn.querySync(`create table BINARY_TAB (id int, B1 BINARY(12), VB1 VARBINARY(12), C4B1 CHAR(12) FOR BIT DATA )`);
	console.log(results)
	
	const data = [
      '628bacf012d4628bacd4',
      '628bacf012d5628bacf012d5',
      '628bacf012d6628bacf0d6',
      '628bacf012d7628bacf0d7',
      '628bacf012d8628bacf012d8'
    ]
	ibmdb.debug(2);
	let idx=0
	let parameters = [{
 		   ParamType: 'ARRAY',
           DataType: 'INT',
           Data: data.map(() => { return idx++ }),
		},{
 		   ParamType: 'ARRAY',
           DataType: 'BINARY',
           Data: data.map((hex) => { return Buffer.from(hex, 'hex')}),
		   Length : 12
		},{
 		   ParamType: 'ARRAY',
           DataType: 'BINARY',
           Data: data.map((hex) => { return Buffer.from(hex, 'hex')}),
		   Length : 12
	    },{
 		   ParamType: 'ARRAY',
           DataType: 'BINARY',
           Data: data.map((hex) => { return Buffer.from(hex, 'hex')}),
		   Length : 12
	    }];
    console.log("Inserted parameters = ", parameters);
	try {
      const query = {
        sql: 		  `insert into BINARY_TAB (ID, B1, VB1, C4B1) values (?, ?, ?, ?)`
	  ,	params: parameters
	  , ArraySize: data.length
	  }
	  
      results = await conn.query(query);
  	  console.log(results)
	} catch (e) {
	  console.log(1,e)
	}
	
	results = await conn.query(`select ID, B1, VB1, C4B1 from BINARY_TAB`)
    console.log("BINARY_TAB contents:\n", results);
     
    results.forEach( (row) => { console.log(row.ID, data[row.ID], Buffer.from(data[row.ID],'hex'), row.B1, (row.B1.toString('hex') === data[row.ID]), row.VB1, (row.VB1.toString('hex') === data[row.ID]), row.C4B1, (row.C4B1.toString('hex') === data[row.ID]))})


	results = await conn.query(`select ID, RAWTOHEX(B1) "B1", RAWTOHEX(VB1) "VB1", RAWTOHEX(C4B1) "C4B1" from BINARY_TAB`)
    console.log("BINARY_TAB contents:\n", results);


}

main().then(() => { console.log('success')}).catch((e) => { console.log(e) })

// To run this test.ts file, follow below steps
// npm install -g typescript
// npm install --save-dev @types/node
// update database connection info in ../test/config.json file
// tsc test.ts --lib es2015,dom
// node test.js
// OR, run: tsc test.ts && node test.js && rm test.js

import * as ibmdb from '../'
import * as common from '../test/common.js'

let cn = common.connectionString
  , pool = new ibmdb.Pool();

async function  main(){
  await pool.initAsync(1, cn)
  const conn = await pool.open(cn)

  // Create table and insert data
  try {
    await conn.query("drop table employee");
  } catch (e) {};
  await conn.query("CREATE TABLE employee ( name VARCHAR(20), department VARCHAR(20), phoneno int)");
  await conn.query("INSERT INTO employee (name, department, phoneno) VALUES ('John Doe', 'Sales', 3978)");

  const query = `SELECT * FROM employee WHERE phoneno = ?;`
  const stmt = await conn.prepare(query);
  let result = await stmt.execute([3978]);
  if(Array.isArray(result)) {
      result = result[0];
  }

  let data = await result.fetchAll()
  console.log(data);
  if(data[0] && !Array.isArray(data[0])) {
      const rows = data as {NAME:string, DEPARTMENT:string, PHONENO:number}[];
      console.log(rows[0]?.PHONENO);
  }
  await conn.close()
}

main();


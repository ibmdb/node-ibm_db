// To run this test.ts file, follow below steps
// npm install -g typescript
// update database connection info in ../test/config.json file
// tsc test.ts --lib es2015,dom
// node test.js

import * as ibmdb from '../'
import * as common from '../test/common.js'

let cn = common.connectionString
  , pool = new ibmdb.Pool();

async function  main(){
  const query = `SELECT * FROM employee WHERE phoneno = ?;`

  await pool.initAsync(1, cn)
  const conn = await pool.open(cn)
  const stmt = await conn.prepare(query)
  const result = await stmt.execute([3978])

  const fetched = await result.fetchAll()
  console.log(fetched);
  await conn.close()
}

main()

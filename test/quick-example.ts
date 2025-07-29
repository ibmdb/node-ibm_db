// To run this quick-example.ts file, follow below steps
//  npm install -g typescript
//  npm install --save-dev @types/node
// update database connection info in ../test/config.json file or
// set environment variables DB2_USER, DB2_PASSWD, etc.
//  tsc quick-example.ts --target ES2016 --lib ES2016 
//  node quick-example.js
// OR, run:
//  tsc quick-example.ts && node quick-example.js && rm quick-example.js
//
//  To run this test program using ts-node, Allow JavaScript
//  Modules in tsconfig.json file. For that -
//  Modify your tsconfig.json to allow JS modules:
//    Open tsconfig.json
//    Add or modify these options:
//    {
//      "compilerOptions": {
//        "allowJs": true,           // Allow JS modules
//        "skipLibCheck": true,       // Skip type checking for JS files
//        "noImplicitAny": false,     // Allow 'any' type
//        "moduleResolution": "node"  // Use Node.js module resolution
//      }
//    }
//    Save the file and rerun:
//    ts-node quick-example.ts

import * as common from "./common";
import * as ibmdb from "../";
import * as assert from "assert";

// Define connection string
const cn: string = common.connectionString;

async function main(): Promise<void> {
  try {
    // Open connection
    const conn = await ibmdb.open(cn);

    // Execute queries
    await conn.query("create table mytab(c1 int, c2 varchar(10))");
    await conn.query("insert into mytab values (?, ?)", [3, 'ibm']);

    // Prepare and execute statement
    const stmt = await conn.prepare("select * from mytab");
    let result = await stmt.execute();
    if(Array.isArray(result)) {
        result = result[0];
    }
    const rows = await result.fetchAll();
    type RowData = { C1: number; C2: string };
    let data: RowData[] = [];
    if( rows[0] && !(Array.isArray(rows[0])) ) {
      data = rows as RowData[];
    }

    // Log results
    console.log("result = ", data);
    console.log("C2 =", data[0]?.C2);
    assert.deepEqual(data, [{ C1: 3, C2: 'ibm' }]);

    // Close resources
    await result.close();
    await stmt.close();
    
    // Drop table (ignore errors)
    await conn.query("drop table mytab").catch((e: Error) => console.error("Drop table error:", e));

    await conn.close();
  } catch (e: unknown) {  // Explicitly type as `unknown` and check type
    if (e instanceof Error) {
      console.error("Error:", e.message);
    } else {
      console.error("An unknown error occurred:", e);
    }
  }
}

// Run the function
main();


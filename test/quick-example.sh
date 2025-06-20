#!/bin/sh

cd ..
npm install -g typescript
npm install --save-dev @types/node
# update database connection info in test/config.json file or
# set environment variables DB2_USER, DB2_PASSWD, etc.
cd test
tsc quick-example.ts 
# If above command fails, run with below command:
# tsc quick-example.ts --target ES2016 --lib ES2016 
node quick-example.js
rm quick-example.js

#  To run quick-example.ts using ts-node, Allow JavaScript
#  Modules in tsconfig.json file. For that -
#  Modify your tsconfig.json to allow JS modules:
#    Open tsconfig.json
#    Add or modify these options:
#    {
#      "compilerOptions": {
#        "allowJs": true,           // Allow JS modules
#        "skipLibCheck": true,       // Skip type checking for JS files
#        "noImplicitAny": false,     // Allow 'any' type
#        "moduleResolution": "node"  // Use Node.js module resolution
#      }
#    }
#    Save the file and rerun:
#    ts-node quick-example.ts


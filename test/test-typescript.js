const exec = require('child_process').exec;
const execSync = require('child_process').execSync;
const fs = require('fs');

execSync('cd .. && npm install -g typescript');
execSync('cd .. && npm install --save-dev @types/node');
var yourscript = exec('tsc quick-example.ts && node quick-example.js',
        (error, stdout, stderr) => {
            console.log(stdout);
            console.log(stderr);
            if (error !== null) {
                console.log(`exec error: ${error}`);
            } else {
                try {
                  fs.unlinkSync('quick-example.js');
                } catch (err) {
                  console.error('Error deleting file:', err);
                }
            }
        });

yourscript = exec('tsc ../typescript/tests/test.ts && node ../typescript/tests/test.js',
        (error, stdout, stderr) => {
            console.log(stdout);
            console.log(stderr);
            if (error !== null) {
                console.log(`exec error: ${error}`);
            } else {
                try {
                  fs.unlinkSync('../typescript/tests/test.js');
                  fs.unlinkSync('../typescript/ConnStr.js');
                  fs.unlinkSync('../typescript/DB2Error.js');
                  fs.unlinkSync('../typescript/Database.js');
                  fs.unlinkSync('../typescript/DescribeObject.js');
                  fs.unlinkSync('../typescript/ODBC.js');
                  fs.unlinkSync('../typescript/ODBCConnection.js');
                  fs.unlinkSync('../typescript/ODBCResult.js');
                  fs.unlinkSync('../typescript/ODBCStatement.js');
                  fs.unlinkSync('../typescript/Options.js');
                  fs.unlinkSync('../typescript/Pool.js');
                  fs.unlinkSync('../typescript/PoolOptions.js');
                  fs.unlinkSync('../typescript/attributes.js');
                  fs.unlinkSync('../typescript/index.js');
                } catch (err) {
                  console.error('Error deleting file:', err);
                }
            }
        });

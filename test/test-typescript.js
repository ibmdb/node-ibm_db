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


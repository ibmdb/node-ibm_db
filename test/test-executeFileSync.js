var fs = require('fs');
var path = require('path')
var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;

ibmdb.open(cn, function (err, conn) {
    if (err) {
        return console.log(err);
    }

    var inputfile = __dirname + '/data/sample1.txt';
    var res = conn.executeFileSync(inputfile);
    assert.deepEqual(res, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}];[{"NO":1},{"NO":2}];');


    var res1 = conn.executeFileSync(inputfile, ';');
    assert.deepEqual(res1, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}];[{"NO":1},{"NO":2}];');


    var outputfile1 = __dirname + '/data/out1.txt';
    conn.executeFileSync(inputfile, ';', outputfile1);
    var result1 = fs.readFileSync(outputfile1, 'utf8')
    assert.deepEqual(result1, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}];[{"NO":1},{"NO":2}];');
    if (fs.existsSync(outputfile1)) {
        fs.unlinkSync(outputfile1)
    }

    var inputfile2 = __dirname + '/data/sample2.txt';
    var res2 = conn.executeFileSync(inputfile2, '%');
    assert.deepEqual(res2, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]%[{"NO":1},{"NO":2}]%');

    var outputfile2 = __dirname + '/data/out2.txt';
    conn.executeFileSync(inputfile2, '%', outputfile2);
    var result2 = fs.readFileSync(outputfile2, 'utf8')
    assert.deepEqual(result2, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]%[{"NO":1},{"NO":2}]%');
    if (fs.existsSync(outputfile2)) {
        fs.unlinkSync(outputfile2)
    }

    var inputfile3 = __dirname + '/data/sample3.txt';
    var res3 = conn.executeFileSync(inputfile3, '@');
    assert.deepEqual(res3, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]@[{"NO":1},{"NO":2}]@');

    var outputfile3 = __dirname + '/data/out3.txt';
    conn.executeFileSync(inputfile3, '@', outputfile3);
    var result3 = fs.readFileSync(outputfile3, 'utf8')
    assert.deepEqual(result3, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]@[{"NO":1},{"NO":2}]@');
    if (fs.existsSync(outputfile3)) {
        fs.unlinkSync(outputfile3)
    }

    var outputfile4 = __dirname + '/data/abc/out3.txt';
    conn.executeFileSync(inputfile3, '@', outputfile4);
    var result4 = fs.readFileSync(outputfile4, 'utf8')
    assert.deepEqual(result4, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]@[{"NO":1},{"NO":2}]@');
    if (fs.existsSync(outputfile4)) {
        fs.unlinkSync(outputfile4)
        fs.rmdirSync(__dirname + '/data/abc')
    }

    var err = null;
    try {
        err = conn.executeFileSync(inputfile3);
    }
    catch (e) {
        err = e;
    }
    assert.equal(err.error, "[node-ibm_db] Error in ODBCConnection::QuerySync while executing query.");

    var inputfile30 = __dirname + '/data/sample100.txt';
    var err1 = null;
    try {
        err1 = conn.executeFileSync(inputfile30);
    }
    catch (e) {
        err1 = e;
    }
    assert.equal(err1, "Error: [node-ibm_db] Input file " + inputfile30 + " does not exists");
});
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
    try {
      conn.querySync("drop table sample");
      conn.querySync("drop table sample1");
    } catch(e) {}

    var inputfile = __dirname + '/data/sample1.txt';
    conn.executeFile(inputfile, function (err, res) {
        if (err) {
            console.log(err);
        } else {
            console.log(res);
        }
        assert.deepEqual(res, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}];[{"NO":1},{"NO":2}];');
    });

    conn.executeFile(inputfile, ';', function (err, res1) {
        if (err) {
            console.log(err);
        } else {
            console.log(res1);
        }
        assert.deepEqual(res1, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}];[{"NO":1},{"NO":2}];');
    });

    var outputfile1 = __dirname + '/data/out1.txt';
    conn.executeFile(inputfile, ';', outputfile1, function (err) {
        if (err) {
            console.log(err)
        }
        else {
            fs.readFile(outputfile1, function (err, result1) {
                if (err) {
                    console.log(err);
                } else {
                    console.log(result1)
                }
                result11 = result1.toString();
                assert.deepEqual(result11, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}];[{"NO":1},{"NO":2}];');
                fs.stat((outputfile1), function (err) {
                    if (!err) {
                        fs.unlink((outputfile1), function (err) {
                            if (err) {
                                console.log("error1")
                            }
                        });
                    }
                });
            });
        }
    });

    var inputfile2 = __dirname + '/data/sample2.txt';
    conn.executeFile(inputfile2, '%', function (err, res2) {
        if (err) {
            console.log(err);
        } else {
            console.log(res2);
        }
        assert.deepEqual(res2, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]%[{"NO":1},{"NO":2}]%');
    });



    var outputfile2 = __dirname + '/data/out2.txt';
    conn.executeFile(inputfile2, '%', outputfile2, function (err) {
        if (err) {
            console.log(err)
        }
        else {
            fs.readFile(outputfile2, function (err, result2) {
                if (err) {
                    console.log(err);
                } else {
                    console.log(result2)
                }
                result12 = result2.toString();
                assert.deepEqual(result12, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]%[{"NO":1},{"NO":2}]%');
                fs.stat((outputfile2), function (err) {
                    if (!err) {
                        fs.unlink((outputfile2), function (err) {
                            if (err) {
                                console.log("error1")
                            }
                        });
                    }
                });
            });
        }
    });

    var inputfile3 = __dirname + '/data/sample3.txt';
    conn.executeFile(inputfile3, '@', function (err, res3) {
        if (err) {
            console.log(err);
        } else {
            console.log(res3);
        }
        assert.deepEqual(res3, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]@[{"NO":1},{"NO":2}]@');
    });


    var outputfile3 = __dirname + '/data/out3.txt';
    conn.executeFile(inputfile3, '@', outputfile3, function (err) {
        if (err) {
            console.log(err)
        }
        else {
            fs.readFile(outputfile3, function (err, result3) {
                if (err) {
                    console.log(err);
                } else {
                    console.log(result3)
                }
                result13 = result3.toString();
                assert.deepEqual(result13, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]@[{"NO":1},{"NO":2}]@');
                fs.stat((outputfile3), function (err) {
                    if (!err) {
                        fs.unlink((outputfile3), function (err) {
                            if (err) {
                                console.log("error1")
                            }
                        });
                    }
                });
            });
        }
    });

    var outputfile4 = __dirname + '/data/abc1/out3.txt';
    conn.executeFile(inputfile3, '@', outputfile4, function (err) {
        if (err) {
            console.log(err)
        }
        else {
            fs.readFile(outputfile4, function (err, result4) {
                if (err) {
                    console.log(err);
                } else {
                    console.log(result4)
                }
                result14 = result4.toString();
                assert.deepEqual(result14, '[{"NO":1,"NAME":"pri"},{"NO":2,"NAME":"anbu"}]@[{"NO":1},{"NO":2}]@');
                fs.stat((outputfile4), function (err) {
                    if (!err) {
                        fs.unlink((outputfile4), function (err) {
                            if (err) {
                                console.log("error1")
                            }
                            fs.rmdirSync(__dirname + '/data/abc1')
                        });
                    }
                });
            });
        }
    });
});

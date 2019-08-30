var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString;

ibmdb.open(cn, function (err, conn) {
    if (err) {
        return console.log(err);
    }

    conn.querySync("create table closeTest(col1 varchar(10))");
    conn.prepare("insert into closeTest values(?)", function (err, stmt) {
        if (err) {
            console.log(err);
            return conn.closeSync();
        }
        stmt.execute(['hi'], function (err, result) {
            if (err) console.log(err);
            conn.querySync("drop table closeTest");
            stmt.close(0, function (err) {
                if (err) {
                    console.log(err)
                }
                assert.equal(err, null);
            });
        });
    });

    conn.querySync("create table closeTest1(col1 varchar(10))");
    conn.prepare("insert into closeTest1 values(?)", function (err, stmt) {
        if (err) {
            console.log(err);
            return conn.closeSync();
        }
        stmt.execute(['hi'], function (err, result) {
            if (err) console.log(err);
            conn.querySync("drop table closeTest1");
            stmt.close(1, function (err) {
                if (err) {
                    console.log(err)
                }
                assert.equal(err, null);
            });
        });
    });

    conn.querySync("create table closeTest2(col1 varchar(10))");
    conn.prepare("insert into closeTest2 values(?)", function (err, stmt) {
        if (err) {
            console.log(err);
            return conn.closeSync();
        }
        stmt.execute(['hi'], function (err, result) {
            if (err) console.log(err);
            conn.querySync("drop table closeTest2");
            stmt.close(2, function (err) {
                if (err) {
                    console.log(err)
                }
                assert.equal(err, null);
            });
        });
    });


    conn.querySync("create table closeTest3(col1 varchar(10))");
    conn.prepare("insert into closeTest3 values(?)", function (err, stmt) {
        if (err) {
            console.log(err);
            return conn.closeSync();
        }
        stmt.execute(['hi'], function (err, result) {
            if (err) console.log(err);
            conn.querySync("drop table closeTest3");
            stmt.close(3, function (err) {
                if (err) {
                    console.log(err)
                }
                assert.equal(err, null);
            });
        });
    });


    conn.querySync("create table closeTest4(col1 varchar(10))");
    conn.prepare("insert into closeTest4 values(?)", function (err, stmt) {
        if (err) {
            console.log(err);
            return conn.closeSync();
        }
        stmt.execute(['hi'], function (err, result) {
            if (err) console.log(err);
            conn.querySync("drop table closeTest4");
            stmt.close(function (err) {
                if (err) {
                    console.log(err)
                }
                assert.equal(err, null);
            });
        });
    });


    conn.querySync("create table closeTest5(col1 varchar(10))");
    conn.prepare("insert into closeTest5 values(?)", function (err, stmt) {
        if (err) {
            console.log(err);
            return conn.closeSync();
        }
        stmt.execute(['hi'], function (err, result) {
            if (err) console.log(err);
            conn.querySync("drop table closeTest5");
            stmt.close(99, function (err) {
                assert.deepEqual(err.message, "[IBM][CLI Driver] CLI0133E  Option type out of range. SQLSTATE=HY092");
            });
        });
    });
});
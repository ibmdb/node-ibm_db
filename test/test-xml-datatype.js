var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , cn = common.connectionString
    , isZOS = common.isZOS
    , schema = common.connectionObject.CURRENTSCHEMA;

var query = "CaLL " + schema + ".PROC1(?, ?, ?)";
ibmdb.open(cn, function (err, conn) {
    if (err) console.log(err);
    assert.equal(err, null);
    try {
        if (isZOS) {
            // The DROP PROCEDURE syntax on z/OS can only specify the name of the procedure to drop.
            conn.querySync("drop procedure " + schema + ".PROC1");
        } else {
            conn.querySync("drop procedure " + schema + ".PROC1(INT, INT, XML)");
        }
    } catch (e) { }

    if (isZOS) {
        // CREATE OR REPLACE syntax is not supported on z/OS.
        // When creating an SQL native procedure on z/OS, you will need to have a WLM environment
        // defined for your system if you want to run the procedure in debugging mode.
        // Adding "disable debug mode" to bypass this requirement.
        conn.querySync("create procedure " + schema + ".PROC1 " +
            "(IN v1 INTEGER, OUT v2 INTEGER, INOUT v3 XML) " +
            "disable debug mode " +
            "BEGIN set v2 = v1 + 1; set v3 = XMLPARSE(DOCUMENT'<name> xml </name>'); END");
    } else {
        conn.querySync("create or replace procedure " + schema + ".PROC1 " +
            "(IN v1 INTEGER, OUT v2 INTEGER, INOUT v3 XML) " +
            "BEGIN set v2 = v1 + 1; set v3 = XMLPARSE(DOCUMENT'<name> xml </name>'); END");
    }
    var param1 = { ParamType: "INPUT", DataType: 1, Data: 0 };
    var param2 = { ParamType: "OUTPUT", DataType: 1, Data: 0 };
    var param3 = { ParamType: "INOUT", DataType: "XML", Data: "<name> xml </name>", Length: 200  };

    result = conn.querySync(query, [param1, param2, param3]);
    console.log("Output Parameters V2 = ", result[0], ", V3 = ", result[1]);

    if (isZOS) {
        // The DROP PROCEDURE syntax on z/OS can only specify the name of the procedure to drop.
        conn.querySync("drop procedure " + schema + ".PROC1");
    } else {
        conn.querySync("drop procedure " + schema + ".PROC1(INT, INT, XML)");
    }
    if (isZOS) {
        // CREATE OR REPLACE syntax is not supported on z/OS.  Drop PROCEDURE first.
        // When creating an SQL native procedure on z/OS, you will need to have a WLM environment
        // defined for your system if you want to run the procedure in debugging mode.
        // Adding "disable debug mode" to bypass this requirement.
        try {
            conn.querySync("drop procedure " + schema + ".PROC2");
        } catch (e) { }
        conn.querySync("create procedure " + schema + ".PROC2 (IN v1 INTEGER) disable debug mode BEGIN END");
    } else {
        conn.querySync("create or replace procedure " + schema + ".PROC2 (IN v1 INTEGER) BEGIN END");
    }
    result = conn.querySync("call " + schema + ".PROC2(?)", [param1]);
    assert.deepEqual(result, []);
    if (isZOS) {
        // The DROP PROCEDURE syntax on z/OS can only specify the name of the procedure to drop.
        conn.querySync("drop procedure " + schema + ".PROC2");
    } else {
        conn.querySync("drop procedure " + schema + ".PROC2(INT)");
    }
    conn.closeSync();
    console.log('done');
});

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , schema = common.connectionObject.CURRENTSCHEMA;


if (common.isZOS) {
  // Does not support ARRAY Data Type in CALL stmt.
  return 0;
}

ibmdb.open(common.connectionString, function (err, conn) {
    if(err) {
      console.log(err);
      process.exit(-1);
    }
    var arr1 = [10, 20, 30, 40, 50];
    var arr2 = ["Row 10", "Row 20", "Row 30", "Row 40", "Row 50"];
    var param1= { ParamType: "ARRAY", DataType: "INTEGER", Data: arr1 } ;
    var param2= { ParamType: "ARRAY", DataType: "VARCHAR", Data: arr2, Length:6 } ;
    var queryOptions = {sql: "CALL ARRAYPROCEDURE(?, ?)", 
                        params: [param1, param2],
                        ArraySize: 5};

    try { conn.querySync("drop table arrtab2;"); }
    catch (e) { }

    conn.beginTransactionSync();
    var ret = conn.querySync("create table arrtab2(c1 int, c2 varchar(10));");
    console.log("crete table ret = ", ret);
    ret  = conn.querySync("CREATE PROCEDURE ARRAYPROCEDURE (" +
       "IN arr1 INT, IN arr2 VARCHAR(10)) LANGUAGE SQL BEGIN " +
       "INSERT INTO ARRTAB2 VALUES (arr1, arr2);" +
       "END");
    console.log("create procedure ret = ", ret);

    // Call SP Synchronously.
    ret = conn.querySync(queryOptions);
    var result = conn.querySync("select * from arrtab2");
    console.log("Result for Select = ", result);
    var data = [
          { C1: 10, C2: 'Row 10' },
          { C1: 20, C2: 'Row 20' },
          { C1: 30, C2: 'Row 30' },
          { C1: 40, C2: 'Row 40' },
          { C1: 50, C2: 'Row 50' }
        ];

    assert.deepEqual(data, result);
    conn.rollbackTransaction();
    conn.closeSync();

});


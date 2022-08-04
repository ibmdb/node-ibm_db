var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , insertCount = 0;
  ;

//ibmdb.debug(2);
console.log("\n Test 1:\n =======\n");
ibmdb.open(common.connectionString, function(err, conn) {
  if(err) {
    console.log(err);
    return;
  }

  // Insert array data using querySync API
  // =====================================
  err = conn.querySync("create table arrtab (c1 int, c2 double, c3 smallint, c4 varchar(10))");
  if(err.length) { console.log(err); return; }
  err = conn.querySync("insert into arrtab values (9, 4.5, true, 'rocket')");
  if(err.length) { console.log(err); return; }
  var param1 = {ParamType:"ARRAY", DataType:1, Data:[null,5,6,7,8]};
  var param2 = {ParamType:"ARRAY", DataType:"DOUBLE", Data:[4.1,null,6.14,7,8.3]};
  var param3 = {ParamType:"ARRAY", DataType:1, Data:[0,1,null,false,true]};
  var namearr = ["", "Row 200", null, "Row 4000", "Last Row"];
  var param4 = {ParamType:"ARRAY", DataType:1, Data:namearr, Length:9};
  var queryOptions = {sql:"insert into arrtab values (?, ?, ?, ?)", 
                      params: [param1, param2, param3, param4],
                      ArraySize:5};

  conn.querySync(queryOptions);

  // Insert array data using prepare-bind-execute API
  // ================================================
  conn.querySync("create table arrtab2 (c1 int, c2 varchar(10))");
  var param5 = {ParamType:"ARRAY", DataType:1, Data:[10,20,30,40]};
  var namearr = ["Row 10", "Row 2000", "Row 30", "Last Row"];
  var param6 = {ParamType:"ARRAY", DataType:1, Data:namearr};

  //Note: param4 uses "Length" key, but param6 do not use Length key
  var tab2data =  [
        { C1: 10, C2: 'Row 10' },
        { C1: 20, C2: 'Row 20' },
        { C1: 30, C2: 'Row 30' },
        { C1: 40, C2: 'Last R' },
        { C1: 10, C2: 'Row 10' },
        { C1: 20, C2: 'Row 20' },
        { C1: 30, C2: 'Row 30' },
        { C1: 40, C2: 'Last R' }
      ];

  var stmt = conn.prepareSync("insert into arrtab2 values (?, ?)");
  stmt.bindSync([param5, param6]);
  stmt.setAttrSync(ibmdb.SQL_ATTR_PARAMSET_SIZE, 4);
  var result = stmt.executeSync();
  stmt.closeSync();

  // Insert array data using query API
  // =====================================
  conn.query(queryOptions, function(err, result) {
    if(err) console.log("Error in query,", err);
    else {
      var data = conn.querySync('select c1, c2, c3, c4, length(c4) "C4LEN" from arrtab');
      console.log("\nSelected data for table ARRTAB =\n", data);
      conn.querySync("drop table arrtab");

      // Insert array data using prepare-bind-execute API
      // ================================================
      conn.prepare("insert into arrtab2 values (?, ?)", function(err, stmt) {
        if(err) { console.log(err); stmt.closeSync(); }
        else {
          stmt.setAttr(ibmdb.SQL_ATTR_PARAMSET_SIZE, 4, function(err, result) {
            if(err) { console.log(err); stmt.closeSync(); }
            else {
              stmt.execute([param5, param6], function(err, result) {
                if(err) console.log(err);
                stmt.closeSync();
                var data = conn.querySync("select * from arrtab2");
                console.log("\nSelected data for table ARRTAB2 =\n", data);
                conn.querySync("drop table arrtab2");
                assert.deepEqual(data, tab2data);
                conn.closeSync();
                testEmptyArryElement();
              });
            }
          });
        }
      });
    }
  });
});

// Test array insert when one of the element in array is empty.
function testEmptyArryElement() {
console.log("\n Test 2:\n =======\n");
ibmdb.open(common.connectionString, function(err, conn) {
  if(err) {
    console.log(err);
    return;
  }

  // Insert array data using querySync API
  // =====================================
  err = conn.querySync("create table arrtab3 (c1 varchar(10))");
  if(err.length) { console.log(err); return; }
  var params1 = [{ParamType:"ARRAY", DataType:1, Data: ["","Hello"], Length:5}];
  var params2 = [{ParamType:"ARRAY", DataType:1, Data: ["Hello",""], Length:5}];
  var queryOptions1 = {sql:"insert into arrtab3 values (?)", 
                      params: [{ParamType:"ARRAY", DataType:1, Data: ["","Hello"], Length:5}],
                      ArraySize:2};
  var queryOptions2 = {sql:"insert into arrtab3 values (?)", 
                      params: [{ParamType:"ARRAY", DataType:1, Data: ["Hello"," "], Length:5}],
                      ArraySize:2};

  conn.querySync(queryOptions1);
  conn.querySync(queryOptions2);

  // Insert array data using prepare-bind-execute API
  // ================================================
  conn.querySync("create table arrtab4 (c1 varchar(10))");

  var tab4data =  [
        { C1: '' },
        { C1: 'Hello' },
        { C1: 'Hello' },
        { C1: '' }
      ];

  var stmt = conn.prepareSync("insert into arrtab4 values (?)");
  stmt.bindSync(params1);
  stmt.setAttrSync(ibmdb.SQL_ATTR_PARAMSET_SIZE, 2);
  var result = stmt.executeSync();
  stmt.closeSync();

  // Insert array data using query API
  // =====================================
  conn.query(queryOptions1, function(err, result) {
    if(err) console.log(err);
    else {
      var data = conn.querySync('select c1, length(c1) "C1LEN" from arrtab3');
      console.log("\nSelected data for table ARRTAB3 =\n", data);
      console.log("drop table result = ", conn.querySync("drop table arrtab3"));
      var expectedData3 = [
          { C1: '', C1LEN: 0 },
          { C1: 'Hello', C1LEN: 5 },
          { C1: 'Hello', C1LEN: 5 },
          { C1: ' ', C1LEN: 1 },
          { C1: '', C1LEN: 0 },
          { C1: 'Hello', C1LEN: 5 }
        ];
      assert.deepEqual(data, expectedData3);

      // Insert array data using prepare-bind-execute API
      // ================================================
      conn.prepare("insert into arrtab4 values (?)", function(err, stmt) {
        if(err) { console.log("error in prepare,", err); stmt.closeSync(); }
        else {
          stmt.setAttr(ibmdb.SQL_ATTR_PARAMSET_SIZE, 2, function(err, result) {
            if(err) { console.log("error in setAttr,", err); stmt.closeSync(); }
            else {
              stmt.execute(params2, function(err, result) {
                if(err) console.log("error in execute,", err);
                stmt.closeSync();
                var data = conn.querySync("select * from arrtab4");
                console.log("\nSelected data for table ARRTAB4 =\n", data);
                conn.querySync("drop table arrtab4");
                assert.deepEqual(data, tab4data);
                conn.closeSync();
              });
            }
          });
        }
      });
    }
  });
});
}

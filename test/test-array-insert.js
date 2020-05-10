var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , insertCount = 0;
  ;

ibmdb.debug(true);
ibmdb.open(common.connectionString, function(err, conn) {
  if(err) {
    console.log(err);
    return;
  }

  // Insert array data using querySync API
  // =====================================
  conn.querySync("create table arrtab (c1 int, c2 double, c3 boolean, c4 varchar(10))");
  conn.querySync("insert into arrtab values (9, 4.5, true, 'rocket')");
  var param1 = {ParamType:"ARRAY", DataType:1, Data:[4,5,6,7,8]};
  var param2 = {ParamType:"ARRAY", DataType:"DOUBLE", Data:[4.1,5.3,6.14,7,8.3]};
  var param3 = {ParamType:"ARRAY", DataType:1, Data:[0,1,false,true,0]};
  var namearr = ["Row 10", "Row 20", "Row 30", "Row 40", "Last Row"];
  var param4 = {ParamType:"ARRAY", DataType:1, Data:namearr, Length:9};
  var queryOptions = {sql:"insert into arrtab values (?, ?, ?, ?)", 
                      params: [param1, param2, param3, param4],
                      ArraySize:5};

  conn.querySync(queryOptions);

  // Insert array data using prepare-bind-execute API
  // ================================================
  conn.querySync("create table arrtab2 (c1 int, c2 varchar(10))");
  var param5 = {ParamType:"ARRAY", DataType:1, Data:[10,20,30,40]};
  var namearr = ["Row 10", "Row 20", "Row 30", "Last Row"];
  var param6 = {ParamType:"ARRAY", DataType:1, Data:namearr, Length:9};

  var stmt = conn.prepareSync("insert into arrtab2 values (?, ?)");
  stmt.bindSync([param5, param6]);
  stmt.setAttrSync(ibmdb.SQL_ATTR_PARAMSET_SIZE, 4);
  var result = stmt.executeSync();
  stmt.closeSync();

  // Insert array data using query API
  // =====================================
  conn.query(queryOptions, function(err, result) {
    if(err) console.log(err);
    else {
      var data = conn.querySync("select * from arrtab");
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
                conn.closeSync();
              });
            }
          });
        }
      });
    }
  });
});


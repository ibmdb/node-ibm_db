var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  ;

// Test row-wise array insert feature (issue #698)
// This allows users to pass data as rows instead of column-wise arrays.

console.log("\n Test 1: Row-wise array insert using querySync\n ==============================================\n");
ibmdb.open(common.connectionString, function(err, conn) {
  if(err) {
    console.log(err);
    return;
  }

  // Clean up any leftover tables from previous failed runs
  conn.querySync("drop table rowarrtab");
  conn.querySync("drop table rowarrtab2");
  conn.querySync("drop table rowarrtab3");
  conn.querySync("drop table rowarrtab4");

  // Create test table
  err = conn.querySync("create table rowarrtab (c1 int, c2 double, c3 varchar(10))");
  if(err.length) { console.log(err); return; }
  conn.querySync("insert into rowarrtab values (9, 4.5, 'existing')");

  // Row-wise array insert using querySync with rows option
  var queryOptions = {
    sql: "insert into rowarrtab values (?, ?, ?)",
    rows: [
      [1, 10.5, "Row 1"],
      [2, 20.3, "Row 2"],
      [3, 30.7, "Row 3"]
    ]
  };
  conn.querySync(queryOptions);

  var data = conn.querySync("select * from rowarrtab order by c1");
  console.log("Selected data after row-wise querySync =\n", data);
  var expectedData = [
    { C1: 1, C2: 10.5, C3: 'Row 1' },
    { C1: 2, C2: 20.3, C3: 'Row 2' },
    { C1: 3, C2: 30.7, C3: 'Row 3' },
    { C1: 9, C2: 4.5, C3: 'existing' }
  ];
  assert.deepEqual(data, expectedData);
  console.log("Test 1 passed: querySync with rows option works!\n");

  // Test 2: Row-wise array insert using query (async) with rows option
  console.log(" Test 2: Row-wise array insert using async query\n ================================================\n");
  var queryOptions2 = {
    sql: "insert into rowarrtab values (?, ?, ?)",
    rows: [
      [4, 40.1, "Row 4"],
      [5, 50.2, "Row 5"]
    ]
  };
  conn.query(queryOptions2, function(err, result) {
    if(err) { console.log("Error in async query:", err); return; }

    var data2 = conn.querySync("select * from rowarrtab order by c1");
    console.log("Selected data after async row-wise query =\n", data2);
    assert.equal(data2.length, 6); // 1 existing + 3 from test1 + 2 from test2
    assert.equal(data2[3].C1, 4);
    assert.equal(data2[4].C1, 5);
    console.log("Test 2 passed: async query with rows option works!\n");

    // Test 3: Row-wise insert with columns metadata
    console.log(" Test 3: Row-wise insert with columns metadata\n ==============================================\n");
    conn.querySync("create table rowarrtab2 (c1 int, c2 varchar(20))");
    var queryOptions3 = {
      sql: "insert into rowarrtab2 values (?, ?)",
      columns: [
        { DataType: "INTEGER" },
        { DataType: "VARCHAR", Length: 20 }
      ],
      rows: [
        [100, "Hello World"],
        [200, "Goodbye World"]
      ]
    };
    conn.querySync(queryOptions3);
    var data3 = conn.querySync("select * from rowarrtab2 order by c1");
    console.log("Selected data with columns metadata =\n", data3);
    assert.deepEqual(data3, [
      { C1: 100, C2: 'Hello World' },
      { C1: 200, C2: 'Goodbye World' }
    ]);
    console.log("Test 3 passed: rows with columns metadata works!\n");

    // Test 4: Row-wise insert with null values
    console.log(" Test 4: Row-wise insert with null values\n =========================================\n");
    conn.querySync("create table rowarrtab3 (c1 int, c2 varchar(10))");
    var queryOptions4 = {
      sql: "insert into rowarrtab3 values (?, ?)",
      rows: [
        [1, null],
        [null, "notnull"],
        [3, "three"]
      ]
    };
    conn.querySync(queryOptions4);
    var data4 = conn.querySync("select * from rowarrtab3 order by c1");
    console.log("Selected data with null values =\n", data4);
    assert.equal(data4.length, 3);
    console.log("Test 4 passed: rows with null values works!\n");

    // Test 5: convertRowsToColumns utility function
    console.log(" Test 5: convertRowsToColumns utility\n =====================================\n");
    var converted = ibmdb.convertRowsToColumns(
      [[10, "a"], [20, "b"], [30, "c"]],
      [{ DataType: 1 }, { DataType: "VARCHAR", Length: 5 }]
    );
    assert.ok(!converted.error, "No error expected");
    assert.equal(converted.ArraySize, 3);
    assert.equal(converted.params.length, 2);
    assert.equal(converted.params[0].ParamType, "ARRAY");
    assert.deepEqual(converted.params[0].Data, [10, 20, 30]);
    assert.deepEqual(converted.params[1].Data, ["a", "b", "c"]);
    assert.equal(converted.params[1].Length, 5);
    console.log("Converted result =", JSON.stringify(converted, null, 2));
    console.log("Test 5 passed: convertRowsToColumns utility works!\n");

    // Test 6: Error handling - mismatched row lengths
    console.log(" Test 6: Error handling\n ======================\n");
    var bad1 = ibmdb.convertRowsToColumns([[1, 2], [3]]);
    assert.ok(bad1.error, "Expected error for mismatched row lengths");
    console.log("Mismatched rows error:", bad1.error);

    var bad2 = ibmdb.convertRowsToColumns([]);
    assert.ok(bad2.error, "Expected error for empty rows");
    console.log("Empty rows error:", bad2.error);

    var bad3 = ibmdb.convertRowsToColumns([[1, 2]], [{ DataType: 1 }]);
    assert.ok(bad3.error, "Expected error for columns/rows mismatch");
    console.log("Columns mismatch error:", bad3.error);
    console.log("Test 6 passed: Error handling works!\n");

    // Test 7: prepare/bind/execute with convertRowsToColumns
    console.log(" Test 7: prepare/bind/execute with convertRowsToColumns\n ======================================================\n");
    conn.querySync("create table rowarrtab4 (c1 int, c2 varchar(10))");
    var converted2 = ibmdb.convertRowsToColumns(
      [[10, "ten"], [20, "twenty"], [30, "thirty"]]
    );
    var stmt = conn.prepareSync("insert into rowarrtab4 values (?, ?)");
    stmt.bindSync(converted2.params);
    stmt.setAttrSync(ibmdb.SQL_ATTR_PARAMSET_SIZE, converted2.ArraySize);
    var result = stmt.executeSync();
    stmt.closeSync();
    var data7 = conn.querySync("select * from rowarrtab4 order by c1");
    console.log("Selected data from prepare/bind/execute =\n", data7);
    assert.deepEqual(data7, [
      { C1: 10, C2: 'ten' },
      { C1: 20, C2: 'twenty' },
      { C1: 30, C2: 'thirty' }
    ]);
    console.log("Test 7 passed: prepare/bind/execute with convertRowsToColumns works!\n");

    // Cleanup
    conn.querySync("drop table rowarrtab");
    conn.querySync("drop table rowarrtab2");
    conn.querySync("drop table rowarrtab3");
    conn.querySync("drop table rowarrtab4");
    conn.closeSync();
    console.log("\nAll row-wise array insert tests passed!\n");
  });
});

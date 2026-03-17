// Test for issue #977:
// When SQL_ATTR_ROW_ARRAY_SIZE > 1, ibm_db should use SQLBindCol-based
// block fetch to correctly return all rows. With 20 rows and
// ROW_ARRAY_SIZE=5, all 20 rows must be returned.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString
  ;

var tableName = "TEST_ISSUE977";

ibmdb.open(cn, function(err, conn) {
  if (err) { console.log(err); return; }
  console.log("Connection opened successfully.");

  // Drop table if exists
  try { conn.querySync("DROP TABLE " + tableName); } catch(e) {}

  // Create table and insert 20 rows
  conn.querySync("CREATE TABLE " + tableName +
    " (ID INTEGER, NAME VARCHAR(20))");
  for (var i = 1; i <= 20; i++) {
    conn.querySync("INSERT INTO " + tableName +
      " VALUES (" + i + ", 'Row" + i + "')");
  }
  console.log("Inserted 20 rows into " + tableName);

  // Verify all 20 rows exist using normal querySync (no ROW_ARRAY_SIZE)
  var allRows = conn.querySync("SELECT * FROM " + tableName + " ORDER BY ID");
  console.log("\nTest 0: querySync without ROW_ARRAY_SIZE = " +
    allRows.length + " rows (expected 20)");
  assert.equal(allRows.length, 20);

  testFetchAllSync();

  function testFetchAllSync() {
    // Test 1: prepare + setAttrSync(ROW_ARRAY_SIZE, 5) + fetchAllSync
    console.log("\nTest 1: fetchAllSync with SQL_ATTR_ROW_ARRAY_SIZE = 5");
    var stmt = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result = stmt.executeSync();
    var data = result.fetchAllSync();
    console.log("  Returned " + data.length + " rows (expected 20)");
    assert.equal(data.length, 20, "fetchAllSync should return 20 rows");
    // Verify all rows are present and in order
    for (var i = 0; i < 20; i++) {
      assert.equal(data[i].ID, i + 1,
        "Row " + i + " should have ID=" + (i + 1));
      assert.equal(data[i].NAME, "Row" + (i + 1));
    }
    console.log("  Data = ", data);
    console.log("  PASS: All 20 rows correctly returned with proper data.");
    result.closeSync();
    stmt.closeSync();

    testFetchAll();
  }

  function testFetchAll() {
    // Test 2: prepare + setAttrSync(ROW_ARRAY_SIZE, 5) + async fetchAll
    console.log("\nTest 2: async fetchAll with SQL_ATTR_ROW_ARRAY_SIZE = 5");
    var stmt2 = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt2.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result2 = stmt2.executeSync();

    result2.fetchAll(function(err, data2) {
      if (err) {
        console.log("  ERROR:", err);
        assert.fail("fetchAll should not error");
      }
      console.log("  Returned " + data2.length + " rows (expected 20)");
      assert.equal(data2.length, 20, "fetchAll should return 20 rows");
      for (var i = 0; i < 20; i++) {
        assert.equal(data2[i].ID, i + 1);
      }
      console.log("  PASS: All 20 rows correctly returned.");
      result2.closeSync();
      stmt2.closeSync();

      testFetchSync();
    });
  }

  function testFetchSync() {
    // Test 3: prepare + setAttrSync(ROW_ARRAY_SIZE, 5) + fetchSync loop
    // fetchSync returns an array of up to ROW_ARRAY_SIZE rows per call
    console.log("\nTest 3: fetchSync loop with SQL_ATTR_ROW_ARRAY_SIZE = 5");
    var stmt3 = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt3.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result3 = stmt3.executeSync();

    var allRows = [];
    var blockCount = 0;
    var block;
    while ((block = result3.fetchSync()) !== null) {
      blockCount++;
      assert.ok(Array.isArray(block), "fetchSync should return an array when ROW_ARRAY_SIZE > 1");
      assert.ok(block.length <= 5, "Block should have at most ROW_ARRAY_SIZE rows");
      console.log("  Block " + blockCount + ": " + block.length + " rows");
      console.log("  Block data = ", block);
      for (var i = 0; i < block.length; i++) {
        allRows.push(block[i]);
      }
    }
    console.log("  Total: " + allRows.length + " rows in " + blockCount + " blocks (expected 20 rows, 4 blocks)");
    assert.equal(allRows.length, 20, "fetchSync should return 20 total rows");
    assert.equal(blockCount, 4, "Should fetch 4 blocks of 5 rows each");
    for (var i = 0; i < 20; i++) {
      assert.equal(allRows[i].ID, i + 1);
    }
    console.log("  PASS: All 20 rows correctly returned in 4 blocks of 5.");
    result3.closeSync();
    stmt3.closeSync();

    testFetchNSync();
  }

  function testFetchNSync() {
    // Test 4: prepare + setAttrSync(ROW_ARRAY_SIZE, 5) + fetchNSync
    console.log("\nTest 4: fetchNSync(10) with SQL_ATTR_ROW_ARRAY_SIZE = 5");
    var stmt4 = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt4.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result4 = stmt4.executeSync();

    var batch1 = result4.fetchNSync(10);
    console.log("  Batch 1: " + batch1.length + " rows (expected 10)");
    assert.equal(batch1.length, 10, "fetchNSync(10) should return 10 rows");
    console.log("  Batch 1 data = ", batch1);
    for (var i = 0; i < 10; i++) {
      assert.equal(batch1[i].ID, i + 1);
    }

    var batch2 = result4.fetchNSync(10);
    console.log("  Batch 2: " + batch2.length + " rows (expected 10)");
    assert.equal(batch2.length, 10, "fetchNSync(10) should return 10 rows");
    console.log("  Batch 2 data = ", batch2);
    for (var i = 0; i < 10; i++) {
      assert.equal(batch2[i].ID, i + 11);
    }

    var batch3 = result4.fetchNSync(10);
    console.log("  Batch 3: " + batch3.length + " rows (expected 0)");
    assert.equal(batch3.length, 0, "fetchNSync(10) after all rows should return 0");

    console.log("  PASS: fetchNSync correctly batched all 20 rows.");
    result4.closeSync();
    stmt4.closeSync();

    testFetchArrayMode();
  }

  function testFetchArrayMode() {
    // Test 5: fetchAllSync with FETCH_ARRAY mode and ROW_ARRAY_SIZE=5
    console.log("\nTest 5: fetchAllSync FETCH_ARRAY with SQL_ATTR_ROW_ARRAY_SIZE = 5");
    var stmt5 = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt5.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result5 = stmt5.executeSync();
    var data5 = result5.fetchAllSync({fetchMode: 3}); // FETCH_ARRAY = 3
    console.log("  Returned " + data5.length + " rows (expected 20)");
    assert.equal(data5.length, 20, "fetchAllSync FETCH_ARRAY should return 20 rows");
    assert.equal(data5[0][0], 1, "First row ID should be 1");
    assert.equal(data5[0][1], "Row1", "First row NAME should be 'Row1'");
    assert.equal(data5[19][0], 20, "Last row ID should be 20");
    console.log("  PASS: All 20 rows correctly returned as arrays.");
    result5.closeSync();
    stmt5.closeSync();

    testFetchAsync();
  }

  function testFetchAsync() {
    // Test 6: async fetch() (callback) with ROW_ARRAY_SIZE=5
    // fetch() returns an array of up to ROW_ARRAY_SIZE rows per callback
    console.log("\nTest 6: async fetch() callback with SQL_ATTR_ROW_ARRAY_SIZE = 5");
    var stmt6 = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt6.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result6 = stmt6.executeSync();
    var allRows6 = [];
    var blockCount6 = 0;

    function fetchNext() {
      result6.fetch(function(err, block) {
        if (err) {
          console.log("  ERROR:", err);
          assert.fail("fetch() should not error");
          return;
        }
        if (block) {
          blockCount6++;
          assert.ok(Array.isArray(block), "fetch() should return an array when ROW_ARRAY_SIZE > 1");
          assert.ok(block.length <= 5, "Block should have at most ROW_ARRAY_SIZE rows");
          console.log("  Block " + blockCount6 + ": " + block.length + " rows");
          console.log("  Block data = ", block);
          for (var i = 0; i < block.length; i++) {
            allRows6.push(block[i]);
          }
          fetchNext();
        } else {
          console.log("  Total: " + allRows6.length + " rows in " + blockCount6 + " blocks (expected 20 rows, 4 blocks)");
          assert.equal(allRows6.length, 20, "fetch() should return 20 total rows");
          assert.equal(blockCount6, 4, "Should fetch 4 blocks of 5 rows each");
          for (var i = 0; i < 20; i++) {
            assert.equal(allRows6[i].ID, i + 1);
          }
          console.log("  PASS: All 20 rows correctly returned via async fetch() in 4 blocks.");
          result6.closeSync();
          stmt6.closeSync();

          testFetchNAsync();
        }
      });
    }
    fetchNext();
  }

  function testFetchNAsync() {
    // Test 7: async fetchN() (callback) with ROW_ARRAY_SIZE=5
    console.log("\nTest 7: async fetchN() callback with SQL_ATTR_ROW_ARRAY_SIZE = 5");
    var stmt7 = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt7.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result7 = stmt7.executeSync();

    result7.fetchN(10, function(err, batch1) {
      if (err) { console.log("  ERROR:", err); assert.fail("fetchN should not error"); }
      console.log("  Batch 1: " + batch1.length + " rows (expected 10)");
      assert.equal(batch1.length, 10, "fetchN(10) should return 10 rows");
      for (var i = 0; i < 10; i++) {
        assert.equal(batch1[i].ID, i + 1);
      }

      result7.fetchN(10, function(err, batch2) {
        if (err) { console.log("  ERROR:", err); assert.fail("fetchN should not error"); }
        console.log("  Batch 2: " + batch2.length + " rows (expected 10)");
        assert.equal(batch2.length, 10, "fetchN(10) should return 10 rows");
        for (var i = 0; i < 10; i++) {
          assert.equal(batch2[i].ID, i + 11);
        }

        result7.fetchN(10, function(err, batch3) {
          if (err) { console.log("  ERROR:", err); assert.fail("fetchN should not error"); }
          console.log("  Batch 3: " + batch3.length + " rows (expected 0)");
          assert.equal(batch3.length, 0, "fetchN(10) after all rows should return 0");

          console.log("  PASS: async fetchN() correctly batched all 20 rows.");
          result7.closeSync();
          stmt7.closeSync();

          testAsyncAwait();
        });
      });
    });
  }

  function testAsyncAwait() {
    // Test 8: async/await with all APIs and ROW_ARRAY_SIZE=5
    console.log("\nTest 8: async/await with SQL_ATTR_ROW_ARRAY_SIZE = 5");
    asyncAwaitTests().then(function() {
      cleanup();
    }).catch(function(err) {
      console.log("  async/await test error:", err);
      assert.fail("async/await tests should not error");
    });
  }

  async function asyncAwaitTests() {
    // 8a: fetchAll via Promise/await
    console.log("  8a: await fetchAll()");
    var stmt8a = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt8a.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result8a = stmt8a.executeSync();
    var data8a = await result8a.fetchAll();
    console.log("    Returned " + data8a.length + " rows (expected 20)");
    assert.equal(data8a.length, 20, "await fetchAll() should return 20 rows");
    for (var i = 0; i < 20; i++) {
      assert.equal(data8a[i].ID, i + 1);
    }
    console.log("    PASS");
    result8a.closeSync();
    stmt8a.closeSync();

    // 8b: fetch via Promise/await loop - returns array of rows per block
    console.log("  8b: await fetch() blocks");
    var stmt8b = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt8b.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result8b = stmt8b.executeSync();
    var rows8b = [];
    var blockCount8b = 0;
    var block8b;
    while ((block8b = await result8b.fetch()) !== null) {
      blockCount8b++;
      assert.ok(Array.isArray(block8b), "await fetch() should return an array when ROW_ARRAY_SIZE > 1");
      console.log("    Block data = ", block8b);
      for (var i = 0; i < block8b.length; i++) {
        rows8b.push(block8b[i]);
      }
    }
    console.log("    Returned " + rows8b.length + " rows in " + blockCount8b + " blocks (expected 20 rows, 4 blocks)");
    assert.equal(rows8b.length, 20, "await fetch() should return 20 total rows");
    assert.equal(blockCount8b, 4, "Should fetch 4 blocks of 5 rows each");
    for (var i = 0; i < 20; i++) {
      assert.equal(rows8b[i].ID, i + 1);
    }
    console.log("    PASS");
    result8b.closeSync();
    stmt8b.closeSync();

    // 8c: fetchN via Promise/await batches
    console.log("  8c: await fetchN() batches");
    var stmt8c = conn.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt8c.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result8c = stmt8c.executeSync();
    var b1 = await result8c.fetchN(7);
    console.log("    Batch 1: " + b1.length + " rows (expected 7)");
    assert.equal(b1.length, 7, "await fetchN(7) should return 7 rows");
    assert.equal(b1[0].ID, 1);
    assert.equal(b1[6].ID, 7);

    var b2 = await result8c.fetchN(7);
    console.log("    Batch 2: " + b2.length + " rows (expected 7)");
    assert.equal(b2.length, 7, "await fetchN(7) should return 7 rows");
    assert.equal(b2[0].ID, 8);
    assert.equal(b2[6].ID, 14);

    var b3 = await result8c.fetchN(7);
    console.log("    Batch 3: " + b3.length + " rows (expected 6)");
    assert.equal(b3.length, 6, "await fetchN(7) should return remaining 6 rows");
    assert.equal(b3[0].ID, 15);
    assert.equal(b3[5].ID, 20);

    var b4 = await result8c.fetchN(7);
    console.log("    Batch 4: " + b4.length + " rows (expected 0)");
    assert.equal(b4.length, 0, "await fetchN(7) after all rows should return 0");

    console.log("    PASS");
    result8c.closeSync();
    stmt8c.closeSync();

    // 8d: full async/await using ibmdb.open Promise
    console.log("  8d: full async/await with ibmdb.open");
    var conn2 = await ibmdb.open(cn);
    var stmt8d = conn2.prepareSync("SELECT * FROM " + tableName + " ORDER BY ID");
    stmt8d.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
    var result8d = stmt8d.executeSync();
    var data8d = await result8d.fetchAll();
    assert.equal(data8d.length, 20, "full async/await should return 20 rows");
    for (var i = 0; i < 20; i++) {
      assert.equal(data8d[i].ID, i + 1);
      assert.equal(data8d[i].NAME, "Row" + (i + 1));
    }
    console.log("    Returned " + data8d.length + " rows - PASS");
    result8d.closeSync();
    stmt8d.closeSync();
    await conn2.close();
    console.log("  PASS: All async/await tests passed.");
  }

  function cleanup() {
    conn.querySync("DROP TABLE " + tableName);
    conn.closeSync();
    console.log("\nAll tests passed! Connection closed.");
  }
});

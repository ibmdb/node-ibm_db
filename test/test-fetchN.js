var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  ;

// Test fetchN (batch fetch) feature (issue #977)
// Fetch rows in batches of N for efficient processing of large result sets.

ibmdb.open(common.connectionString, function(err, conn) {
  if(err) {
    console.log(err);
    return;
  }

  // Clean up any leftover table from previous failed runs
  conn.querySync("drop table fetchntab");

  // Create and populate test table
  conn.querySync("create table fetchntab (c1 int, c2 varchar(20))");
  for(var i = 1; i <= 10; i++) {
    conn.querySync("insert into fetchntab values (" + i + ", 'Row " + i + "')");
  }

  // Test 1: fetchNSync - fetch all 10 rows in batches of 3
  console.log("Test 1: fetchNSync in batches of 3");
  console.log("==================================");
  var result1 = conn.queryResultSync("select * from fetchntab order by c1");
  var allRows = [];
  var batch;

  batch = result1.fetchNSync(3);
  assert.equal(batch.length, 3, "First batch should have 3 rows");
  assert.equal(batch[0].C1, 1);
  assert.equal(batch[2].C1, 3);
  allRows = allRows.concat(batch);

  batch = result1.fetchNSync(3);
  assert.equal(batch.length, 3, "Second batch should have 3 rows");
  assert.equal(batch[0].C1, 4);
  assert.equal(batch[2].C1, 6);
  allRows = allRows.concat(batch);

  batch = result1.fetchNSync(3);
  assert.equal(batch.length, 3, "Third batch should have 3 rows");
  assert.equal(batch[0].C1, 7);
  assert.equal(batch[2].C1, 9);
  allRows = allRows.concat(batch);

  batch = result1.fetchNSync(3);
  assert.equal(batch.length, 1, "Fourth batch should have 1 remaining row");
  assert.equal(batch[0].C1, 10);
  allRows = allRows.concat(batch);

  batch = result1.fetchNSync(3);
  assert.equal(batch.length, 0, "Fifth batch should be empty (no more rows)");

  result1.closeSync();
  assert.equal(allRows.length, 10, "Total rows should be 10");
  console.log("All batches:", allRows.map(function(r) { return r.C1; }));
  console.log("All rows:", allRows);
  console.log("Test 1 passed: fetchNSync works!\n");

  // Test 2: fetchN (async) with Promise
  console.log("Test 2: fetchN (async) with Promise");
  console.log("====================================");
  conn.queryResult("select * from fetchntab order by c1", async function(err, result2) {
    if(err) { console.log(err); return; }

    var batch1 = await result2.fetchN(5);
    assert.equal(batch1.length, 5, "Async batch 1 should have 5 rows");
    assert.equal(batch1[0].C1, 1);
    assert.equal(batch1[4].C1, 5);
    console.log("Batch 1:", batch1.map(function(r) { return r.C1; }));

    var batch2 = await result2.fetchN(5);
    assert.equal(batch2.length, 5, "Async batch 2 should have 5 rows");
    assert.equal(batch2[0].C1, 6);
    assert.equal(batch2[4].C1, 10);
    console.log("Batch 2:", batch2.map(function(r) { return r.C1; }));
    console.log("Batch 2 rows:", batch2);

    var batch3 = await result2.fetchN(5);
    assert.equal(batch3.length, 0, "Async batch 3 should be empty");

    result2.closeSync();
    console.log("Test 2 passed: fetchN async with Promise works!\n");

    // Test 3: fetchN with callback
    console.log("Test 3: fetchN with callback");
    console.log("============================");
    conn.queryResult("select * from fetchntab order by c1", function(err, result3) {
      if(err) { console.log(err); return; }

      result3.fetchN(4, function(err, batch) {
        if(err) { console.log(err); return; }
        assert.equal(batch.length, 4, "Callback batch should have 4 rows");
        assert.equal(batch[0].C1, 1);
        assert.equal(batch[3].C1, 4);
        console.log("Callback batch:", batch.map(function(r) { return r.C1; }));

        result3.fetchN(100, function(err, batch2) {
          if(err) { console.log(err); return; }
          assert.equal(batch2.length, 6, "Remaining batch should have 6 rows");
          console.log("Remaining batch:", batch2.map(function(r) { return r.C1; }));

          result3.closeSync();
          console.log("Test 3 passed: fetchN with callback works!\n");

          // Test 4: fetchNSync with fetchMode FETCH_ARRAY (3)
          console.log("Test 4: fetchNSync with fetchMode FETCH_ARRAY");
          console.log("==============================================");
          var result4 = conn.queryResultSync("select c1, c2 from fetchntab order by c1");
          var arrBatch = result4.fetchNSync(2, {fetchMode: 3});
          assert.equal(arrBatch.length, 2, "Array mode batch should have 2 rows");
          assert.ok(Array.isArray(arrBatch[0]), "Row should be an array in FETCH_ARRAY mode");
          assert.equal(arrBatch[0][0], 1);
          assert.equal(arrBatch[1][0], 2);
          console.log("Array mode batch:", arrBatch);
          result4.closeSync();
          console.log("Test 4 passed: fetchNSync with FETCH_ARRAY works!\n");

          // Cleanup
          conn.querySync("drop table fetchntab");
          conn.closeSync();
          console.log("All fetchN tests passed!");
        });
      });
    });
  });
});

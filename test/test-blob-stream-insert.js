// Test for SQLPutData-based chunked LOB insert.
// Tests inserting BLOB data via array of Buffers (sync + async)
// and via Readable stream (async only).
//
// This uses the DATA_AT_EXEC parameter type internally, which
// uses SQLBindParameter with SQL_LEN_DATA_AT_EXEC + SQLPutData
// to send LOB data in chunks to the server.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , fs = require("fs")
  , path = require("path")
  , { Readable } = require("stream")
  , cn = common.connectionString
  ;

var tableName = "TEST_BLOB_STREAM_INSERT";
var dataDir = path.join(__dirname, "data");

// Source file for test data
var srcImage = path.join(dataDir, "phool.jpg");
var srcImageBuf = fs.readFileSync(srcImage);
var srcImageSize = srcImageBuf.length;

console.log("Source file: " + srcImage + " (" + srcImageSize + " bytes)");

ibmdb.open(cn, function(err, conn) {
  if (err) { console.log(err); process.exit(-1); }
  console.log("Connection opened.");

  // Setup table
  try { conn.querySync("DROP TABLE " + tableName); } catch(e) {}
  conn.querySync("CREATE TABLE " + tableName +
    " (ID INTEGER, PHOTO BLOB(1M), DESCR VARCHAR(100))");

  testSyncChunkedInsert();

  // ======================================================================
  // Test 1: Sync insert with array of Buffers via querySync
  // ======================================================================
  function testSyncChunkedInsert() {
    console.log("\n--- Test 1: querySync with array of Buffers ---");

    // Split the image into 64KB chunks
    var chunkSize = 65536;
    var chunks = [];
    for (var offset = 0; offset < srcImageBuf.length; offset += chunkSize) {
      var end = Math.min(offset + chunkSize, srcImageBuf.length);
      chunks.push(srcImageBuf.subarray(offset, end));
    }
    console.log("  Split into " + chunks.length + " chunks of ~" +
                chunkSize + " bytes");

    conn.querySync("INSERT INTO " + tableName +
      " (ID, PHOTO, DESCR) VALUES (?, ?, ?)",
      [
        1,
        { ParamType: "INPUT", DataType: "BLOB", Data: chunks },
        'chunked-sync'
      ]);

    // Verify by reading back
    var rows = conn.querySync("SELECT PHOTO, DESCR FROM " + tableName +
      " WHERE ID = 1");
    assert.equal(rows.length, 1, "Should return 1 row");
    assert.equal(rows[0].DESCR, 'chunked-sync');

    var photoData = rows[0].PHOTO;
    assert.ok(Buffer.isBuffer(photoData), "PHOTO should be a Buffer");
    assert.equal(photoData.length, srcImageSize,
      "PHOTO size should match source (" + photoData.length +
      " vs " + srcImageSize + ")");
    assert.ok(srcImageBuf.equals(photoData),
      "PHOTO content should match source exactly");

    console.log("  PASS: querySync with chunked Buffer array works.");

    testAsyncChunkedInsert();
  }

  // ======================================================================
  // Test 2: Async insert with array of Buffers via query()
  // ======================================================================
  function testAsyncChunkedInsert() {
    console.log("\n--- Test 2: query() with array of Buffers ---");

    var chunkSize = 32768;
    var chunks = [];
    for (var offset = 0; offset < srcImageBuf.length; offset += chunkSize) {
      var end = Math.min(offset + chunkSize, srcImageBuf.length);
      chunks.push(srcImageBuf.subarray(offset, end));
    }
    console.log("  Split into " + chunks.length + " chunks of ~" +
                chunkSize + " bytes");

    conn.query("INSERT INTO " + tableName +
      " (ID, PHOTO, DESCR) VALUES (?, ?, ?)",
      [
        2,
        { ParamType: "INPUT", DataType: "BLOB", Data: chunks },
        'chunked-async'
      ],
      function(err) {
        if (err) { console.log("ERROR:", err); assert.fail(); }

        var rows = conn.querySync("SELECT PHOTO, DESCR FROM " + tableName +
          " WHERE ID = 2");
        assert.equal(rows.length, 1);
        assert.equal(rows[0].DESCR, 'chunked-async');

        var photoData = rows[0].PHOTO;
        assert.ok(Buffer.isBuffer(photoData));
        assert.equal(photoData.length, srcImageSize);
        assert.ok(srcImageBuf.equals(photoData));

        console.log("  PASS: async query() with chunked Buffer array works.");

        testPrepareExecuteSync();
      });
  }

  // ======================================================================
  // Test 3: Sync prepare/bind/execute with array of Buffers
  // ======================================================================
  function testPrepareExecuteSync() {
    console.log("\n--- Test 3: prepareSync + executeSync with array of Buffers ---");

    var chunkSize = 100000;
    var chunks = [];
    for (var offset = 0; offset < srcImageBuf.length; offset += chunkSize) {
      var end = Math.min(offset + chunkSize, srcImageBuf.length);
      chunks.push(srcImageBuf.subarray(offset, end));
    }
    console.log("  Split into " + chunks.length + " chunks of ~" +
                chunkSize + " bytes");

    var stmt = conn.prepareSync("INSERT INTO " + tableName +
      " (ID, PHOTO, DESCR) VALUES (?, ?, ?)");
    stmt.executeSync([
      3,
      { ParamType: "INPUT", DataType: "BLOB", Data: chunks },
      'chunked-prepare-sync'
    ]);
    stmt.closeSync();

    var rows = conn.querySync("SELECT PHOTO, DESCR FROM " + tableName +
      " WHERE ID = 3");
    assert.equal(rows.length, 1);
    assert.equal(rows[0].DESCR, 'chunked-prepare-sync');
    assert.equal(rows[0].PHOTO.length, srcImageSize);
    assert.ok(srcImageBuf.equals(rows[0].PHOTO));

    console.log("  PASS: prepareSync + executeSync with chunks works.");

    testStreamInsert();
  }

  // ======================================================================
  // Test 4: Async insert with Readable stream via query()
  // ======================================================================
  function testStreamInsert() {
    console.log("\n--- Test 4: query() with Readable stream ---");

    var stream = fs.createReadStream(srcImage, { highWaterMark: 65536 });
    console.log("  Using fs.createReadStream with 64KB highWaterMark");

    conn.query("INSERT INTO " + tableName +
      " (ID, PHOTO, DESCR) VALUES (?, ?, ?)",
      [
        4,
        { ParamType: "INPUT", DataType: "BLOB",
          Data: stream, Length: srcImageSize },
        'stream-async'
      ],
      function(err) {
        if (err) { console.log("ERROR:", err); assert.fail(); }

        var rows = conn.querySync("SELECT PHOTO, DESCR FROM " + tableName +
          " WHERE ID = 4");
        assert.equal(rows.length, 1);
        assert.equal(rows[0].DESCR, 'stream-async');
        assert.equal(rows[0].PHOTO.length, srcImageSize);
        assert.ok(srcImageBuf.equals(rows[0].PHOTO));

        console.log("  PASS: async query() with Readable stream works.");

        testStreamExecute();
      });
  }

  // ======================================================================
  // Test 5: Async prepare/execute with Readable stream
  // ======================================================================
  function testStreamExecute() {
    console.log("\n--- Test 5: prepare + execute with Readable stream ---");

    var stream = fs.createReadStream(srcImage, { highWaterMark: 32768 });
    console.log("  Using fs.createReadStream with 32KB highWaterMark");

    var stmt = conn.prepareSync("INSERT INTO " + tableName +
      " (ID, PHOTO, DESCR) VALUES (?, ?, ?)");

    stmt.execute([
      5,
      { ParamType: "INPUT", DataType: "BLOB",
        Data: stream, Length: srcImageSize },
      'stream-execute'
    ],
    function(err) {
      if (err) { console.log("ERROR:", err); assert.fail(); }

      stmt.closeSync();

      var rows = conn.querySync("SELECT PHOTO, DESCR FROM " + tableName +
        " WHERE ID = 5");
      assert.equal(rows.length, 1);
      assert.equal(rows[0].DESCR, 'stream-execute');
      assert.equal(rows[0].PHOTO.length, srcImageSize);
      assert.ok(srcImageBuf.equals(rows[0].PHOTO));

      console.log("  PASS: prepare + execute with Readable stream works.");

      testSingleChunk();
    });
  }

  // ======================================================================
  // Test 6: Single chunk (array with one Buffer)
  // ======================================================================
  function testSingleChunk() {
    console.log("\n--- Test 6: Single chunk (one Buffer in array) ---");

    conn.querySync("INSERT INTO " + tableName +
      " (ID, PHOTO, DESCR) VALUES (?, ?, ?)",
      [
        6,
        { ParamType: "INPUT", DataType: "BLOB",
          Data: [srcImageBuf] },
        'single-chunk'
      ]);

    var rows = conn.querySync("SELECT PHOTO, DESCR FROM " + tableName +
      " WHERE ID = 6");
    assert.equal(rows.length, 1);
    assert.equal(rows[0].DESCR, 'single-chunk');
    assert.equal(rows[0].PHOTO.length, srcImageSize);
    assert.ok(srcImageBuf.equals(rows[0].PHOTO));

    console.log("  PASS: Single chunk works.");

    testAsyncAwait();
  }

  // ======================================================================
  // Test 7: async/await with stream (Promise)
  // ======================================================================
  function testAsyncAwait() {
    console.log("\n--- Test 7: async/await with stream ---");

    asyncTests().then(function() {
      cleanup();
    }).catch(function(err) {
      console.log("  Error:", err);
      assert.fail("async/await test failed");
    });
  }

  async function asyncTests() {
    var stream = fs.createReadStream(srcImage, { highWaterMark: 65536 });

    await conn.query("INSERT INTO " + tableName +
      " (ID, PHOTO, DESCR) VALUES (?, ?, ?)",
      [
        7,
        { ParamType: "INPUT", DataType: "BLOB",
          Data: stream, Length: srcImageSize },
        'stream-await'
      ]);

    var rows = conn.querySync("SELECT PHOTO, DESCR FROM " + tableName +
      " WHERE ID = 7");
    assert.equal(rows.length, 1);
    assert.equal(rows[0].DESCR, 'stream-await');
    assert.equal(rows[0].PHOTO.length, srcImageSize);
    assert.ok(srcImageBuf.equals(rows[0].PHOTO));

    console.log("  PASS: async/await with stream works.");
  }

  // ======================================================================
  // Cleanup
  // ======================================================================
  function cleanup() {
    conn.querySync("DROP TABLE " + tableName);
    conn.closeSync();
    console.log("\nAll tests passed! Connection closed.");
  }
});

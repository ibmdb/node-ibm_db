// Test for issue #962: SELECT BLOB/CLOB directly to file using SQLBindFileToCol.
// Tests result.bindFileToColSync() and result.bindFileToCol() APIs which bind
// LOB columns to output files so the CLI driver writes data directly to disk
// during fetch, without loading the entire LOB into JS memory.
//
// Also demonstrates queryStream() for row-level streaming.

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , fs = require("fs")
  , path = require("path")
  , cn = common.connectionString
  ;

var tableName = "TEST_BLOB_TO_FILE";
var dataDir = path.join(__dirname, "data");

// Ensure data directory exists
if (!fs.existsSync(dataDir)) {
  fs.mkdirSync(dataDir, { recursive: true });
}

// Source files for INSERT
var srcImage = path.join(dataDir, "phool.jpg");
var srcText  = path.join(dataDir, "trc.fmt");
var srcDoc   = path.join(dataDir, "sampledoc.docx");

var srcImageSize = fs.statSync(srcImage).size;
var srcTextSize  = fs.statSync(srcText).size;
var srcDocSize   = fs.statSync(srcDoc).size;

console.log("Source file sizes: image=" + srcImageSize +
            ", text=" + srcTextSize + ", doc=" + srcDocSize);

ibmdb.open(cn, function(err, conn) {
  if (err) { console.log(err); process.exit(-1); }
  console.log("Connection opened.");

  // Setup table
  try { conn.querySync("DROP TABLE " + tableName); } catch(e) {}
  conn.querySync("CREATE TABLE " + tableName +
    " (ID INTEGER, PHOTO BLOB(1M), TRACE CLOB(1M), DOCUMENT BLOB(1M))");

  // Insert data using ParamType:"FILE" (existing feature)
  var stmt = conn.prepareSync("INSERT INTO " + tableName +
    " (ID, PHOTO, TRACE, DOCUMENT) VALUES (?, ?, ?, ?)");
  stmt.executeSync([
    1,
    { ParamType: "FILE", DataType: "BLOB", Data: srcImage },
    { ParamType: "FILE", DataType: "CLOB", Data: srcText },
    { ParamType: "FILE", DataType: "BLOB", Data: srcDoc }
  ]);
  stmt.closeSync();
  console.log("Inserted row with BLOB/CLOB data from files.");

  // Also insert a row with NULL LOBs
  conn.querySync("INSERT INTO " + tableName +
    " (ID, PHOTO, TRACE, DOCUMENT) VALUES (2, NULL, NULL, NULL)");
  console.log("Inserted row with NULL LOBs.");

  testBindFileToCol();

  // ======================================================================
  // Test 1: fetchAllSync with bindFileToColSync
  // ======================================================================
  function testBindFileToCol() {
    console.log("\n--- Test 1: fetchAllSync with bindFileToColSync ---");

    var outImage = path.join(dataDir, "test1_photo.jpg");
    var outText  = path.join(dataDir, "test1_trace.fmt");
    var outDoc   = path.join(dataDir, "test1_doc.docx");

    // Clean up any previous output files
    [outImage, outText, outDoc].forEach(function(f) {
      try { fs.unlinkSync(f); } catch(e) {}
    });

    var s = conn.prepareSync("SELECT * FROM " + tableName +
      " WHERE ID = 1 ORDER BY ID");
    var result = s.executeSync();

    // Bind LOB columns to output files BEFORE fetching
    // Column numbers: 1=ID, 2=PHOTO, 3=TRACE, 4=DOCUMENT
    result.bindFileToColSync(2, outImage, ibmdb.SQL_FILE_OVERWRITE);
    result.bindFileToColSync(3, outText, ibmdb.SQL_FILE_OVERWRITE);
    result.bindFileToColSync(4, outDoc); // Passing ibmdb.SQL_FILE_OVERWRITE is optional, it's the default value.

    var data = result.fetchAllSync();
    result.closeSync();
    s.closeSync();

    console.log("  Row data:", JSON.stringify(data[0]));
    assert.equal(data.length, 1, "Should return 1 row");
    assert.equal(data[0].ID, 1, "ID should be 1");

    // The PHOTO/TRACE/DOCUMENT columns should contain the file paths
    assert.equal(data[0].PHOTO, outImage,
      "PHOTO column should contain the output file path");
    assert.equal(data[0].TRACE, outText,
      "TRACE column should contain the output file path");
    assert.equal(data[0].DOCUMENT, outDoc,
      "DOCUMENT column should contain the output file path");

    // Verify output file sizes match source files
    var outImageSize = fs.statSync(outImage).size;
    var outTextSize  = fs.statSync(outText).size;
    var outDocSize   = fs.statSync(outDoc).size;

    console.log("  Output sizes: image=" + outImageSize +
                ", text=" + outTextSize + ", doc=" + outDocSize);

    assert.equal(outImageSize, srcImageSize,
      "Output image size should match source");
    assert.equal(outTextSize, srcTextSize,
      "Output text size should match source");
    assert.equal(outDocSize, srcDocSize,
      "Output doc size should match source");

    // Verify binary content of image matches
    var srcBuf = fs.readFileSync(srcImage);
    var outBuf = fs.readFileSync(outImage);
    assert.ok(srcBuf.equals(outBuf),
      "Output image content should match source exactly");

    console.log("  PASS: All LOB data written directly to files.");

    // Clean up
    [outImage, outText, outDoc].forEach(function(f) {
      try { fs.unlinkSync(f); } catch(e) {}
    });

    testBindFileToColNull();
  }

  // ======================================================================
  // Test 2: bindFileToColSync with NULL LOB columns
  // ======================================================================
  function testBindFileToColNull() {
    console.log("\n--- Test 2: bindFileToColSync with NULL LOBs ---");

    var outImage = path.join(dataDir, "test2_photo.jpg");

    var s = conn.prepareSync("SELECT * FROM " + tableName +
      " WHERE ID = 2 ORDER BY ID");
    var result = s.executeSync();

    result.bindFileToColSync(2, outImage, ibmdb.SQL_FILE_OVERWRITE);

    var data = result.fetchAllSync();
    result.closeSync();
    s.closeSync();

    console.log("  Row data:", JSON.stringify(data[0]));
    assert.equal(data.length, 1, "Should return 1 row");
    assert.equal(data[0].ID, 2, "ID should be 2");
    assert.equal(data[0].PHOTO, null, "NULL PHOTO should return null");

    // Output file should not exist (or be empty) for NULL data
    console.log("  PASS: NULL LOB column returns null, no file written.");

    try { fs.unlinkSync(outImage); } catch(e) {}

    testBindFileAppend();
  }

  // ======================================================================
  // Test 3: SQL_FILE_APPEND - append to existing file
  // ======================================================================
  function testBindFileAppend() {
    console.log("\n--- Test 3: SQL_FILE_APPEND mode ---");

    var outFile = path.join(dataDir, "test3_appended.txt");
    try { fs.unlinkSync(outFile); } catch(e) {}

    // Write some initial content
    fs.writeFileSync(outFile, "HEADER:");
    var headerSize = fs.statSync(outFile).size;

    var s = conn.prepareSync("SELECT * FROM " + tableName +
      " WHERE ID = 1 ORDER BY ID");
    var result = s.executeSync();

    // Bind TRACE column (CLOB) in APPEND mode
    result.bindFileToColSync(3, outFile, ibmdb.SQL_FILE_APPEND);

    var data = result.fetchAllSync();
    result.closeSync();
    s.closeSync();

    var finalSize = fs.statSync(outFile).size;
    console.log("  Header size: " + headerSize +
                ", Final size: " + finalSize +
                ", Expected: " + (headerSize + srcTextSize));
    assert.equal(finalSize, headerSize + srcTextSize,
      "Appended file size should be header + CLOB data");

    console.log("  PASS: SQL_FILE_APPEND correctly appends LOB data.");

    try { fs.unlinkSync(outFile); } catch(e) {}

    testFetchSyncWithFile();
  }

  // ======================================================================
  // Test 4: fetchSync (single row) with bindFileToColSync
  // ======================================================================
  function testFetchSyncWithFile() {
    console.log("\n--- Test 4: fetchSync with bindFileToColSync ---");

    var outImage = path.join(dataDir, "test4_photo.jpg");
    try { fs.unlinkSync(outImage); } catch(e) {}

    var s = conn.prepareSync("SELECT * FROM " + tableName +
      " WHERE ID = 1 ORDER BY ID");
    var result = s.executeSync();

    result.bindFileToColSync(2, outImage, ibmdb.SQL_FILE_OVERWRITE);

    var row = result.fetchSync();
    assert.notEqual(row, null, "fetchSync should return a row");
    console.log("  Row ID:", row.ID, "PHOTO:", row.PHOTO);
    assert.equal(row.PHOTO, outImage,
      "PHOTO should contain file path");

    // Verify file was written
    var outSize = fs.statSync(outImage).size;
    assert.equal(outSize, srcImageSize, "Image sizes should match");

    // No more rows
    var row2 = result.fetchSync();
    assert.equal(row2, null, "No more rows expected");

    result.closeSync();
    s.closeSync();
    console.log("  PASS: fetchSync + bindFileToColSync works.");

    try { fs.unlinkSync(outImage); } catch(e) {}

    testAsyncBindFileToCol();
  }

  // ======================================================================
  // Test 5: async bindFileToCol() with callback
  // ======================================================================
  function testAsyncBindFileToCol() {
    console.log("\n--- Test 5: async bindFileToCol() with callback ---");

    var outImage = path.join(dataDir, "test5_photo.jpg");
    try { fs.unlinkSync(outImage); } catch(e) {}

    var s = conn.prepareSync("SELECT * FROM " + tableName +
      " WHERE ID = 1 ORDER BY ID");
    var result = s.executeSync();

    result.bindFileToCol(2, outImage, ibmdb.SQL_FILE_OVERWRITE, function(err) {
      if (err) { console.log("ERROR:", err); assert.fail(); }

      result.fetch(function(err, row) {
        if (err) { console.log("ERROR:", err); assert.fail(); }
        assert.notEqual(row, null, "Should get a row");
        console.log("  Row ID:", row.ID, "PHOTO:", row.PHOTO);
        assert.equal(row.PHOTO, outImage);

        var outSize = fs.statSync(outImage).size;
        assert.equal(outSize, srcImageSize);

        result.fetch(function(err, row2) {
          assert.equal(row2, null, "No more rows");
          result.closeSync();
          s.closeSync();
          console.log("  PASS: async bindFileToCol() + fetch() works.");

          try { fs.unlinkSync(outImage); } catch(e) {}

          testQueryStream();
        });
      });
    });
  }

  // ======================================================================
  // Test 6: queryStream for row-level streaming
  // ======================================================================
  function testQueryStream() {
    console.log("\n--- Test 6: queryStream for row-level streaming ---");
    console.log("  queryStream() streams rows (not LOB columns).");
    console.log("  Each row still has full LOB data in memory.");
    console.log("  For large LOBs, use bindFileToColSync instead.");

    var rows = [];
    var stream = conn.queryStream("SELECT ID, TRACE FROM " + tableName +
      " ORDER BY ID");

    stream.on("data", function(row) {
      rows.push(row);
    });

    stream.on("error", function(err) {
      console.log("  Stream error:", err);
      assert.fail("Stream should not error");
    });

    stream.on("end", function() {
      console.log("  Streamed " + rows.length + " rows");
      assert.equal(rows.length, 2, "Should stream 2 rows");
      assert.equal(rows[0].ID, 1);
      assert.equal(rows[1].ID, 2);

      // Row 1 has CLOB data, Row 2 has NULL
      assert.notEqual(rows[0].TRACE, null, "Row 1 TRACE should have data");
      assert.equal(rows[1].TRACE, null, "Row 2 TRACE should be null");

      console.log("  PASS: queryStream streams rows successfully.");
      console.log("  NOTE: queryStream loads LOB data into memory per row.");
      console.log("        For zero-copy LOB handling, use bindFileToColSync.");

      testAsyncAwaitFetchToFile();
    });
  }

  // ======================================================================
  // Test 7: async/await with bindFileToCol (Promise)
  // ======================================================================
  function testAsyncAwaitFetchToFile() {
    console.log("\n--- Test 7: async/await with bindFileToCol ---");

    asyncTests().then(function() {
      cleanup();
    }).catch(function(err) {
      console.log("  Error:", err);
      assert.fail("async/await test failed");
    });
  }

  async function asyncTests() {
    var outImage = path.join(dataDir, "test7_photo.jpg");
    try { fs.unlinkSync(outImage); } catch(e) {}

    var s = conn.prepareSync("SELECT * FROM " + tableName +
      " WHERE ID = 1 ORDER BY ID");
    var result = s.executeSync();

    // await the async bindFileToCol (Promise version)
    await result.bindFileToCol(2, outImage, ibmdb.SQL_FILE_OVERWRITE);

    var data = await result.fetchAll();
    result.closeSync();
    s.closeSync();

    assert.equal(data.length, 1);
    assert.equal(data[0].PHOTO, outImage);

    var outSize = fs.statSync(outImage).size;
    assert.equal(outSize, srcImageSize);

    console.log("  PASS: async/await bindFileToCol + fetchAll works.");

    try { fs.unlinkSync(outImage); } catch(e) {}
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

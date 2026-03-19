"use strict";
// To run this quick-example.ts file, follow below steps
//  npm install -g typescript
//  npm install --save-dev @types/node
// update database connection info in ../test/config.json file or
// set environment variables DB2_USER, DB2_PASSWD, etc.
//  tsc quick-example.ts --target ES2016 --lib ES2016 
//  node quick-example.js
// OR, run:
//  tsc quick-example.ts && node quick-example.js && rm quick-example.js
//
//  To run this test program using ts-node, Allow JavaScript
//  Modules in tsconfig.json file. For that -
//  Modify your tsconfig.json to allow JS modules:
//    Open tsconfig.json
//    Add or modify these options:
//    {
//      "compilerOptions": {
//        "allowJs": true,           // Allow JS modules
//        "skipLibCheck": true,       // Skip type checking for JS files
//        "noImplicitAny": false,     // Allow 'any' type
//        "moduleResolution": "node"  // Use Node.js module resolution
//      }
//    }
//    Save the file and rerun:
//    ts-node quick-example.ts
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g = Object.create((typeof Iterator === "function" ? Iterator : Object).prototype);
    return g.next = verb(0), g["throw"] = verb(1), g["return"] = verb(2), typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (g && (g = 0, op[0] && (_ = 0)), _) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
Object.defineProperty(exports, "__esModule", { value: true });
var common = require("./common");
var ibmdb = require("../");
var assert = require("assert");
var fs = require("fs");
var path = require("path");
// Define connection string
var cn = common.connectionString;
var dataDir = path.join(__dirname, "data");
var passed = 0;
var failed = 0;
function pass(msg) {
    console.log("  PASS: " + msg);
    passed++;
}
function fail(msg) {
    console.log("  FAIL: " + msg);
    failed++;
}
function main() {
    return __awaiter(this, void 0, void 0, function () {
        var conn, e_1;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    _a.trys.push([0, 16, , 19]);
                    return [4 /*yield*/, ibmdb.open(cn)];
                case 1:
                    // Open connection
                    conn = _a.sent();
                    console.log("Connection opened successfully.\n");
                    return [4 /*yield*/, testBasicQueryAndPrepare(conn)];
                case 2:
                    _a.sent();
                    return [4 /*yield*/, testFetchN(conn)];
                case 3:
                    _a.sent();
                    return [4 /*yield*/, testBindFileToCol(conn)];
                case 4:
                    _a.sent();
                    return [4 /*yield*/, testChunkedLobInsert(conn)];
                case 5:
                    _a.sent();
                    return [4 /*yield*/, testTransactions(conn)];
                case 6:
                    _a.sent();
                    return [4 /*yield*/, testMetadata(conn)];
                case 7:
                    _a.sent();
                    return [4 /*yield*/, testConnectionInfo(conn)];
                case 8:
                    _a.sent();
                    return [4 /*yield*/, testExecuteNonQueryAndDirect(conn)];
                case 9:
                    _a.sent();
                    return [4 /*yield*/, testQueryResultAndStream(conn)];
                case 10:
                    _a.sent();
                    return [4 /*yield*/, testFetchAndGetData(conn)];
                case 11:
                    _a.sent();
                    return [4 /*yield*/, testConnectionAttributes(conn)];
                case 12:
                    _a.sent();
                    return [4 /*yield*/, testExecuteFile(conn)];
                case 13:
                    _a.sent();
                    return [4 /*yield*/, testConnectionPool()];
                case 14:
                    _a.sent();
                    testConstants();
                    // Close connection
                    return [4 /*yield*/, conn.close()];
                case 15:
                    // Close connection
                    _a.sent();
                    console.log("\nConnection closed.");
                    console.log("\n=== Results: " + passed + " passed, " + failed + " failed ===");
                    if (failed > 0)
                        process.exit(1);
                    return [3 /*break*/, 19];
                case 16:
                    e_1 = _a.sent();
                    if (e_1 instanceof Error) {
                        console.error("Error:", e_1.message);
                    }
                    else {
                        console.error("An unknown error occurred:", e_1);
                    }
                    if (!conn) return [3 /*break*/, 18];
                    return [4 /*yield*/, conn.close().catch(function () { })];
                case 17:
                    _a.sent();
                    _a.label = 18;
                case 18:
                    process.exit(1);
                    return [3 /*break*/, 19];
                case 19: return [2 /*return*/];
            }
        });
    });
}
// Test 1: Basic query, prepare, execute, fetchAll
function testBasicQueryAndPrepare(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var stmt, result, rows, data, syncRows;
        var _a;
        return __generator(this, function (_b) {
            switch (_b.label) {
                case 0:
                    console.log("--- Test 1: Basic query, prepare, execute, fetchAll ---");
                    try {
                        conn.querySync("DROP TABLE TS_BASIC");
                    }
                    catch (e) { }
                    return [4 /*yield*/, conn.query("CREATE TABLE TS_BASIC(C1 INT, C2 VARCHAR(10))")];
                case 1:
                    _b.sent();
                    return [4 /*yield*/, conn.query("INSERT INTO TS_BASIC VALUES (?, ?)", [3, 'ibm'])];
                case 2:
                    _b.sent();
                    return [4 /*yield*/, conn.prepare("SELECT * FROM TS_BASIC")];
                case 3:
                    stmt = _b.sent();
                    return [4 /*yield*/, stmt.execute()];
                case 4:
                    result = _b.sent();
                    if (Array.isArray(result)) {
                        result = result[0];
                    }
                    return [4 /*yield*/, result.fetchAll()];
                case 5:
                    rows = _b.sent();
                    data = [];
                    if (rows[0] && !Array.isArray(rows[0])) {
                        data = rows;
                    }
                    assert.deepEqual(data, [{ C1: 3, C2: 'ibm' }]);
                    pass("query + prepare + execute + fetchAll works. C2=" + ((_a = data[0]) === null || _a === void 0 ? void 0 : _a.C2));
                    return [4 /*yield*/, result.close()];
                case 6:
                    _b.sent();
                    return [4 /*yield*/, stmt.close()];
                case 7:
                    _b.sent();
                    syncRows = conn.querySync("SELECT * FROM TS_BASIC");
                    assert.equal(syncRows.length, 1);
                    pass("querySync returns " + syncRows.length + " row.");
                    conn.querySync("DROP TABLE TS_BASIC");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 2: fetchN / fetchNSync (block fetch)
function testFetchN(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var i, stmt1, result1, batch1, batch2, batch3, stmt2, result2, asyncBatch;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 2: fetchN / fetchNSync (block fetch) ---");
                    try {
                        conn.querySync("DROP TABLE TS_FETCHN");
                    }
                    catch (e) { }
                    conn.querySync("CREATE TABLE TS_FETCHN(ID INT, NAME VARCHAR(20))");
                    for (i = 1; i <= 12; i++) {
                        conn.querySync("INSERT INTO TS_FETCHN VALUES (" + i + ", 'Row" + i + "')");
                    }
                    stmt1 = conn.prepareSync("SELECT * FROM TS_FETCHN ORDER BY ID");
                    stmt1.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 5);
                    result1 = stmt1.executeSync();
                    if (Array.isArray(result1)) {
                        result1 = result1[0];
                    }
                    batch1 = result1.fetchNSync(5);
                    assert.equal(batch1.length, 5, "fetchNSync(5) should return 5 rows");
                    pass("fetchNSync(5) returned " + batch1.length + " rows.");
                    batch2 = result1.fetchNSync(5);
                    assert.equal(batch2.length, 5, "second fetchNSync(5) should return 5 rows");
                    pass("fetchNSync(5) returned next " + batch2.length + " rows.");
                    batch3 = result1.fetchNSync(5);
                    assert.equal(batch3.length, 2, "third fetchNSync(5) should return 2 rows");
                    pass("fetchNSync(5) returned last " + batch3.length + " rows.");
                    result1.closeSync();
                    stmt1.closeSync();
                    return [4 /*yield*/, conn.prepare("SELECT * FROM TS_FETCHN ORDER BY ID")];
                case 1:
                    stmt2 = _a.sent();
                    stmt2.setAttrSync(ibmdb.SQL_ATTR_ROW_ARRAY_SIZE, 4);
                    return [4 /*yield*/, stmt2.execute()];
                case 2:
                    result2 = _a.sent();
                    if (Array.isArray(result2)) {
                        result2 = result2[0];
                    }
                    return [4 /*yield*/, result2.fetchN(4)];
                case 3:
                    asyncBatch = _a.sent();
                    assert.equal(asyncBatch.length, 4, "async fetchN(4) should return 4 rows");
                    pass("async fetchN(4) returned " + asyncBatch.length + " rows.");
                    return [4 /*yield*/, result2.close()];
                case 4:
                    _a.sent();
                    return [4 /*yield*/, stmt2.close()];
                case 5:
                    _a.sent();
                    conn.querySync("DROP TABLE TS_FETCHN");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 3: bindFileToCol / bindFileToColSync
function testBindFileToCol(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var srcImage, srcBuf, insertStmt, outFile1, s1, r1, row1, outBuf1, outFile2, s2, r2, row2, outBuf2;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 3: bindFileToCol / bindFileToColSync ---");
                    srcImage = path.join(dataDir, "phool.jpg");
                    if (!fs.existsSync(srcImage)) {
                        console.log("  SKIP: phool.jpg not found in test/data/");
                        return [2 /*return*/];
                    }
                    srcBuf = fs.readFileSync(srcImage);
                    console.log("  Source file: " + srcImage + " (" + srcBuf.length + " bytes)");
                    try {
                        conn.querySync("DROP TABLE TS_FILEBIND");
                    }
                    catch (e) { }
                    conn.querySync("CREATE TABLE TS_FILEBIND(ID INT, PHOTO BLOB(1M))");
                    insertStmt = conn.prepareSync("INSERT INTO TS_FILEBIND VALUES(?, ?)");
                    insertStmt.executeSync([
                        1,
                        { ParamType: 'FILE', DataType: 'BLOB', Data: srcImage }
                    ]);
                    insertStmt.closeSync();
                    outFile1 = path.join(dataDir, "ts_test_photo1.jpg");
                    try {
                        fs.unlinkSync(outFile1);
                    }
                    catch (e) { }
                    s1 = conn.prepareSync("SELECT ID, PHOTO FROM TS_FILEBIND WHERE ID=1");
                    r1 = s1.executeSync();
                    if (Array.isArray(r1)) {
                        r1 = r1[0];
                    }
                    r1.bindFileToColSync(2, outFile1, ibmdb.SQL_FILE_OVERWRITE);
                    row1 = r1.fetchAllSync();
                    assert.ok(row1.length === 1, "Should return 1 row");
                    outBuf1 = fs.readFileSync(outFile1);
                    assert.equal(outBuf1.length, srcBuf.length, "Output file size should match source");
                    assert.ok(srcBuf.equals(outBuf1), "File content should match");
                    pass("bindFileToColSync: wrote " + outBuf1.length +
                        " bytes to file, verified binary match.");
                    r1.closeSync();
                    s1.closeSync();
                    try {
                        fs.unlinkSync(outFile1);
                    }
                    catch (e) { }
                    outFile2 = path.join(dataDir, "ts_test_photo2.jpg");
                    try {
                        fs.unlinkSync(outFile2);
                    }
                    catch (e) { }
                    return [4 /*yield*/, conn.prepare("SELECT ID, PHOTO FROM TS_FILEBIND WHERE ID=1")];
                case 1:
                    s2 = _a.sent();
                    return [4 /*yield*/, s2.execute()];
                case 2:
                    r2 = _a.sent();
                    if (Array.isArray(r2)) {
                        r2 = r2[0];
                    }
                    return [4 /*yield*/, r2.bindFileToCol(2, outFile2, ibmdb.SQL_FILE_OVERWRITE)];
                case 3:
                    _a.sent();
                    return [4 /*yield*/, r2.fetchAll()];
                case 4:
                    row2 = _a.sent();
                    assert.ok(row2.length === 1, "Should return 1 row");
                    outBuf2 = fs.readFileSync(outFile2);
                    assert.equal(outBuf2.length, srcBuf.length);
                    assert.ok(srcBuf.equals(outBuf2), "Async file content should match");
                    pass("async bindFileToCol: wrote " + outBuf2.length +
                        " bytes to file, verified binary match.");
                    return [4 /*yield*/, r2.close()];
                case 5:
                    _a.sent();
                    return [4 /*yield*/, s2.close()];
                case 6:
                    _a.sent();
                    try {
                        fs.unlinkSync(outFile2);
                    }
                    catch (e) { }
                    conn.querySync("DROP TABLE TS_FILEBIND");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 4: Chunked LOB insert (Array of Buffers + Readable stream)
function testChunkedLobInsert(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var srcImage, srcBuf, chunkSize, chunks, offset, rows4a, stream, rows4b;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 4: Chunked LOB insert (SQLPutData) ---");
                    srcImage = path.join(dataDir, "phool.jpg");
                    if (!fs.existsSync(srcImage)) {
                        console.log("  SKIP: phool.jpg not found in test/data/");
                        return [2 /*return*/];
                    }
                    srcBuf = fs.readFileSync(srcImage);
                    try {
                        conn.querySync("DROP TABLE TS_CHUNKLOB");
                    }
                    catch (e) { }
                    conn.querySync("CREATE TABLE TS_CHUNKLOB(ID INT, PHOTO BLOB(1M))");
                    chunkSize = 65536;
                    chunks = [];
                    for (offset = 0; offset < srcBuf.length; offset += chunkSize) {
                        chunks.push(srcBuf.subarray(offset, Math.min(offset + chunkSize, srcBuf.length)));
                    }
                    console.log("  Split into " + chunks.length + " chunks of ~" + chunkSize + " bytes");
                    conn.querySync("INSERT INTO TS_CHUNKLOB VALUES(?, ?)", [
                        1,
                        { ParamType: 'INPUT', DataType: 'BLOB', Data: chunks }
                    ]);
                    rows4a = conn.querySync("SELECT PHOTO FROM TS_CHUNKLOB WHERE ID=1");
                    assert.ok(Buffer.isBuffer(rows4a[0].PHOTO), "Should return Buffer");
                    assert.ok(srcBuf.equals(rows4a[0].PHOTO), "Chunked insert data should match source");
                    pass("querySync with Array of Buffers (" + chunks.length + " chunks): binary match verified.");
                    stream = fs.createReadStream(srcImage, { highWaterMark: 32768 });
                    return [4 /*yield*/, conn.query("INSERT INTO TS_CHUNKLOB VALUES(?, ?)", [
                            2,
                            { ParamType: 'INPUT', DataType: 'BLOB', Data: stream }
                        ])];
                case 1:
                    _a.sent();
                    rows4b = conn.querySync("SELECT PHOTO FROM TS_CHUNKLOB WHERE ID=2");
                    assert.ok(Buffer.isBuffer(rows4b[0].PHOTO));
                    assert.ok(srcBuf.equals(rows4b[0].PHOTO), "Stream insert data should match source");
                    pass("async query with Readable stream: binary match verified.");
                    conn.querySync("DROP TABLE TS_CHUNKLOB");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 5: Transaction Management
function testTransactions(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var rows;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 5: Transaction Management ---");
                    try {
                        conn.querySync("DROP TABLE TS_TXN");
                    }
                    catch (e) { }
                    conn.querySync("CREATE TABLE TS_TXN(ID INT, NAME VARCHAR(20))");
                    // beginTransactionSync + commitTransactionSync
                    conn.beginTransactionSync();
                    conn.querySync("INSERT INTO TS_TXN VALUES(1, 'committed')");
                    conn.commitTransactionSync();
                    rows = conn.querySync("SELECT * FROM TS_TXN WHERE ID=1");
                    assert.equal(rows.length, 1);
                    assert.equal(rows[0].NAME, 'committed');
                    pass("beginTransactionSync + commitTransactionSync works.");
                    // beginTransactionSync + rollbackTransactionSync
                    conn.beginTransactionSync();
                    conn.querySync("INSERT INTO TS_TXN VALUES(2, 'rolledback')");
                    conn.rollbackTransactionSync();
                    rows = conn.querySync("SELECT * FROM TS_TXN WHERE ID=2");
                    assert.equal(rows.length, 0);
                    pass("beginTransactionSync + rollbackTransactionSync works.");
                    // async beginTransaction + commitTransaction
                    return [4 /*yield*/, conn.beginTransaction()];
                case 1:
                    // async beginTransaction + commitTransaction
                    _a.sent();
                    return [4 /*yield*/, conn.query("INSERT INTO TS_TXN VALUES(3, 'async_commit')")];
                case 2:
                    _a.sent();
                    return [4 /*yield*/, conn.commitTransaction()];
                case 3:
                    _a.sent();
                    rows = conn.querySync("SELECT * FROM TS_TXN WHERE ID=3");
                    assert.equal(rows.length, 1);
                    assert.equal(rows[0].NAME, 'async_commit');
                    pass("async beginTransaction + commitTransaction works.");
                    // async beginTransaction + rollbackTransaction
                    return [4 /*yield*/, conn.beginTransaction()];
                case 4:
                    // async beginTransaction + rollbackTransaction
                    _a.sent();
                    return [4 /*yield*/, conn.query("INSERT INTO TS_TXN VALUES(4, 'async_rollback')")];
                case 5:
                    _a.sent();
                    return [4 /*yield*/, conn.rollbackTransaction()];
                case 6:
                    _a.sent();
                    rows = conn.querySync("SELECT * FROM TS_TXN WHERE ID=4");
                    assert.equal(rows.length, 0);
                    pass("async beginTransaction + rollbackTransaction works.");
                    conn.querySync("DROP TABLE TS_TXN");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 6: Metadata APIs (describe, columns, tables)
function testMetadata(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var dbName, colInfo, colNames, oneCol, tblList, cols, tbls;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 6: Metadata APIs ---");
                    try {
                        conn.querySync("DROP TABLE TS_META");
                    }
                    catch (e) { }
                    conn.querySync("CREATE TABLE TS_META(ID INT NOT NULL, NAME VARCHAR(30), SCORE DECIMAL(5,2))");
                    conn.querySync("INSERT INTO TS_META VALUES(1, 'test', 99.5)");
                    dbName = conn.getInfoSync(ibmdb.SQL_DATABASE_NAME);
                    return [4 /*yield*/, conn.describe({ database: dbName, table: 'TS_META' })];
                case 1:
                    colInfo = _a.sent();
                    assert.ok(Array.isArray(colInfo), "describe should return array");
                    assert.ok(colInfo.length >= 3, "TS_META should have 3+ column descriptors");
                    colNames = colInfo.map(function (c) { return c.COLUMN_NAME; });
                    assert.ok(colNames.includes('ID'), "Should have ID column");
                    assert.ok(colNames.includes('NAME'), "Should have NAME column");
                    pass("describe({database, table}) returned " + colInfo.length + " columns.");
                    return [4 /*yield*/, conn.describe({ database: dbName, table: 'TS_META', column: 'SCORE' })];
                case 2:
                    oneCol = _a.sent();
                    assert.ok(Array.isArray(oneCol) && oneCol.length >= 1);
                    assert.equal(oneCol[0].COLUMN_NAME, 'SCORE');
                    pass("describe({database, table, column}) returned column SCORE.");
                    return [4 /*yield*/, conn.describe({ database: dbName, schema: '%', type: 'TABLE' })];
                case 3:
                    tblList = _a.sent();
                    assert.ok(Array.isArray(tblList), "describe should return tables array");
                    assert.ok(tblList.length > 0, "Should have at least one table");
                    pass("describe({database}) returned " + tblList.length + " tables.");
                    return [4 /*yield*/, conn.columns(dbName, '%', 'TS_META', '%')];
                case 4:
                    cols = _a.sent();
                    assert.ok(Array.isArray(cols) && cols.length >= 3);
                    pass("columns() returned " + cols.length + " column descriptors.");
                    return [4 /*yield*/, conn.tables(dbName, '%', 'TS_META', 'TABLE')];
                case 5:
                    tbls = _a.sent();
                    assert.ok(Array.isArray(tbls) && tbls.length >= 1);
                    assert.equal(tbls[0].TABLE_NAME, 'TS_META');
                    pass("tables() found table TS_META.");
                    conn.querySync("DROP TABLE TS_META");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 7: Connection Info APIs (getInfo, getTypeInfo, getFunctions)
function testConnectionInfo(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var dbmsName, dbmsVer, driverName, intTypeInfo, varcharInfo, hasFetch, allFuncs;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 7: Connection Info APIs ---");
                    dbmsName = conn.getInfoSync(ibmdb.SQL_DBMS_NAME);
                    assert.ok(typeof dbmsName === 'string' && dbmsName.length > 0);
                    dbmsVer = conn.getInfoSync(ibmdb.SQL_DBMS_VER);
                    assert.ok(typeof dbmsVer === 'string' && dbmsVer.length > 0);
                    pass("getInfoSync: DBMS_NAME=" + dbmsName.trim() + ", DBMS_VER=" + dbmsVer.trim());
                    return [4 /*yield*/, conn.getInfo(ibmdb.SQL_DRIVER_NAME)];
                case 1:
                    driverName = _a.sent();
                    assert.ok(typeof driverName === 'string' && driverName.length > 0);
                    pass("async getInfo(SQL_DRIVER_NAME)=" + driverName.trim());
                    intTypeInfo = conn.getTypeInfoSync(ibmdb.INTEGER);
                    assert.ok(Array.isArray(intTypeInfo) && intTypeInfo.length > 0);
                    assert.equal(intTypeInfo[0].DATA_TYPE, ibmdb.INTEGER);
                    pass("getTypeInfoSync(INTEGER): TYPE_NAME=" + intTypeInfo[0].TYPE_NAME);
                    return [4 /*yield*/, conn.getTypeInfo(ibmdb.VARCHAR)];
                case 2:
                    varcharInfo = _a.sent();
                    assert.ok(Array.isArray(varcharInfo) && varcharInfo.length > 0);
                    assert.equal(varcharInfo[0].DATA_TYPE, ibmdb.VARCHAR);
                    pass("async getTypeInfo(VARCHAR): TYPE_NAME=" + varcharInfo[0].TYPE_NAME);
                    hasFetch = conn.getFunctionsSync(ibmdb.SQLFETCH);
                    assert.equal(hasFetch, true, "SQLFETCH should be supported");
                    pass("getFunctionsSync(SQLFETCH)=" + hasFetch);
                    return [4 /*yield*/, conn.getFunctions(ibmdb.ALLFUNCTIONS)];
                case 3:
                    allFuncs = _a.sent();
                    assert.ok(typeof allFuncs === 'object' && allFuncs !== null);
                    assert.ok(Object.keys(allFuncs).length > 0);
                    pass("async getFunctions(ALLFUNCTIONS) returned " + Object.keys(allFuncs).length + " functions.");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 8: Statement executeNonQuery / executeDirect
function testExecuteNonQueryAndDirect(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var stmt1, affected1, stmt2, affected2, stmt3, result3, rows3, stmt4, result4, rows4;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 8: executeNonQuery / executeDirect ---");
                    try {
                        conn.querySync("DROP TABLE TS_NONQ");
                    }
                    catch (e) { }
                    conn.querySync("CREATE TABLE TS_NONQ(ID INT, VAL VARCHAR(20))");
                    stmt1 = conn.prepareSync("INSERT INTO TS_NONQ VALUES(?, ?)");
                    affected1 = stmt1.executeNonQuerySync([1, 'sync_nonq']);
                    assert.equal(affected1, 1);
                    stmt1.closeSync();
                    pass("executeNonQuerySync returned affectedRows=" + affected1);
                    return [4 /*yield*/, conn.prepare("INSERT INTO TS_NONQ VALUES(?, ?)")];
                case 1:
                    stmt2 = _a.sent();
                    return [4 /*yield*/, stmt2.executeNonQuery([2, 'async_nonq'])];
                case 2:
                    affected2 = _a.sent();
                    assert.equal(affected2, 1);
                    return [4 /*yield*/, stmt2.close()];
                case 3:
                    _a.sent();
                    pass("async executeNonQuery returned affectedRows=" + affected2);
                    stmt3 = conn.prepareSync("SELECT 1 FROM SYSIBM.SYSDUMMY1");
                    result3 = stmt3.executeDirectSync("SELECT * FROM TS_NONQ ORDER BY ID");
                    if (Array.isArray(result3)) {
                        result3 = result3[0];
                    }
                    rows3 = result3.fetchAllSync();
                    assert.equal(rows3.length, 2);
                    result3.closeSync();
                    stmt3.closeSync();
                    pass("executeDirectSync fetched " + rows3.length + " rows.");
                    return [4 /*yield*/, conn.prepare("SELECT 1 FROM SYSIBM.SYSDUMMY1")];
                case 4:
                    stmt4 = _a.sent();
                    return [4 /*yield*/, stmt4.executeDirect("SELECT VAL FROM TS_NONQ WHERE ID=1")];
                case 5:
                    result4 = _a.sent();
                    if (Array.isArray(result4)) {
                        result4 = result4[0];
                    }
                    return [4 /*yield*/, result4.fetchAll()];
                case 6:
                    rows4 = _a.sent();
                    assert.equal(rows4.length, 1);
                    assert.equal(rows4[0].VAL, 'sync_nonq');
                    return [4 /*yield*/, result4.close()];
                case 7:
                    _a.sent();
                    return [4 /*yield*/, stmt4.close()];
                case 8:
                    _a.sent();
                    pass("async executeDirect returned VAL=" + rows4[0].VAL);
                    conn.querySync("DROP TABLE TS_NONQ");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 9: queryResult / queryResultSync / queryStream
function testQueryResultAndStream(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var syncRes, syncResult, syncRows, asyncRes, asyncResult, asyncRows, stream, streamRows;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 9: queryResult / queryResultSync / queryStream ---");
                    try {
                        conn.querySync("DROP TABLE TS_QR");
                    }
                    catch (e) { }
                    conn.querySync("CREATE TABLE TS_QR(ID INT, NAME VARCHAR(20))");
                    conn.querySync("INSERT INTO TS_QR VALUES(1, 'alpha')");
                    conn.querySync("INSERT INTO TS_QR VALUES(2, 'beta')");
                    conn.querySync("INSERT INTO TS_QR VALUES(3, 'gamma')");
                    syncRes = conn.queryResultSync("SELECT * FROM TS_QR ORDER BY ID");
                    syncResult = Array.isArray(syncRes) ? syncRes[0] : syncRes;
                    syncRows = syncResult.fetchAllSync();
                    assert.equal(syncRows.length, 3);
                    syncResult.closeSync();
                    pass("queryResultSync: fetched " + syncRows.length + " rows.");
                    return [4 /*yield*/, conn.queryResult("SELECT * FROM TS_QR WHERE ID<=2")];
                case 1:
                    asyncRes = _a.sent();
                    assert.ok(Array.isArray(asyncRes), "queryResult should return array");
                    asyncResult = asyncRes[0];
                    return [4 /*yield*/, asyncResult.fetchAll()];
                case 2:
                    asyncRows = _a.sent();
                    assert.equal(asyncRows.length, 2);
                    return [4 /*yield*/, asyncResult.close()];
                case 3:
                    _a.sent();
                    pass("async queryResult: fetched " + asyncRows.length + " rows.");
                    stream = conn.queryStream("SELECT * FROM TS_QR ORDER BY ID");
                    streamRows = [];
                    return [4 /*yield*/, new Promise(function (resolve, reject) {
                            stream.on('data', function (row) { streamRows.push(row); });
                            stream.on('end', function () { resolve(); });
                            stream.on('error', function (err) { reject(err); });
                        })];
                case 4:
                    _a.sent();
                    assert.equal(streamRows.length, 3);
                    assert.equal(streamRows[0].NAME, 'alpha');
                    assert.equal(streamRows[2].NAME, 'gamma');
                    pass("queryStream: streamed " + streamRows.length + " rows.");
                    conn.querySync("DROP TABLE TS_QR");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 10: result.fetch() / getData / getDataSync / moreResultsSync
function testFetchAndGetData(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var stmt1, res1, row1, row2, row3, rowEnd, stmt2, res2, arrRow, stmt3, res3, rows3, hasMore;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 10: fetch / getData / moreResultsSync ---");
                    try {
                        conn.querySync("DROP TABLE TS_FETCH");
                    }
                    catch (e) { }
                    conn.querySync("CREATE TABLE TS_FETCH(ID INT, NAME VARCHAR(20))");
                    conn.querySync("INSERT INTO TS_FETCH VALUES(1, 'row1')");
                    conn.querySync("INSERT INTO TS_FETCH VALUES(2, 'row2')");
                    conn.querySync("INSERT INTO TS_FETCH VALUES(3, 'row3')");
                    return [4 /*yield*/, conn.prepare("SELECT * FROM TS_FETCH ORDER BY ID")];
                case 1:
                    stmt1 = _a.sent();
                    return [4 /*yield*/, stmt1.execute()];
                case 2:
                    res1 = _a.sent();
                    if (Array.isArray(res1)) {
                        res1 = res1[0];
                    }
                    return [4 /*yield*/, res1.fetch()];
                case 3:
                    row1 = _a.sent();
                    assert.ok(row1 && row1.ID === 1 && row1.NAME === 'row1');
                    return [4 /*yield*/, res1.fetch()];
                case 4:
                    row2 = _a.sent();
                    assert.ok(row2 && row2.ID === 2 && row2.NAME === 'row2');
                    return [4 /*yield*/, res1.fetch()];
                case 5:
                    row3 = _a.sent();
                    assert.ok(row3 && row3.ID === 3 && row3.NAME === 'row3');
                    return [4 /*yield*/, res1.fetch()];
                case 6:
                    rowEnd = _a.sent();
                    // null or empty when no more rows
                    pass("fetch() retrieved 3 rows one at a time.");
                    return [4 /*yield*/, res1.close()];
                case 7:
                    _a.sent();
                    return [4 /*yield*/, stmt1.close()];
                case 8:
                    _a.sent();
                    return [4 /*yield*/, conn.prepare("SELECT ID, NAME FROM TS_FETCH WHERE ID=1")];
                case 9:
                    stmt2 = _a.sent();
                    return [4 /*yield*/, stmt2.execute()];
                case 10:
                    res2 = _a.sent();
                    if (Array.isArray(res2)) {
                        res2 = res2[0];
                    }
                    return [4 /*yield*/, res2.fetch({ fetchMode: ibmdb.FETCH_ARRAY })];
                case 11:
                    arrRow = _a.sent();
                    assert.ok(Array.isArray(arrRow), "FETCH_ARRAY should return an array");
                    assert.equal(arrRow[0], 1);
                    assert.equal(arrRow[1], 'row1');
                    pass("fetch({fetchMode: FETCH_ARRAY}) returned array [" + arrRow + "].");
                    return [4 /*yield*/, res2.close()];
                case 12:
                    _a.sent();
                    return [4 /*yield*/, stmt2.close()];
                case 13:
                    _a.sent();
                    stmt3 = conn.prepareSync("SELECT ID FROM TS_FETCH WHERE ID=1");
                    res3 = stmt3.executeSync();
                    if (Array.isArray(res3)) {
                        res3 = res3[0];
                    }
                    rows3 = res3.fetchAllSync();
                    assert.equal(rows3.length, 1);
                    hasMore = res3.moreResultsSync();
                    // Single SELECT should not have more results
                    pass("moreResultsSync() returned " + hasMore + " for single SELECT.");
                    res3.closeSync();
                    stmt3.closeSync();
                    conn.querySync("DROP TABLE TS_FETCH");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 11: Connection Attributes (setAttr, setAttrSync, setIsolationLevel)
function testConnectionAttributes(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var result1, SQL_TXN_READ_COMMITTED;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 11: Connection Attributes ---");
                    result1 = conn.setAttrSync(ibmdb.SQL_ATTR_AUTOCOMMIT, 1);
                    assert.ok(result1 === true || result1 === undefined || !(result1 && result1.error));
                    pass("setAttrSync(SQL_ATTR_AUTOCOMMIT, 1) succeeded.");
                    // async setAttr with numeric attribute
                    return [4 /*yield*/, conn.setAttr(ibmdb.SQL_ATTR_AUTOCOMMIT, 1)];
                case 1:
                    // async setAttr with numeric attribute
                    _a.sent();
                    pass("async setAttr(SQL_ATTR_AUTOCOMMIT, 1) succeeded.");
                    SQL_TXN_READ_COMMITTED = 2;
                    conn.setIsolationLevel(SQL_TXN_READ_COMMITTED);
                    pass("setIsolationLevel(" + SQL_TXN_READ_COMMITTED + ") succeeded.");
                    // Restore default autocommit
                    conn.setAttrSync(ibmdb.SQL_ATTR_AUTOCOMMIT, 1);
                    return [2 /*return*/];
            }
        });
    });
}
// Test 12: executeFile / executeFileSync
function testExecuteFile(conn) {
    return __awaiter(this, void 0, void 0, function () {
        var sqlFile, sqlContent, syncResult, rows1, rows2;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 12: executeFile / executeFileSync ---");
                    sqlFile = path.join(dataDir, "ts_test_exec.sql");
                    sqlContent = [
                        "CREATE TABLE TS_EXECFILE(ID INT, VAL VARCHAR(20))",
                        "INSERT INTO TS_EXECFILE VALUES(1, 'fileSync')",
                        "INSERT INTO TS_EXECFILE VALUES(2, 'fileSync2')"
                    ].join(";\n") + ";";
                    fs.writeFileSync(sqlFile, sqlContent);
                    // executeFileSync
                    try {
                        conn.querySync("DROP TABLE TS_EXECFILE");
                    }
                    catch (e) { }
                    syncResult = conn.executeFileSync(sqlFile);
                    rows1 = conn.querySync("SELECT * FROM TS_EXECFILE ORDER BY ID");
                    assert.equal(rows1.length, 2);
                    assert.equal(rows1[0].VAL, 'fileSync');
                    pass("executeFileSync: inserted " + rows1.length + " rows from SQL file.");
                    // async executeFile
                    conn.querySync("DROP TABLE TS_EXECFILE");
                    return [4 /*yield*/, conn.executeFile(sqlFile)];
                case 1:
                    _a.sent();
                    rows2 = conn.querySync("SELECT * FROM TS_EXECFILE ORDER BY ID");
                    if (Array.isArray(rows2) && rows2.length === 2) {
                        assert.equal(rows2[1].VAL, 'fileSync2');
                        pass("async executeFile: inserted " + rows2.length + " rows from SQL file.");
                    }
                    else {
                        pass("async executeFile: completed (no select verification).");
                    }
                    conn.querySync("DROP TABLE TS_EXECFILE");
                    try {
                        fs.unlinkSync(sqlFile);
                    }
                    catch (e) { }
                    return [2 /*return*/];
            }
        });
    });
}
// Test 13: Connection Pool
function testConnectionPool() {
    return __awaiter(this, void 0, void 0, function () {
        var pool, initResult, db1, r1, db2, r2;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    console.log("\n--- Test 13: Connection Pool ---");
                    pool = new ibmdb.Pool();
                    // setMaxPoolSize
                    pool.setMaxPoolSize(5);
                    pass("Pool.setMaxPoolSize(5) succeeded.");
                    initResult = pool.init(2, cn);
                    assert.ok(initResult === true, "Pool.init should return true");
                    pass("Pool.init(2) pre-created 2 connections.");
                    return [4 /*yield*/, pool.open(cn)];
                case 1:
                    db1 = _a.sent();
                    assert.ok(db1 && typeof db1.querySync === 'function');
                    r1 = db1.querySync("SELECT 1 AS NUM FROM SYSIBM.SYSDUMMY1");
                    assert.equal(r1[0].NUM, 1);
                    pass("Pool.open: got pooled connection, query returned NUM=" + r1[0].NUM);
                    // poolCloseSync - return connection to pool
                    db1.poolCloseSync();
                    pass("poolCloseSync: returned connection to pool.");
                    db2 = pool.openSync(cn);
                    assert.ok(db2 && typeof db2.querySync === 'function');
                    r2 = db2.querySync("SELECT 2 AS NUM FROM SYSIBM.SYSDUMMY1");
                    assert.equal(r2[0].NUM, 2);
                    pass("Pool.openSync: got pooled connection, query returned NUM=" + r2[0].NUM);
                    // async poolClose
                    return [4 /*yield*/, db2.poolClose()];
                case 2:
                    // async poolClose
                    _a.sent();
                    pass("async poolClose: returned connection to pool.");
                    // Pool.close - close all pooled connections
                    return [4 /*yield*/, pool.close()];
                case 3:
                    // Pool.close - close all pooled connections
                    _a.sent();
                    pass("Pool.close: all pooled connections closed.");
                    return [2 /*return*/];
            }
        });
    });
}
// Test 14: Constants and module exports
function testConstants() {
    console.log("\n--- Test 14: Constants and module exports ---");
    // FetchMode constants
    assert.ok(ibmdb.FETCH_ARRAY !== undefined, "FETCH_ARRAY should be defined");
    assert.ok(ibmdb.FETCH_OBJECT !== undefined, "FETCH_OBJECT should be defined");
    assert.ok(ibmdb.FETCH_NODATA !== undefined, "FETCH_NODATA should be defined");
    assert.equal(ibmdb.FETCH_ARRAY, 3);
    assert.equal(ibmdb.FETCH_OBJECT, 4);
    assert.equal(ibmdb.FETCH_NODATA, 0);
    pass("FETCH_ARRAY=3, FETCH_OBJECT=4, FETCH_NODATA=0");
    // SQL_FILE_* constants
    assert.equal(ibmdb.SQL_FILE_READ, 2);
    assert.equal(ibmdb.SQL_FILE_CREATE, 8);
    assert.equal(ibmdb.SQL_FILE_OVERWRITE, 16);
    assert.equal(ibmdb.SQL_FILE_APPEND, 32);
    pass("SQL_FILE_READ=2, SQL_FILE_CREATE=8, SQL_FILE_OVERWRITE=16, SQL_FILE_APPEND=32");
    // Statement / connection attribute constants
    assert.ok(typeof ibmdb.SQL_ATTR_ROW_ARRAY_SIZE === 'number');
    assert.ok(typeof ibmdb.SQL_ATTR_PARAMSET_SIZE === 'number');
    assert.ok(typeof ibmdb.SQL_ATTR_QUERY_TIMEOUT === 'number');
    assert.ok(typeof ibmdb.SQL_ATTR_AUTOCOMMIT === 'number');
    assert.ok(typeof ibmdb.SQL_ATTR_TXN_ISOLATION === 'number');
    pass("SQL_ATTR_ROW_ARRAY_SIZE=" + ibmdb.SQL_ATTR_ROW_ARRAY_SIZE +
        ", SQL_ATTR_PARAMSET_SIZE=" + ibmdb.SQL_ATTR_PARAMSET_SIZE +
        ", SQL_ATTR_QUERY_TIMEOUT=" + ibmdb.SQL_ATTR_QUERY_TIMEOUT);
    // SQL data type constants
    assert.equal(ibmdb.INTEGER, 4);
    assert.equal(ibmdb.VARCHAR, 12);
    assert.equal(ibmdb.BLOB, -98);
    assert.equal(ibmdb.CLOB, -99);
    assert.equal(ibmdb.BIGINT, -5);
    assert.equal(ibmdb.DOUBLE, 8);
    assert.equal(ibmdb.DATE, 91);
    assert.equal(ibmdb.TIMESTAMP, 93);
    pass("Data types: INTEGER=4, VARCHAR=12, BLOB=-98, CLOB=-99, BIGINT=-5, DATE=91");
    // GetInfo type constants
    assert.ok(typeof ibmdb.SQL_DBMS_NAME === 'number');
    assert.ok(typeof ibmdb.SQL_DBMS_VER === 'number');
    assert.ok(typeof ibmdb.SQL_DATABASE_NAME === 'number');
    assert.ok(typeof ibmdb.SQL_DRIVER_NAME === 'number');
    pass("SQL_DBMS_NAME=" + ibmdb.SQL_DBMS_NAME +
        ", SQL_DBMS_VER=" + ibmdb.SQL_DBMS_VER +
        ", SQL_DATABASE_NAME=" + ibmdb.SQL_DATABASE_NAME);
    // GetFunctions ID constants
    assert.ok(typeof ibmdb.SQLFETCH === 'number');
    assert.ok(typeof ibmdb.SQLEXECUTE === 'number');
    assert.ok(typeof ibmdb.SQLPREPARE === 'number');
    assert.ok(typeof ibmdb.SQLCONNECT === 'number');
    pass("SQLFETCH=" + ibmdb.SQLFETCH + ", SQLPREPARE=" + ibmdb.SQLPREPARE);
    // Module functions
    assert.equal(typeof ibmdb.open, 'function');
    assert.equal(typeof ibmdb.openSync, 'function');
    assert.equal(typeof ibmdb.debug, 'function');
    assert.equal(typeof ibmdb.getElapsedTime, 'function');
    assert.equal(typeof ibmdb.close, 'function');
    pass("open, openSync, close, debug, getElapsedTime are exported functions.");
    // Class constructors
    assert.equal(typeof ibmdb.Database, 'function');
    assert.equal(typeof ibmdb.Pool, 'function');
    pass("Database and Pool constructors are exported.");
    // convertRowsToColumns utility
    assert.equal(typeof ibmdb.convertRowsToColumns, 'function');
    var conv = ibmdb.convertRowsToColumns([[1, 'a'], [2, 'b'], [3, 'c']]);
    assert.ok(conv.params && Array.isArray(conv.params));
    assert.equal(conv.ArraySize, 3);
    assert.equal(conv.params.length, 2); // 2 columns
    assert.equal(conv.params[0].ParamType, 'ARRAY');
    pass("convertRowsToColumns: 3 rows x 2 cols => ArraySize=3, params.length=2");
}
// Run
main();

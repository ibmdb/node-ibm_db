var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert");

var safetyTimerFired = false;
var safetyTimer = setTimeout(function() {
  safetyTimerFired = true;
}, 5000);

(async function() {
  var conn;
  try {
    conn = await ibmdb.open(common.connectionString);
    assert.strictEqual(safetyTimerFired, false,
      "open() promise continuation waited for unrelated timer activity");

    var rows = await conn.query("SELECT 1 AS X FROM SYSIBM.SYSDUMMY1");
    assert.strictEqual(rows[0].X, 1);
    assert.strictEqual(safetyTimerFired, false,
      "query() promise continuation waited for unrelated timer activity");

    await conn.close();
    assert.strictEqual(safetyTimerFired, false,
      "close() promise continuation waited for unrelated timer activity");

    console.log("Test 1082: passed");
  } finally {
    clearTimeout(safetyTimer);
    if (conn && conn.connected) await conn.close();
  }
})().catch(function(err) {
  clearTimeout(safetyTimer);
  console.error(err);
  process.exitCode = 1;
});

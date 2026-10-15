// Regression test for issue #1080:
// Column-wise ARRAY insert of BLOB values (Data: array of per-row Buffers)
// was mis-detected as a data-at-exec chunked LOB insert because parseParams()
// only looked at the shape of Data and ignored ParamType:"ARRAY".
var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert");

ibmdb.open(common.connectionString, function(err, conn) {
  if (err) {
    console.log(err);
    return;
  }

  try { conn.querySync("drop table array_blob_test"); } catch (e) {}
  conn.querySync("create table array_blob_test (id integer, b blob(1M))");

  var queryOptions = {
    sql: "insert into array_blob_test values (?, ?)",
    params: [
      { ParamType: "ARRAY", DataType: 1, Data: [1, 2, 3] },
      { ParamType: "ARRAY", SQLType: "BLOB", DataType: "BLOB",
        Data: [Buffer.from("one"), Buffer.from("two"), Buffer.from("three")], Length: 5 }
    ],
    ArraySize: 3
  };

  conn.querySync(queryOptions);

  var data = conn.querySync("select id, b from array_blob_test order by id");
  conn.querySync("drop table array_blob_test");
  conn.closeSync();

  assert.strictEqual(data.length, 3);
  assert.strictEqual(data[0].B.toString(), "one");
  assert.strictEqual(data[1].B.toString(), "two");
  assert.strictEqual(data[2].B.toString(), "three");

  console.log("Test 1080 : passed");
});

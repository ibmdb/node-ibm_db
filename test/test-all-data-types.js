var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , iszos = common.isZOS
    , cn = common.connectionString;

const expectedData =
[ { C1: 1,
    C2: 2,
    C3: '9007199254741997',
    C4: 1234,
    C5: 67.98,
    C6: 5689,
    C7: 56.239,
    C8: 34567890,
    C9: 45.234,
    C10: 'bimal     ',
    C11: 'kumar',
    C12: Buffer.from('P'),
    C13: 'jha123456',
    C14: '㐀㐁㐂㐃㐄㐅㐆',
    C15: '2015-09-10',
    C16: '10:16:33',
    C17: '2015-09-10 10:16:33.770139',
    C18: Buffer.from('abc'),
    C19: true } ];

const zosexpectedData =
[ { C1: 1,
    C2: 2,
    C3: '9007199254741997',
    C4: 1234,
    C5: 67.98,
    C6: 5689,
    C7: 56.239,
    C8: 34567890,
    C9: 45.234,
    C10: 'bimal     ',
    C11: 'kumar',
    C12: Buffer.from('P'),
    C13: 'jha123456',
    C14: '㐀㐁㐂㐃㐄㐅㐆',
    C15: '2015-09-10',
    C16: '10:16:33',
    C17: '2015-09-10 10:16:33.770139',
    C18: Buffer.from('abc') } ];

ibmdb.open(cn, function(err, conn) {
  if(err) console.log(err);
  assert.equal(err, null);

  try {
    conn.querySync("drop table mytab1");
  } catch (e) {}
    if (iszos) {
        conn.querySync("create table mytab1 (c1 int, c2 SMALLINT, c3 BIGINT, c4 INTEGER, c5 DECIMAL(4,2), c6 NUMERIC, c7 float, c8 double, c9 decfloat, c10 char(10), c11 varchar(10), c12 char for bit data, c13 clob(10),c14 dbclob(100), c15 date, c16 time, c17 timestamp, c18 blob(10)) ccsid unicode");
        conn.querySync("insert into mytab1 values (1, 2, 9007199254741997, 1234, 67.98, 5689, 56.2390, 34567890, 45.234, 'bimal', 'kumar', '\x50', 'jha123456','㐀㐁㐂㐃㐄㐅㐆','2015-09-10', '10:16:33', '2015-09-10 10:16:33.770139', BLOB(x'616263'))");
    }
    else {
        conn.querySync("create table mytab1 (c1 int, c2 SMALLINT, c3 BIGINT, c4 INTEGER, c5 DECIMAL(4,2), c6 NUMERIC, c7 float, c8 double, c9 decfloat, c10 char(10), c11 varchar(10), c12 char for bit data, c13 clob(10),c14 dbclob(100), c15 date, c16 time, c17 timestamp, c18 blob(10), c19 boolean) ccsid unicode");
        conn.querySync("insert into mytab1 values (1, 2, 9007199254741997, 1234, 67.98, 5689, 56.2390, 34567890, 45.234, 'bimal', 'kumar', '\x50', 'jha123456','㐀㐁㐂㐃㐄㐅㐆','2015-09-10', '10:16:33', '2015-09-10 10:16:33.770139', BLOB(x'616263'), true)");
    }
    conn.query("select * from mytab1", function (err, data) {
      if(err) console.log(err);
      else {
        console.log("Data1 = ", data);
      }
      if (iszos) {
            assert.deepEqual(data, zosexpectedData);
      }
      else {
            assert.deepEqual(data, expectedData);
      }
     
      conn.querySync("delete from mytab1");
      var blobParam = {DataType: "BLOB", Data: Buffer.from('abc')};
        if (iszos) {
            err = conn.querySync("insert into mytab1 values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", [1, 2, '9007199254741997', 1234, 67.98, 5689, 56.2390, 34567890,
                45.234, 'bimal', 'kumar', Buffer.from('P'), 'jha123456', '㐀㐁㐂㐃㐄㐅㐆', '2015-09-10',
                '10:16:33', '2015-09-10 10:16:33.770139', blobParam]);
        }
        else {
            err = conn.querySync("insert into mytab1 values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", [1, 2, '9007199254741997', 1234, 67.98, 5689, 56.2390, 34567890,
                45.234, 'bimal', 'kumar', Buffer.from('P'), 'jha123456', '㐀㐁㐂㐃㐄㐅㐆', '2015-09-10',
                '10:16:33', '2015-09-10 10:16:33.770139', blobParam, true]);
        }
        if(err.length) { console.log("Err = ", err); }
        else {
             data = conn.querySync("select * from mytab1");
             console.log("Data2 = ", data);
             if (iszos) {
                assert.deepEqual(data, zosexpectedData);
             }
             else {
               assert.deepEqual(data, expectedData);
             }
        }
        ibmdb.debug(2);
        conn.prepare("select * from mytab1", function(err, stmt) {
            stmt = stmt.executeSync();
            data = stmt.fetchAllSync();
            console.log("Data3 = ", data);
            console.log("Metadata = ", stmt.getColumnMetadataSync());
            stmt.closeSync();
            conn.querySync("drop table mytab1");
            conn.closeSync();
        });
    });
});

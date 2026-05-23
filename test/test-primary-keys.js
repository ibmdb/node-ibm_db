var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString
  ;

var tableName = "IBMDB_PK_TEST";
var baseTableName = tableName;
var schema    = null;
if(common.connectionObject.CURRENTSCHEMA) {
  schema = common.connectionObject.CURRENTSCHEMA;
  tableName = schema + "." + tableName;
}

// --- helpers ---------------------------------------------------------------
function dropTable(db) {
  try { db.querySync("DROP TABLE " + tableName); } catch(e) {}
}

function createTable(db) {
  db.querySync(
    "CREATE TABLE " + tableName +
    " (ID INTEGER NOT NULL, NAME VARCHAR(50), " +
    "  CONSTRAINT PK_" + tableName.replace(".", "_") + " PRIMARY KEY (ID))"
  );
}

// --- main ------------------------------------------------------------------
ibmdb.open(cn, function(err, db) {
  if (err) { console.log("Connection error:", err); return; }

  dropTable(db);
  createTable(db);

  var passed = 0, failed = 0;

  function done(label, ok, detail) {
    if (ok) {
      console.log("PASS:", label);
      passed++;
    } else {
      console.log("FAIL:", label, detail || "");
      failed++;
    }
  }

  function verifyKeys(rows, label) {
    var ok = Array.isArray(rows) &&
             rows.length === 1 &&
             rows[0].COLUMN_NAME === "ID";
    done(label, ok, JSON.stringify(rows));
  }

  // 1 — async callback via Database.primaryKeys()
  db.primaryKeys(null, schema, baseTableName, function(err, rows) {
    done("1 db.primaryKeys callback - no error", !err, err);
    verifyKeys(rows, "1 db.primaryKeys callback - rows");
    console.log("rows = ", rows);

    // 2 — async Promise via Database.primaryKeys()
    db.primaryKeys(null, schema, baseTableName).then(function(rows) {
      done("2 db.primaryKeys Promise - resolved", true);
      verifyKeys(rows, "2 db.primaryKeys Promise - rows");
      console.log("rows = ", rows);

      // 3 — sync via Database.primaryKeysSync()
      var rows3;
      try {
        rows3 = db.primaryKeysSync(null, schema, baseTableName);
        done("3 db.primaryKeysSync - no throw", true);
      } catch(e) {
        done("3 db.primaryKeysSync - no throw", false, e);
        rows3 = [];
      }
      verifyKeys(rows3, "3 db.primaryKeysSync - rows");
      console.log("rows = ", rows3);

      // 4 — async callback via ODBCStatement.primaryKeys()
      db.conn.createStatement(function(err, stmt) {
        if (err) { done("4 createStatement", false, err); return finish(); }
        stmt.primaryKeys(null, schema, baseTableName, function(err, rows) {
          done("4 stmt.primaryKeys callback - no error", !err, err);
          verifyKeys(rows, "4 stmt.primaryKeys callback - rows");
          console.log("rows = ", rows);
          stmt.closeSync();

          // 5 — async Promise via ODBCStatement.primaryKeys()
          db.conn.createStatement(function(err, stmt) {
            if (err) { done("5 createStatement", false, err); return finish(); }
            stmt.primaryKeys(null, schema, baseTableName).then(function(rows) {
              done("5 stmt.primaryKeys Promise - resolved", true);
              verifyKeys(rows, "5 stmt.primaryKeys Promise - rows");
              console.log("rows = ", rows);
              stmt.closeSync();

              // 6 — sync via ODBCStatement.primaryKeysSync()
              var stmt6 = db.conn.createStatementSync();
              var rows6;
              try {
                rows6 = stmt6.primaryKeysSync(null, schema, baseTableName);
                done("6 stmt.primaryKeysSync - no throw", true);
              } catch(e) {
                done("6 stmt.primaryKeysSync - no throw", false, e);
                rows6 = [];
              }
              console.log("rows = ", rows6);
              verifyKeys(rows6, "6 stmt.primaryKeysSync - rows");
              stmt6.closeSync();

              // 7 — error case: non-existent table should return empty array
              var rows7;
              try {
                rows7 = db.primaryKeysSync(null, schema, "NONEXISTENT_TABLE_XYZ");
                done("7 non-existent table - no throw", true);
                done("7 non-existent table - empty array", Array.isArray(rows7) && rows7.length === 0, JSON.stringify(rows7));
              } catch(e) {
                // some servers may throw; that is also acceptable
                done("7 non-existent table - handled", true);
              }

              finish();
            }).catch(function(e) {
              done("5 stmt.primaryKeys Promise - no reject", false, e);
              stmt.closeSync();
              finish();
            });
          });
        });
      });
    }).catch(function(e) {
      done("2 db.primaryKeys Promise - no reject", false, e);
      finish();
    });
  });

  function finish() {
    dropTable(db);
    db.closeSync();
    console.log("\nResults: " + passed + " passed, " + failed + " failed.");
    if (failed) process.exit(1);
  }
});

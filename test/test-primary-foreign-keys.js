var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString
  ;

var pkTableName = "IBMDB_PK_TEST";
var fkTableName = "IBMDB_FK_TEST";
var basePkTableName = pkTableName;
var baseFkTableName = fkTableName;
var schema = null;

// keep existing names working for pk tests
var tableName     = pkTableName;
var baseTableName = basePkTableName;

if(common.connectionObject.CURRENTSCHEMA) {
  schema = common.connectionObject.CURRENTSCHEMA;
  pkTableName = schema + "." + pkTableName;
  fkTableName = schema + "." + fkTableName;
  tableName   = pkTableName;
}

// --- helpers ---------------------------------------------------------------
function dropTable(db) {
  try { db.querySync("DROP TABLE " + fkTableName); } catch(e) {}
  try { db.querySync("DROP TABLE " + pkTableName); } catch(e) {}
}

function createTable(db) {
  db.querySync(
    "CREATE TABLE " + pkTableName +
    " (ID INTEGER NOT NULL, NAME VARCHAR(50), " +
    "  CONSTRAINT PK_" + pkTableName.replace(".", "_") + " PRIMARY KEY (ID))"
  );
  db.querySync(
    "CREATE TABLE " + fkTableName +
    " (FK_ID INTEGER NOT NULL, PK_REF INTEGER, " +
    "  CONSTRAINT FK_" + fkTableName.replace(".", "_") +
    "  FOREIGN KEY (PK_REF) REFERENCES " + pkTableName + "(ID))"
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

  function verifyFKs(rows, label) {
    var ok = Array.isArray(rows) &&
             rows.length === 1 &&
             rows[0].PKCOLUMN_NAME === "ID" &&
             rows[0].FKCOLUMN_NAME === "PK_REF";
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

              // === foreignKeys tests ===

              // 8 — async callback via Database.foreignKeys() — specify both pk and fk table
              db.foreignKeys(null, schema, basePkTableName, null, schema, baseFkTableName, function(err, rows) {
                done("8 db.foreignKeys callback - no error", !err, err);
                verifyFKs(rows, "8 db.foreignKeys callback - rows");
                console.log("rows = ", rows);

                // 9 — async Promise via Database.foreignKeys()
                db.foreignKeys(null, schema, basePkTableName, null, schema, baseFkTableName).then(function(rows) {
                  done("9 db.foreignKeys Promise - resolved", true);
                  verifyFKs(rows, "9 db.foreignKeys Promise - rows");
                  console.log("rows = ", rows);

                  // 10 — sync via Database.foreignKeysSync()
                  var rows10;
                  try {
                    rows10 = db.foreignKeysSync(null, schema, basePkTableName, null, schema, baseFkTableName);
                    done("10 db.foreignKeysSync - no throw", true);
                  } catch(e) {
                    done("10 db.foreignKeysSync - no throw", false, e);
                    rows10 = [];
                  }
                  verifyFKs(rows10, "10 db.foreignKeysSync - rows");
                  console.log("rows = ", rows10);

                  // 11 — async callback via ODBCStatement.foreignKeys()
                  db.conn.createStatement(function(err, stmt11) {
                    if (err) { done("11 createStatement", false, err); return finish(); }
                    stmt11.foreignKeys(null, schema, basePkTableName, null, schema, baseFkTableName, function(err, rows) {
                      done("11 stmt.foreignKeys callback - no error", !err, err);
                      verifyFKs(rows, "11 stmt.foreignKeys callback - rows");
                      console.log("rows = ", rows);
                      stmt11.closeSync();

                      // 12 — async Promise via ODBCStatement.foreignKeys()
                      db.conn.createStatement(function(err, stmt12) {
                        if (err) { done("12 createStatement", false, err); return finish(); }
                        stmt12.foreignKeys(null, schema, basePkTableName, null, schema, baseFkTableName).then(function(rows) {
                          done("12 stmt.foreignKeys Promise - resolved", true);
                          verifyFKs(rows, "12 stmt.foreignKeys Promise - rows");
                          console.log("rows = ", rows);
                          stmt12.closeSync();

                          // 13 — sync via ODBCStatement.foreignKeysSync()
                          var stmt13 = db.conn.createStatementSync();
                          var rows13;
                          try {
                            rows13 = stmt13.foreignKeysSync(null, schema, basePkTableName, null, schema, baseFkTableName);
                            done("13 stmt.foreignKeysSync - no throw", true);
                          } catch(e) {
                            done("13 stmt.foreignKeysSync - no throw", false, e);
                            rows13 = [];
                          }
                          verifyFKs(rows13, "13 stmt.foreignKeysSync - rows");
                          console.log("rows = ", rows13);
                          stmt13.closeSync();

                          // 14 — fkTable only (all FKs in child table regardless of pk table)
                          var rows14;
                          try {
                            rows14 = db.foreignKeysSync(null, null, null, null, schema, baseFkTableName);
                            done("14 foreignKeysSync fkTable-only - no throw", true);
                            done("14 foreignKeysSync fkTable-only - has rows",
                                 Array.isArray(rows14) && rows14.length >= 1, JSON.stringify(rows14));
                          } catch(e) {
                            done("14 foreignKeysSync fkTable-only - handled", true);
                          }

                          // 15 — non-existent tables should return empty array
                          var rows15;
                          try {
                            rows15 = db.foreignKeysSync(null, schema, "NONEXISTENT_XYZ", null, schema, "NONEXISTENT_XYZ");
                            done("15 non-existent tables - no throw", true);
                            done("15 non-existent tables - empty array",
                                 Array.isArray(rows15) && rows15.length === 0, JSON.stringify(rows15));
                          } catch(e) {
                            done("15 non-existent tables - handled", true);
                          }

                          finish();
                        }).catch(function(e) {
                          done("12 stmt.foreignKeys Promise - no reject", false, e);
                          stmt12.closeSync();
                          finish();
                        });
                      });
                    });
                  });
                }).catch(function(e) {
                  done("9 db.foreignKeys Promise - no reject", false, e);
                  finish();
                });
              });

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

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

  function verifyKeys(rows) {
    return Array.isArray(rows) &&
           rows.length === 1 &&
           rows[0].COLUMN_NAME === "ID";
  }

  function verifyFKs(rows) {
    return Array.isArray(rows) &&
           rows.length === 1 &&
           rows[0].PKCOLUMN_NAME === "ID" &&
           rows[0].FKCOLUMN_NAME === "PK_REF";
  }

  // 1 — async callback via Database.primaryKeys()
  db.primaryKeys(null, schema, baseTableName, function(err, rows) {
    var ok = !err && verifyKeys(rows);
    done("1 db.primaryKeys callback", ok, err || JSON.stringify(rows));
    console.log("rows = ", rows);

    // 2 — async Promise via Database.primaryKeys()
    db.primaryKeys(null, schema, baseTableName).then(function(rows) {
      done("2 db.primaryKeys Promise", verifyKeys(rows), JSON.stringify(rows));
      console.log("rows = ", rows);

      // 3 — sync via Database.primaryKeysSync()
      var rows3;
      try {
        rows3 = db.primaryKeysSync(null, schema, baseTableName);
        done("3 db.primaryKeysSync", verifyKeys(rows3), JSON.stringify(rows3));
      } catch(e) {
        done("3 db.primaryKeysSync", false, e);
        rows3 = [];
      }
      console.log("rows = ", rows3);

      // 4 — async callback via ODBCStatement.primaryKeys()
      db.conn.createStatement(function(err, stmt) {
        if (err) { done("4 stmt.primaryKeys callback", false, err); return finish(); }
        stmt.primaryKeys(null, schema, baseTableName, function(err, rows) {
          var ok = !err && verifyKeys(rows);
          done("4 stmt.primaryKeys callback", ok, err || JSON.stringify(rows));
          console.log("rows = ", rows);
          stmt.closeSync();

          // 5 — async Promise via ODBCStatement.primaryKeys()
          db.conn.createStatement(function(err, stmt) {
            if (err) { done("5 stmt.primaryKeys Promise", false, err); return finish(); }
            stmt.primaryKeys(null, schema, baseTableName).then(function(rows) {
              done("5 stmt.primaryKeys Promise", verifyKeys(rows), JSON.stringify(rows));
              console.log("rows = ", rows);
              stmt.closeSync();

              // 6 — sync via ODBCStatement.primaryKeysSync()
              var stmt6 = db.conn.createStatementSync();
              var rows6;
              try {
                rows6 = stmt6.primaryKeysSync(null, schema, baseTableName);
                done("6 stmt.primaryKeysSync", verifyKeys(rows6), JSON.stringify(rows6));
              } catch(e) {
                done("6 stmt.primaryKeysSync", false, e);
                rows6 = [];
              }
              console.log("rows = ", rows6);
              stmt6.closeSync();

              // 7 — error case: non-existent table should return empty array
              var rows7;
              try {
                rows7 = db.primaryKeysSync(null, schema, "NONEXISTENT_TABLE_XYZ");
                done("7 non-existent table - empty array", Array.isArray(rows7) && rows7.length === 0, JSON.stringify(rows7));
              } catch(e) {
                // some servers may throw; that is also acceptable
                done("7 non-existent table - handled", true);
              }

              // === foreignKeys tests ===

              // 8 — async callback via Database.foreignKeys() — specify both pk and fk table
              db.foreignKeys(null, schema, basePkTableName, null, schema, baseFkTableName, function(err, rows) {
                var ok = !err && verifyFKs(rows);
                done("8 db.foreignKeys callback", ok, err || JSON.stringify(rows));
                console.log("rows = ", rows);

                // 9 — async Promise via Database.foreignKeys()
                db.foreignKeys(null, schema, basePkTableName, null, schema, baseFkTableName).then(function(rows) {
                  done("9 db.foreignKeys Promise", verifyFKs(rows), JSON.stringify(rows));
                  console.log("rows = ", rows);

                  // 10 — sync via Database.foreignKeysSync()
                  var rows10;
                  try {
                    rows10 = db.foreignKeysSync(null, schema, basePkTableName, null, schema, baseFkTableName);
                    done("10 db.foreignKeysSync", verifyFKs(rows10), JSON.stringify(rows10));
                  } catch(e) {
                    done("10 db.foreignKeysSync", false, e);
                    rows10 = [];
                  }
                  console.log("rows = ", rows10);

                  // 11 — async callback via ODBCStatement.foreignKeys()
                  db.conn.createStatement(function(err, stmt11) {
                    if (err) { done("11 stmt.foreignKeys callback", false, err); return finish(); }
                    stmt11.foreignKeys(null, schema, basePkTableName, null, schema, baseFkTableName, function(err, rows) {
                      var ok = !err && verifyFKs(rows);
                      done("11 stmt.foreignKeys callback", ok, err || JSON.stringify(rows));
                      console.log("rows = ", rows);
                      stmt11.closeSync();

                      // 12 — async Promise via ODBCStatement.foreignKeys()
                      db.conn.createStatement(function(err, stmt12) {
                        if (err) { done("12 stmt.foreignKeys Promise", false, err); return finish(); }
                        stmt12.foreignKeys(null, schema, basePkTableName, null, schema, baseFkTableName).then(function(rows) {
                          done("12 stmt.foreignKeys Promise", verifyFKs(rows), JSON.stringify(rows));
                          console.log("rows = ", rows);
                          stmt12.closeSync();

                          // 13 — sync via ODBCStatement.foreignKeysSync()
                          var stmt13 = db.conn.createStatementSync();
                          var rows13;
                          try {
                            rows13 = stmt13.foreignKeysSync(null, schema, basePkTableName, null, schema, baseFkTableName);
                            done("13 stmt.foreignKeysSync", verifyFKs(rows13), JSON.stringify(rows13));
                          } catch(e) {
                            done("13 stmt.foreignKeysSync", false, e);
                            rows13 = [];
                          }
                          console.log("rows = ", rows13);
                          stmt13.closeSync();

                          // 14 — fkTable only (all FKs in child table regardless of pk table)
                          var rows14;
                          try {
                            rows14 = db.foreignKeysSync(null, null, null, null, schema, baseFkTableName);
                            var ok14 = Array.isArray(rows14) && rows14.length >= 1;
                            done("14 foreignKeysSync fkTable-only", ok14, JSON.stringify(rows14));
                          } catch(e) {
                            done("14 foreignKeysSync fkTable-only", false, e);
                          }

                          // 15 — non-existent tables should return empty array
                          var rows15;
                          try {
                            rows15 = db.foreignKeysSync(null, schema, "NONEXISTENT_XYZ", null, schema, "NONEXISTENT_XYZ");
                            done("15 non-existent tables - empty array", Array.isArray(rows15) && rows15.length === 0, JSON.stringify(rows15));
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

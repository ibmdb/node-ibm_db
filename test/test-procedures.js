var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString
  ;

var procName = "MYPROC3";
var baseProcName = procName;
var schema = null;

if(common.connectionObject.CURRENTSCHEMA) {
  schema = common.connectionObject.CURRENTSCHEMA;
  procName = schema + "." + procName;
}

// --- helpers ---------------------------------------------------------------
function dropProc(db) {
  try { db.querySync("DROP PROCEDURE " + procName); } catch(e) {}
}

function createProc(db) {
  // Create a simple stored procedure for testing
  db.querySync(
    "CREATE PROCEDURE " + procName + " (IN p_input INTEGER, OUT p_output INTEGER) " +
    "LANGUAGE SQL " +
    "BEGIN " +
    "  SET p_output = p_input * 2; " +
    "END"
  );
}

// --- main ------------------------------------------------------------------
ibmdb.open(cn, function(err, db) {
  if (err) { console.log("Connection error:", err); return; }

  dropProc(db);
  createProc(db);

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

  function verifyProcs(rows) {
    return Array.isArray(rows) &&
           rows.length >= 1 &&
           rows.some(function(r) {
             return r.PROCEDURE_NAME === baseProcName ||
                    r.PROCEDURE_NAME === baseProcName.toUpperCase();
           });
  }

  // 1 — async callback via Database.procedures()
  db.procedures(null, schema, baseProcName, function(err, rows) {
    var ok = !err && verifyProcs(rows);
    done("1 db.procedures callback", ok, err || JSON.stringify(rows));
    console.log("rows = ", rows);

    // 2 — async Promise via Database.procedures()
    db.procedures(null, schema, baseProcName).then(function(rows) {
      var ok = verifyProcs(rows);
      done("2 db.procedures Promise", ok, JSON.stringify(rows));
      console.log("rows = ", rows);

      // 3 — sync via Database.proceduresSync()
      var rows3;
      try {
        rows3 = db.proceduresSync(null, schema, baseProcName);
        done("3 db.proceduresSync", verifyProcs(rows3), JSON.stringify(rows3));
      } catch(e) {
        done("3 db.proceduresSync", false, e);
        rows3 = [];
      }
      console.log("rows = ", rows3);

      // 4 — async callback via ODBCStatement.procedures()
      db.conn.createStatement(function(err, stmt) {
        if (err) { done("4 stmt.procedures callback", false, err); return finish(); }
        stmt.procedures(null, schema, baseProcName, function(err, rows) {
          var ok = !err && verifyProcs(rows);
          done("4 stmt.procedures callback", ok, err || JSON.stringify(rows));
          console.log("rows = ", rows);
          stmt.closeSync();

          // 5 — async Promise via ODBCStatement.procedures()
          db.conn.createStatement(function(err, stmt) {
            if (err) { done("5 stmt.procedures Promise", false, err); return finish(); }
            stmt.procedures(null, schema, baseProcName).then(function(rows) {
              var ok = verifyProcs(rows);
              done("5 stmt.procedures Promise", ok, JSON.stringify(rows));
              console.log("rows = ", rows);
              stmt.closeSync();

              // 6 — sync via ODBCStatement.proceduresSync()
              var stmt6 = db.conn.createStatementSync();
              var rows6;
              try {
                rows6 = stmt6.proceduresSync(null, schema, baseProcName);
                done("6 stmt.proceduresSync", verifyProcs(rows6), JSON.stringify(rows6));
              } catch(e) {
                done("6 stmt.proceduresSync", false, e);
                rows6 = [];
              }
              console.log("rows = ", rows6);
              stmt6.closeSync();

              // 7 — test with wildcard pattern (%)
              var rows7;
              try {
                rows7 = db.proceduresSync(null, schema, "MYPROC%");
                var ok7 = Array.isArray(rows7) && rows7.length >= 1;
                done("7 wildcard pattern", ok7, JSON.stringify(rows7));
              } catch(e) {
                done("7 wildcard pattern", false, e);
              }

              // 8 — non-existent procedure should return empty array
              var rows8;
              try {
                rows8 = db.proceduresSync(null, schema, "NONEXISTENT_PROC_XYZ");
                done("8 non-existent procedure - empty array", Array.isArray(rows8) && rows8.length === 0, JSON.stringify(rows8));
              } catch(e) {
                // some servers may throw; that is also acceptable
                done("8 non-existent procedure - handled", true);
              }

              finish();

            }).catch(function(err) {
              done("5 stmt.procedures Promise", false, err);
              stmt.closeSync();
              finish();
            });
          });
        });
      });

    }).catch(function(err) {
      done("2 db.procedures Promise", false, err);
      finish();
    });
  });

  function finish() {
    dropProc(db);
    db.closeSync();
    console.log("\n-----------------------------------------");
    console.log("Passed:", passed, "/ Failed:", failed);
    if (failed > 0) {
      process.exit(1);
    }
  }
});

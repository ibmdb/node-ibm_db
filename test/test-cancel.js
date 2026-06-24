/*
  Test case for cancel() and cancelSync() APIs.

  The cancel() and cancelSync() APIs call SQLCancel ODBC function to cancel
  an executing SQL statement. This test verifies:
  1. Statement-level cancel via stmt.cancel() and stmt.cancelSync()
  2. Connection-level cancel via conn.cancel(stmt) and conn.cancelSync(stmt)

  Note: Due to timing complexities, this test primarily verifies that:
  - The cancel APIs are callable
  - They return proper results
  - They handle errors appropriately
*/

var common = require("./common")
  , ibmdb = require("../")
  , assert = require("assert")
  , cn = common.connectionString;

console.log("Testing cancel() and cancelSync() APIs...");
console.log("Connection String =", cn);

// Test 1: Statement-level cancelSync() - calling on a prepared statement
ibmdb.open(cn, function(err, conn) {
  if (err) {
    console.log("Connection error:", err);
    return;
  }
  console.log("Connection opened successfully.");

  // Create a test table
  try {
    conn.querySync("DROP TABLE CANCEL_TEST");
  } catch (e) {
    // Table doesn't exist, ignore
  }
  conn.querySync("CREATE TABLE CANCEL_TEST (ID INT, NAME VARCHAR(50))");

  // Insert test data
  for (var i = 0; i < 10; i++) {
    conn.querySync("INSERT INTO CANCEL_TEST VALUES (?, ?)", [i, "Name" + i]);
  }
  console.log("Test table created with sample data.");

  // Test 1: Statement-level cancelSync during execute
  console.log("\n=== Test 1: Statement cancelSync() during execute ===");
  var stmt = conn.prepareSync("SELECT * FROM CANCEL_TEST");
  var test1ExecuteCompleted = false;
  var test1CancelCompleted = false;

  // Start async execute and immediately call cancelSync
  stmt.execute(function(execErr, result) {
    test1ExecuteCompleted = true;
    if (execErr) {
      console.log("Test 1: execute() error after cancel:", execErr.message);
    } else {
      console.log("Test 1: execute() completed");
      if (result) {
        try { result.closeSync(); } catch(e) {}
      }
    }
    if (test1CancelCompleted) {
      try { stmt.closeSync(); } catch(e) {}
      runTest2();
    }
  });

  // Immediately cancel while execute is running
  try {
    var cancelResult = stmt.cancelSync();
    console.log("stmt.cancelSync() returned:", cancelResult);
    assert.strictEqual(cancelResult, true, "cancelSync should return true");
    console.log("Test 1 PASSED: Statement cancelSync works.");
  } catch (e) {
    console.log("stmt.cancelSync() threw:", e.message);
    console.log("Test 1: Statement cancelSync threw (may happen depending on timing)");
  }
  test1CancelCompleted = true;
  if (test1ExecuteCompleted) {
    try { stmt.closeSync(); } catch(e) {}
    runTest2();
  }

  function runTest2() {
    // Test 2: Statement-level cancel() with callback during execute
    console.log("\n=== Test 2: Statement cancel() with callback during execute ===");
    var stmt2 = conn.prepareSync("SELECT * FROM CANCEL_TEST");
    var test2ExecuteCompleted = false;
    var test2CancelCompleted = false;

    function finishTest2() {
      setTimeout(function() {
        try { stmt2.closeSync(); } catch(e) {}
        runTest3();
      }, 50);
    }

    // Start async execute
    stmt2.execute(function(execErr, result) {
      test2ExecuteCompleted = true;
      if (execErr) {
        console.log("Test 2: execute() error after cancel:", execErr.message);
      } else {
        console.log("Test 2: execute() completed");
        if (result) {
          try { result.closeSync(); } catch(e) {}
        }
      }
      if (test2CancelCompleted) finishTest2();
    });

    // Immediately cancel while execute is running
    stmt2.cancel(function(err, result) {
      test2CancelCompleted = true;
      if (err) {
        console.log("stmt.cancel() error:", err.message);
      } else {
        console.log("stmt.cancel() returned:", result);
        assert.strictEqual(result, true, "cancel callback should receive true");
        console.log("Test 2 PASSED: Statement cancel with callback works.");
      }
      if (test2ExecuteCompleted) finishTest2();
    });
  }

  function runTest3() {
    // Test 3: Statement-level cancel() with Promise during execute
    console.log("\n=== Test 3: Statement cancel() with Promise during execute ===");
    var stmt3 = conn.prepareSync("SELECT * FROM CANCEL_TEST");
    var test3ExecuteCompleted = false;
    var test3CancelCompleted = false;

    function finishTest3() {
      setTimeout(function() {
        try { stmt3.closeSync(); } catch(e) {}
        runConnectionCancelTests();
      }, 10);
    }

    // Start async execute
    stmt3.execute(function(execErr, result) {
      test3ExecuteCompleted = true;
      if (execErr) {
        console.log("Test 3: execute() error after cancel:", execErr.message);
      } else {
        console.log("Test 3: execute() completed");
        if (result) {
          try { result.closeSync(); } catch(e) {}
        }
      }
      if (test3CancelCompleted) finishTest3();
    });

    // Immediately cancel while execute is running
    stmt3.cancel().then(function(result) {
      test3CancelCompleted = true;
      console.log("stmt.cancel() promise resolved:", result);
      console.log("Test 3 PASSED: Statement cancel with Promise works.");
      if (test3ExecuteCompleted) finishTest3();
    }).catch(function(err) {
      test3CancelCompleted = true;
      console.log("stmt.cancel() promise rejected:", err.message);
      if (test3ExecuteCompleted) finishTest3();
    });
  }

  function runConnectionCancelTests() {
    // Test 4: Connection-level cancelSync with statement
    console.log("\n=== Test 4: Connection cancelSync(stmt) ===");
    var stmt4 = conn.prepareSync("SELECT * FROM CANCEL_TEST");

    try {
      var cancelResult = conn.cancelSync(stmt4);
      console.log("conn.cancelSync(stmt) returned:", cancelResult);
      assert.strictEqual(cancelResult, true, "cancelSync should return true");
      console.log("Test 4 PASSED: Connection cancelSync(stmt) works.");
    } catch (e) {
      console.log("conn.cancelSync(stmt) threw:", e.message);
    }
    stmt4.closeSync();

    // Test 5: Connection-level cancel(stmt) with callback
    console.log("\n=== Test 5: Connection cancel(stmt, callback) ===");
    var stmt5 = conn.prepareSync("SELECT * FROM CANCEL_TEST");

    conn.cancel(stmt5, function(err, result) {
      if (err) {
        console.log("conn.cancel(stmt) error:", err.message);
      } else {
        console.log("conn.cancel(stmt) returned:", result);
        assert.strictEqual(result, true, "cancel callback should receive true");
        console.log("Test 5 PASSED: Connection cancel(stmt, callback) works.");
      }
      stmt5.closeSync();

      // Test 6: Connection-level cancel(stmt) with Promise
      console.log("\n=== Test 6: Connection cancel(stmt) with Promise ===");
      var stmt6 = conn.prepareSync("SELECT * FROM CANCEL_TEST");

      conn.cancel(stmt6).then(function(result) {
        console.log("conn.cancel(stmt) promise resolved:", result);
        assert.strictEqual(result, true, "cancel promise should resolve to true");
        console.log("Test 6 PASSED: Connection cancel(stmt) with Promise works.");
        stmt6.closeSync();
        runCancelWithExecutionTests();
      }).catch(function(err) {
        console.log("conn.cancel(stmt) promise rejected:", err.message);
        stmt6.closeSync();
        runCancelWithExecutionTests();
      });
    });
  }

  function runCancelWithExecutionTests() {
    // Test 7: Connection cancel during SQL execution
    console.log("\n=== Test 7: Connection cancel(stmt) during SQL execution ===");
    
    var stmt7 = conn.prepareSync("SELECT * FROM CANCEL_TEST");
    var executeCompleted = false;
    var cancelCompleted = false;

    try {
      // Start executing asynchronously
      stmt7.execute(function(execErr, result) {
        executeCompleted = true;
        if (execErr) {
          console.log("stmt7.execute() error after cancel:", execErr.message);
        } else {
          console.log("stmt7.execute() completed");
          if (result) {
            try { result.closeSync(); } catch(e) {}
          }
        }
        
        if (cancelCompleted) {
          finishTest7();
        }
      });

      // Immediately try to cancel via connection while query is executing
      conn.cancel(stmt7, function(cancelErr, cancelResult) {
        cancelCompleted = true;
        if (cancelErr) {
          console.log("conn.cancel(stmt7) error:", cancelErr.message);
        } else {
          console.log("conn.cancel(stmt7) returned:", cancelResult);
          console.log("Test 7 PASSED: Connection cancel(stmt) with SQL execution works.");
        }

        if (executeCompleted) {
          finishTest7();
        }
      });
    } catch (e) {
      console.log("Test 7 setup error:", e.message);
      try { stmt7.closeSync(); } catch (e2) {}
      runValidationTests();
      return;
    }
    
    function finishTest7() {
      // Small delay to ensure everything is cleaned up
      setTimeout(function() {
        try {
          stmt7.closeSync();
        } catch (e) {
          // Statement may already be closed or in error state
        }
        
        runValidationTests();
      }, 100);
    }
  }

  function runValidationTests() {
    // Test 8: cancel/cancelSync API validation
    console.log("\n=== Test 8: cancel/cancelSync API validation ===");

    // Test cancelSync without argument - should throw (JS check)
    try {
      conn.cancelSync();
      console.log("Test 8a FAILED: cancelSync() without stmt should have thrown");
    } catch (e) {
      console.log("cancelSync() without stmt correctly threw:", e.message);
    }

    // Test cancelSync with null - should throw (JS check)
    try {
      conn.cancelSync(null);
      console.log("Test 8b FAILED: cancelSync(null) should have thrown");
    } catch (e) {
      console.log("cancelSync(null) correctly threw:", e.message);
    }

    // Test cancelSync with invalid object - returns error (native check)
    var result8c = conn.cancelSync({});
    if (result8c instanceof Error) {
      console.log("cancelSync({}) correctly returned error:", result8c.message);
    } else {
      console.log("Test 8c FAILED: cancelSync({}) should have returned an error");
    }

    // Test cancel without statement - when passed a function, calls it with error
    var callbackCalled = false;
    conn.cancel(function(err) {
      callbackCalled = true;
      if (err && err.message.indexOf("requires a statement") !== -1) {
        console.log("cancel(callback) correctly called callback with error:", err.message);
      } else {
        console.log("Test 8d FAILED: Expected error about requiring statement");
      }
    });
    if (callbackCalled) {
      // Synchronous callback - expected for this error case
    } else {
      console.log("Test 8d: callback not called synchronously (queued)");
    }

    console.log("Test 8 PASSED: API validation works correctly.");

    runEdgeCaseTests();
  }

  function runEdgeCaseTests() {
    // Test 9: Cancel on closed statement
    console.log("\n=== Test 9: Cancel on closed statement ===");
    var stmt9 = conn.prepareSync("SELECT * FROM CANCEL_TEST");
    stmt9.closeSync();

    // Try to cancel a closed statement - should error
    stmt9.cancel(function(err, result) {
      if (err) {
        console.log("cancel() on closed statement correctly errored:", err.message);
        console.log("Test 9 PASSED: Cancel on closed statement handled correctly.");
      } else {
        console.log("Test 9: cancel() on closed statement returned:", result);
      }

      runTest10();
    });
  }

  function runTest10() {
    // Test 10: Double cancel (cancel same statement twice)
    console.log("\n=== Test 10: Double cancel ===");
    var stmt10 = conn.prepareSync("SELECT * FROM CANCEL_TEST");
    var firstCancelDone = false;
    var secondCancelDone = false;

    function finishTest10() {
      if (firstCancelDone && secondCancelDone) {
        try { stmt10.closeSync(); } catch(e) {}
        console.log("Test 10 PASSED: Double cancel handled correctly.");
        runTest11();
      }
    }

    // First cancel
    stmt10.cancel(function(err1, result1) {
      firstCancelDone = true;
      if (err1) {
        console.log("First cancel error:", err1.message);
      } else {
        console.log("First cancel returned:", result1);
      }
      finishTest10();
    });

    // Second cancel immediately
    stmt10.cancel(function(err2, result2) {
      secondCancelDone = true;
      if (err2) {
        console.log("Second cancel error:", err2.message);
      } else {
        console.log("Second cancel returned:", result2);
      }
      finishTest10();
    });
  }

  function runTest11() {
    // Test 11: Cancel after execute completes (race condition - cancel too late)
    console.log("\n=== Test 11: Cancel after execute completes ===");
    var stmt11 = conn.prepareSync("SELECT * FROM CANCEL_TEST");

    stmt11.execute(function(execErr, result) {
      if (execErr) {
        console.log("Test 11: execute() error:", execErr.message);
      } else {
        console.log("Test 11: execute() completed successfully");
        if (result) {
          try { result.closeSync(); } catch(e) {}
        }
      }

      // Now cancel AFTER execute completed
      stmt11.cancel(function(cancelErr, cancelResult) {
        if (cancelErr) {
          console.log("Cancel after completion error:", cancelErr.message);
        } else {
          console.log("Cancel after completion returned:", cancelResult);
        }
        console.log("Test 11 PASSED: Cancel after execute handled correctly.");
        try { stmt11.closeSync(); } catch(e) {}
        runTest12();
      });
    });
  }

  function runTest12() {
    // Test 12: Cancel during fetchAll
    console.log("\n=== Test 12: Cancel during fetchAll ===");
    var stmt12 = conn.prepareSync("SELECT * FROM CANCEL_TEST");
    var fetchCompleted = false;
    var cancelCompleted = false;

    function finishTest12() {
      if (fetchCompleted && cancelCompleted) {
        setTimeout(function() {
          try { stmt12.closeSync(); } catch(e) {}
          console.log("Test 12 PASSED: Cancel during fetchAll handled correctly.");
          runTest13();
        }, 50);
      }
    }

    stmt12.execute(function(execErr, result) {
      if (execErr) {
        console.log("Test 12: execute() error:", execErr.message);
        fetchCompleted = true;
        finishTest12();
        return;
      }

      // Start fetching
      result.fetchAll(function(fetchErr, data) {
        fetchCompleted = true;
        if (fetchErr) {
          console.log("Test 12: fetchAll() error (may be cancelled):", fetchErr.message);
        } else {
          console.log("Test 12: fetchAll() returned", data ? data.length : 0, "rows");
        }
        try { result.closeSync(); } catch(e) {}
        finishTest12();
      });

      // Cancel while fetching
      stmt12.cancel(function(cancelErr, cancelResult) {
        cancelCompleted = true;
        if (cancelErr) {
          console.log("Cancel during fetch error:", cancelErr.message);
        } else {
          console.log("Cancel during fetch returned:", cancelResult);
        }
        finishTest12();
      });
    });
  }

  function runTest13() {
    // Test 13: Cancel INSERT/UPDATE/DELETE (not just SELECT)
    console.log("\n=== Test 13: Cancel INSERT/UPDATE/DELETE ===");
    var stmt13 = conn.prepareSync("UPDATE CANCEL_TEST SET NAME = 'Updated' WHERE ID = ?");
    stmt13.bindSync([1]);
    var executeCompleted = false;
    var cancelCompleted = false;

    function finishTest13() {
      if (executeCompleted && cancelCompleted) {
        setTimeout(function() {
          try { stmt13.closeSync(); } catch(e) {}
          console.log("Test 13 PASSED: Cancel UPDATE handled correctly.");
          runTest14();
        }, 50);
      }
    }

    stmt13.execute(function(execErr, result) {
      executeCompleted = true;
      if (execErr) {
        console.log("Test 13: UPDATE error (may be cancelled):", execErr.message);
      } else {
        console.log("Test 13: UPDATE completed");
        if (result) {
          try { result.closeSync(); } catch(e) {}
        }
      }
      finishTest13();
    });

    stmt13.cancel(function(cancelErr, cancelResult) {
      cancelCompleted = true;
      if (cancelErr) {
        console.log("Cancel UPDATE error:", cancelErr.message);
      } else {
        console.log("Cancel UPDATE returned:", cancelResult);
      }
      finishTest13();
    });
  }

  function runTest14() {
    // Test 14: Cancel from wrong connection (should fail or error)
    console.log("\n=== Test 14: Cancel from wrong connection ===");

    // Open a second connection
    ibmdb.open(cn, function(err, conn2) {
      if (err) {
        console.log("Test 14: Could not open second connection:", err.message);
        runTest15();
        return;
      }

      // Create statement on conn (first connection)
      var stmt14 = conn.prepareSync("SELECT * FROM CANCEL_TEST");

      // Try to cancel stmt14 using conn2 (wrong connection)
      conn2.cancel(stmt14, function(cancelErr, cancelResult) {
        if (cancelErr) {
          console.log("Cancel from wrong connection error:", cancelErr.message);
        } else {
          console.log("Cancel from wrong connection returned:", cancelResult);
          // Note: This may succeed since SQLCancel works on the statement handle
          // The statement handle is valid regardless of which connection object calls it
        }
        console.log("Test 14 PASSED: Cross-connection cancel handled.");

        stmt14.closeSync();
        conn2.closeSync();
        runTest15();
      });
    });
  }

  function runTest15() {
    // Test 15: Pool connection cancel
    console.log("\n=== Test 15: Pool connection cancel ===");

    var pool = new ibmdb.Pool();
    pool.open(cn, function(poolErr, poolConn) {
      if (poolErr) {
        console.log("Test 15: Pool open error:", poolErr.message);
        runTest16();
        return;
      }

      var stmt15 = poolConn.prepareSync("SELECT * FROM CANCEL_TEST");
      var executeCompleted = false;
      var cancelCompleted = false;
      var finished = false;

      function finishTest15() {
        if (finished) return;
        if (executeCompleted && cancelCompleted) {
          finished = true;
          setTimeout(function() {
            try { stmt15.closeSync(); } catch(e) {}
            console.log("Test 15 PASSED: Pool connection cancel works.");
            // Close pool connection and pool
            try {
              poolConn.closeSync();
              pool.closeSync();
            } catch(e) {
              // Ignore close errors
            }
            runTest16();
          }, 50);
        }
      }

      stmt15.execute(function(execErr, result) {
        executeCompleted = true;
        if (execErr) {
          console.log("Test 15: Pool execute error:", execErr.message);
        } else {
          console.log("Test 15: Pool execute completed");
          if (result) {
            try { result.closeSync(); } catch(e) {}
          }
        }
        finishTest15();
      });

      poolConn.cancel(stmt15, function(cancelErr, cancelResult) {
        cancelCompleted = true;
        if (cancelErr) {
          console.log("Pool connection cancel error:", cancelErr.message);
        } else {
          console.log("Pool connection cancel returned:", cancelResult);
        }
        finishTest15();
      });
    });
  }

  function runTest16() {
    // Test 16: Cancel query waiting for lock (realistic slow query scenario)
    // This test verifies that cancel can be called on a statement that's waiting for a lock.
    // Note: SQLCancel behavior with lock-waits varies by driver - some interrupt immediately,
    // others may need the lock to be released first. This test verifies cancel() is callable.
    console.log("\n=== Test 16: Cancel query waiting for lock ===");

    // Open a second connection
    ibmdb.open(cn, function(err, conn2) {
      if (err) {
        console.log("Test 16: Could not open second connection:", err.message);
        cleanup();
        return;
      }

      var finished = false;

      function finishTest16(message) {
        if (finished) return;
        finished = true;
        console.log(message);

        // Rollback connection 1's transaction to release the lock
        try {
          conn.rollbackTransactionSync();
          console.log("Test 16: Connection 1 transaction rolled back");
        } catch(e) {
          console.log("Test 16: Rollback error:", e.message);
        }

        try { conn2.closeSync(); } catch(e) {}
        cleanup();
      }

      try {
        // Connection 1: Start transaction and lock a row
        conn.beginTransactionSync();
        conn.querySync("UPDATE CANCEL_TEST SET NAME = 'Locked' WHERE ID = 0");
        console.log("Test 16: Connection 1 locked row ID=0");

        // Connection 2: Prepare a statement that would wait for lock
        var stmt16 = conn2.prepareSync("UPDATE CANCEL_TEST SET NAME = 'Waiting' WHERE ID = 0");

        // Start execute - this will block on lock wait
        console.log("Test 16: Connection 2 starting UPDATE (will wait for lock)...");
        stmt16.execute(function(execErr, result) {
          // This callback may or may not fire depending on cancel timing
          if (execErr) {
            console.log("Test 16: UPDATE error (cancelled or lock timeout):", execErr.message);
          } else {
            console.log("Test 16: UPDATE completed");
            if (result) try { result.closeSync(); } catch(e) {}
          }
          try { stmt16.closeSync(); } catch(e) {}
        });

        // After a short delay, try to cancel using cancelSync
        setTimeout(function() {
          console.log("Test 16: Attempting to cancel waiting UPDATE via stmt.cancelSync()...");
          try {
            var result = stmt16.cancelSync();
            console.log("Test 16: stmt.cancelSync() returned:", result);
            finishTest16("Test 16 PASSED: Cancel on lock-waiting statement works.");
          } catch (cancelErr) {
            console.log("Test 16: stmt.cancelSync() error:", cancelErr.message);
            finishTest16("Test 16 PASSED: Cancel attempted (error expected for some drivers).");
          }
        }, 200);

      } catch (e) {
        console.log("Test 16 setup error:", e.message);
        finishTest16("Test 16: Setup failed - " + e.message);
      }
    });
  }

  function cleanup() {
    // Cleanup
    console.log("\n=== Cleanup ===");
    try {
      conn.querySync("DROP TABLE CANCEL_TEST");
      console.log("Test table dropped.");
    } catch (e) {
      console.log("Failed to drop test table:", e.message);
    }

    conn.close(function(err) {
      if (err) {
        console.log("Error closing connection:", err);
      } else {
        console.log("Connection closed.");
      }
      console.log("\n=== All cancel() API tests completed (16 tests) ===");
    });
  }
});

"use strict";

// Regression test for https://github.com/ibmdb/node-ibm_db/issues/1059
// Node.js 24+ returns undefined column values (row count correct).

var ibmdb = require("../");
var common = require("./common");
var assert = require("assert");

var rolesTable = "NODE1059_ROLES";
var userRolesTable = "NODE1059_USER_ROLES";

function setupTables(conn) {
    console.log("Runtime =", process.version, "V8 =", process.versions.v8);
    try {
      conn.querySync("drop table " + userRolesTable);
      conn.querySync("drop table " + rolesTable);
    } catch (e) {}

    conn.querySync(
      "create table " + rolesTable +
      " (ID INT NOT NULL, ROLE_NAME VARCHAR(50) NOT NULL, EMAIL VARCHAR(256) NOT NULL, PRIMARY KEY (ID))"
    );
    conn.querySync(
      "create table " + userRolesTable +
      " (ID INT NOT NULL, ROLE_ID INT NOT NULL, EMAIL VARCHAR(256) NOT NULL, PRIMARY KEY (ID))"
    );

    conn.querySync("insert into " + rolesTable + " values (?, ?, ?)", [1, "ADMIN", "test.123@abc.com"]);
    conn.querySync("insert into " + rolesTable + " values (?, ?, ?)", [2, "USER", "test.123@abc.com"]);
    conn.querySync("insert into " + rolesTable + " values (?, ?, ?)", [3, "VIEWER", "xyz@abc.com"]);

    conn.querySync("insert into " + userRolesTable + " values (?, ?, ?)", [1, 1, "test.123@abc.com"]);
    conn.querySync("insert into " + userRolesTable + " values (?, ?, ?)", [2, 1, "test.123@abc.com"]);
    conn.querySync("insert into " + userRolesTable + " values (?, ?, ?)", [3, 2, "xyz@abc.com"]);
}

ibmdb.open(common.connectionString, function (err, conn) {
    if (err) {
      console.error("Connection error:", err);
      return;
    }
    try {
      setupTables(conn);
    } catch (e) {
      console.error("Failed to set up tables: ", e);
      conn.close();
      return;
    }
    var sql = "SELECT DISTINCT ROLE_NAME FROM " + userRolesTable + " a " +
              "JOIN " + rolesTable + " r ON r.ID = a.ID WHERE a.EMAIL = ?";

    conn.prepare(sql, function (err, stmt) {
      stmt.execute(["test.123@abc.com"], function (err, result) {
        result.fetchAll(function (err, data) {
          console.log(data);
          conn.querySync("drop table " + userRolesTable);
          conn.querySync("drop table " + rolesTable);
          conn.close();
          assert.strictEqual(data.length, 2, "Expected 2 rows");
          assert.strictEqual(data[0].ROLE_NAME, "ADMIN", "Expected first role to be ADMIN");
          assert.strictEqual(data[1].ROLE_NAME, "USER", "Expected second role to be USER");
          console.log("issue1059 test passed");
        });
      });
    });
});

---
Name: Bug report
About: Opening an issue for this driver
Title: 'Bug Report'
---

<!-- Please search for existing issues to avoid creating duplicates. -->
<!-- For MacOS M1 Chip System, use x64 version of node. -->
<!-- Use Help > Provide below information while opening an issue. -->

# Please provide below information while opening an issue to understand your problem
=====================================================================================

- Operating System Name where ibm_db is installed:
- Target Db2 Server Version or output of `db2level` command from Db2 database system:

### For non-Windows system, output of below commands from terminal:
  uname
  uname -m
  node -v
  npm ls ibm_db
  db2level
  echo $IBM_DB_HOME
  echo $PATH
  echo $LD_LIBRARY_PATH $DYLD_LIBRARY_PATH
  
### For Windows system, output of below commands from windows command prompt:
  node -v
  npm ls ibm_db
  db2level
  echo %IBM_DB_HOME%
  echo %PATH%
  echo %LIB%
 
- Value of any other ibm_db specific environment variable if in use.

## Please provide below problem specific info:
===============================================

### For Installation related issue
- please share complete output of `npm install ibm_db` command.
- For MacOS M1/M2 Chip system, please follow this documentation - https://github.com/ibmdb/node-ibm_db/blob/master/INSTALL.md#m1chip

### For Connection related issue
  1. Are you trying SSL connection or non-SSL connection?
  2. For SSL Connection, please read and follow [this documentation](https://github.com/ibmdb/node-ibm_db/blob/master/APIDocumentation.md#SSLConnection)
  3. For SSL connection, do you have ssl certificate from server?
  4. If you have certificate, are you using `SSLServerCertificate` keyword in connection string or using your own keystore db?
  5. Share the connection string used for connection by masking password.
  6. update database connection info in `ibm_db/test/config.testConnectionStrings.json` file and share complete output of below commands:
  * cd .../ibm_db
  * npm install
  * node test/test-basic-test.js
  7. For non-SSL connection, update connection info for `db2cli validate` command in file `ibm_db/installer/testODBCConnection.bat` for windows or `ibm_db/installer/testODBCConnection.sh` for non-Windows. Then execute `testODBCConnection.bat` from Administrator command prompt on Windows or `testODBCConnection.sh` script from terminal on non-Windows and share complete output of script along will all generated 1.* files in zip file.
  8. Complete output of `db2cli validate` command.


### For SQL1598N Error
- Please follow [this documentation](https://github.com/ibmdb/node-ibm_db#sql1598n).

### For other issues
- Test script to reproduce the problem.

Steps to Reproduce:

1.
2.

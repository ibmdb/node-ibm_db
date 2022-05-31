---
name: Bug report
about: Opening an issue for this driver
title: 'Bug Report'
labels: ''

---

<!-- Please search for existing issues to avoid creating duplicates. -->
<!-- For MacOS M1 Chip System, use x64 version of node. -->
<!-- Use Help > Provide below information while opening an issue. -->

- Operating System Name:
- db2level output from clidriver if in use:
- Target Db2 Server Version:
- node.js Version:
- ibm_db version:
- For non-Windows, output of below commands:
  uname
  uname -m
- Value of below environment variables if set:
  IBM_DB_HOME:
  PATH:
  LIB/LD_LIBRARY_PATH/DYLD_LIBRARY_PATH:

- Test script to reproduce the problem.
- For installation related issue, complete output of `npm install ibm_db` command.
- For connection related issue, update database connection info in `ibm_db/test/config.testConnectionStrings.json` file and share complete output of below commands:
  * cd .../ibm_db
  * npm install
  * node test/test-basic-test.js

Steps to Reproduce:

1.
2.

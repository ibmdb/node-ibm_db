2024-12-01, Version 3.3.0
=========================

 * add electron binaries for macos arm64 system (Bimal Jha)

 * fea: add support for native installation on MacOS M1 Chip system (Bimal Jha)


2024-11-21, Version 3.2.5
=========================

 * test: add test case for issue #996 (Bimal Jha)

 * update adm-zip version, fix of issue #991 (Bimal Jha)

 * add support for nodev23.x (Bimal Jha)

 * add support for nodev22.x on Windows #1005 (Bimal Jha)

 * Update README.md doc for issue #1004 (Bimal Kumar Jha)

 * Axios upgrade due open vulnerability [CVE-2024-39338] (#1001) (Iván Gustavo Ortiz García)

 * upd: nan version to 2.20.0 (Bimal Jha)

 * fea: add binaries for electron_v31 (Bimal Jha)

 * update nan version (Bimal Jha)

 * update mac binaries for electron 28 (Bimal Jha)

 * fix adm-zip version temporarily to fix vscode install issue (Bimal Jha)

 * Add Linux and Windows binaries for electron version 28 (Bimal Jha)

 * doc: update info about JKS file for SSL Connection (Bimal Jha)


2024-03-31, Version 3.2.4
=========================

 * doc update with SSL info for #989 (Bimal Jha)

 * build(deps): bump follow-redirects from 1.15.4 to 1.15.6 (#990) (dependabot[bot])

 * support for set connect attributes, #970 (Bimal Jha)

 * Fix calling FreeColumns multiple times resulting in SIGABRT (#980) (Lasse Hyldahl Jensen)

 * build(deps): bump follow-redirects from 1.15.3 to 1.15.4 (#976) (dependabot[bot])


2023-11-17, Version 3.2.3
=========================

 * update precompiled binaries (Bimal Jha)

 * fea: add support for pool.initAsync() API, PR #953, issue #952 (Bimal Jha)

 * fix: compilation issue on z/OS, define SQL_BOOLEAN, issue #961 (Bimal Jha)

 * build(deps): bump axios from 1.5.0 to 1.6.0 (#964) (dependabot[bot])

 * fix promise bug of executeNonQuery api, issue #960 (Bimal Jha)

 * read Db2 credential from Env var for testing (Bimal Jha)


2023-09-23, Version 3.2.2
=========================

 * update windows binary and electron binaries (Bimal Jha)

 * update test files, doc files and dependency versions. (Bimal Jha)

 * Fix for https://github.com/ibmdb/node-ibm_db/issues/942 (#944) (Andre Asselin)

 * Fix double free of Buffer/str for OUTPUT parameter, issue #942 (Bimal Jha)

 * Fix for https://github.com/ibmdb/node-ibm_db/issues/943 (#945) (Andre Asselin)

 * Support null value for INOUT type parameter of SP, issue #940 (Bimal Jha)

 * doc upate, correct links in api documentation (Bimal Jha)

 * fix: ignore CLI0106E error for endTransaction, issue #938 (Bimal Jha)

 * update test file (Bimal Jha)

 * add support for outparams in await call of stmt.execute() API, issue #936 (Bimal Jha)

 * Add addon binaries for electron v25 (Bimal Jha)

 * update issue template (Bimal Jha)


2023-05-21, Version 3.2.1
=========================

 * add support for DOWNLOAD_CLIDRIVER environment variable (Bimal Jha)

 * fix: catch unhandled exception for conn.prepare, issue #925 (Bimal Jha)

 * correct db2 lib path for macos electron binary, https://github.com/ibmdb/vscode-extension/issues/61 (Bimal Jha)


2023-05-07, Version 3.2.0
=========================

 * update M1 Chip system installation instruction (Bimal Jha)

 * Replace unzip package to fix Node 18 install error (#922) (Timothy Johnson)

 * update documentation (Bimal Jha)

 * Add MacOS M1/M2 Chip install instructions (Bimal Jha)

 * update windows and vscode binaries (Bimal Jha)

 * add char insert test case for issue #915 (Bimal Jha)

 * update minimist version in package-lock.json file, issue #905 (Bimal Jha)

 * fix for issue #904 - correct typo in promise return (Bimal Jha)

 * fix: use passed length as buffer size for binary array insert, issue #896 (Bimal Jha)

 * fix: for issue #899 - correct hard coded path in mac vscode binary (Bimal Jha)


2022-12-04, Version 3.1.0
=========================

 * update Readme file (Bimal Jha)

 * update: windows binary for nodejs v19.x (Bimal Jha)

 * fix: for binary data array insert issue #883 (Bimal Jha)

 * fix: update nan version to 2.17.0 (Bimal Jha)

 * fea: Add support for getFunctions() APIs (Bimal Jha)

 * build(deps): bump minimatch from 3.0.4 to 3.1.2 (#893) (dependabot[bot])

 * fea: add support for getTypeInfo() APIs (Bimal Jha)

 * Fix error in driverInstaller on z/OS (#884) (jolai)


2022-09-06, Version 3.0.0
=========================

 * fix: update binaries for windows and vscode (Bimal Jha)

 * fix: Reloading driver causes failures on async functions #514 (Bimal Jha)

 * fea: Convert the library to support Promises for all methods. #715 (Bimal Jha)

 * fea: add result.close API (Bimal Jha)

 * promisify describe related methods (Bimal Jha)

 * update mac binaries for vscode https://github.com/ibmdb/vscode-extension/issues/50 (Bimal Jha)

 * test: update test files (Bimal Jha)

 * fix: Empty Strings in Batch inserts result in corrupt values being inserted #875 (Bimal Jha)

 * fea: Add support for Buffer() for insert and select for binary data. #702, #859, #860, #862, #864 (Bimal Jha)

 * fea: allow installation using specific version of clidriver (Bimal Jha)


2022-07-16, Version 2.8.2
=========================

 * update vscode binaries for mac and linux (Bimal Jha)

 * update precompiled binary for windows (Bimal Jha)

 * fix: Data Lost at position 1022 when retrieving from DBCLOB. #858 (Bimal Jha)

 * fix: big int data returns wrong value on windows, issue #816 (Bimal Jha)

 * fix: code scan issues reported by polaris and add polaris.yml file (Bimal Jha)

 * fix: update moment version to fix vulnerability (Bimal Jha)

 * fix installation failure issue when IBM site is down, #865, #869 (Bimal Jha)

 * fix: return bigint as string #816, #863 (Bimal Jha)

 * add template for reporting an issue (Bimal Jha)

 * Added support for npm config SSL CA-files for driverInstall.js (#857) (Patrick Lindemann)

 * test: update test file to test the issue #835 (Bimal Jha)

 * fea: Add support for getData and getDataSync APIs (Bimal Jha)

 * Update axios version to fix vulnerability CVE-2022-1214 (Bimal Jha)

 * build(deps-dev): bump async from 2.6.1 to 3.2.2 (#840) (dependabot[bot])

 * build(deps): bump minimist from 1.2.5 to 1.2.6 (#839) (dependabot[bot])

 * build(deps-dev): bump moment from 2.22.2 to 2.29.2 (#838) (dependabot[bot])

 * add electron binary for macos (Bimal Jha)

 * add windows binary for vscode1.66 (Bimal Jha)

 * Verify IBM_DB_HOME has include and find MSBuild on windows (Bimal Jha)

 * build(deps): bump follow-redirects from 1.14.3 to 1.14.8 (#825) (dependabot[bot])


2021-12-18, Version 2.8.1
=========================

 * add support for node v17.x (Bimal Jha)

 * update nan versio of nodev17.x support (Bimal Jha)

 * update unzipper and big-integer version, issue #795 (Bimal Jha)


2021-09-12, Version 2.8.0
=========================

 * update README.md with latest tested node version (Bimal Jha)

 * update windows binaries (Bimal Jha)

 * return SQL_DESC_NAME in column metadata, #678 (Bimal Jha)

 * build(deps): bump axios from 0.21.1 to 0.21.2 (#793) (dependabot[bot])

 * Electron 13 Support (#786) (Akhil Ravuri)

 * update test files (Bimal Jha)

 * fix decimla val issue for array insert (Bimal Jha)

 * fix for decimal output param in SP, issue #782 (Bimal Jha)

 * update test file for issue #778 (Bimal Jha)

 * update package-lock.json file (Bimal Jha)

 * fix: stale connection issue #742 (Bimal Jha)

 * fea: Add support for arm64 on MacOS #784 (Bimal Jha)

 * fea: add support for synchronous conn pool apis. #692 (Bimal Jha)

 * doc: add env var info for silent installation (Bimal Jha)

 * Fix: getSQLErrorSync docs for warnings (#781) (David Sint)

 * fea: support for silent installation #743 (Bimal Jha)

 * fea: add new debug level to dump sensitive data, issue #738 (Bimal Jha)


2021-06-17, Version 2.7.4
=========================

 * fea: add support for node.js v16.x (Bimal Jha)

 * fix test file issue (Bimal Jha)

 * fix for decimal issue#761 (Bimal Jha)

 * build(deps): bump lodash from 4.17.19 to 4.17.21 (#764) (dependabot[bot])

 * Electron 12 support for vs code. (#766) (Akhil Ravuri)

 * fix bigint issue #750 (Bimal Jha)

 * electeron binaries added (#756) (Akhil Ravuri)


2021-03-15, Version 2.7.3
=========================

 * fix warnings and test failure (Bimal Jha)

 * fix: remove undefined macros for z/OS (Bimal Jha)

 * update windows binary for Electron 11.2.1 (Bimal Jha)


2021-01-20, Version 2.7.2
=========================

 * update windows binaries (Bimal Jha)

 * fix: return decimal, numeric and decfloat columns as number, issue #693 (Bimal Jha)

 * Remove dependency on request package and add axios, issue #642 (Bimal Jha)

 * add test case for Array Insert in Stored Procedure, issue #717 (Bimal Jha)

 * define SQL_DBMS_FUNCTIONLVL if not defined, issue #729 (Bimal Jha)

 * update Readme with v15.4.0 (Bimal Jha)

 * support for node v15.x (Bimal Jha)

 * doc: update for SSL connection (Bimal Jha)

 * doc: update for issue #713 (Bimal Jha)

 * add invalid connections check for prepared statement (#711) (Guilherme G. M)

 * Revert "add invalid connections check for prepared statement (#705)" (#710) (Bimal Kumar Jha)

 * add invalid connections check for prepared statement (#705) (Guilherme G. M)

 * add test for issue #706 (Bimal Jha)

 * update bl package version to fix high severity vulnerability (Bimal Jha)

 * add support for promises in executeFile API (Bimal Jha)

 * Add support for electron 9.2.1 (Bimal Jha)


2020-08-27, Version 2.7.1
=========================

 * doc: update latest test node.js version (Bimal Jha)

 * upd: windows binary files (Bimal Jha)

 * update: PATH and LIB to use GSKit libs of clidriver of ibm_db (Bimal Jha)

 * fea: Allow null value in Array Insert #679 (Bimal Jha)

 * doc: correct info about getInfo API (Bimal Jha)

 * feature: Add support for getInfo() API (Bimal Jha)

 * doc: update db2connect license version to v11.5 (Bimal Jha)

 * fix: for array insert bug #660 (Bimal Jha)

 * build(deps): bump lodash from 4.17.14 to 4.17.19 (#673) (dependabot[bot])

 * upd: test file for issue #647 (Bimal Jha)

 * update README.md for node.js v14.4.0 (Bimal Jha)


2020-06-07, Version 2.7.0
=========================

 * fix vulnerabilities (Bimal Jha)

 * update windows binaries (Bimal Jha)

 * doc update (Bimal Jha)

 * doc update for issue #658 (Bimal Jha)

 * upd: test file for issue #634 (Bimal Jha)

 * doc: update for array insert (Bimal Jha)

 * fix for issue #655 (Bimal Jha)

 * fea: Add support for Array Insert #233, #489 (Bimal Jha)

 * fea: read value of IBM_DB_INSTALLER_URL from .npmrc file (Bimal Jha)

 * support for electron7.1.11 version (#619) (Akhil Ravuri)

 * upd: version of minimist to fix security vulnerability (Bimal Jha)

 * upd: test files for Windows to avoid SQL0420N error (Bimal Jha)


2020-02-20, Version 2.6.4
=========================

 * upd: windows binary (Bimal Jha)

 * upd: test file (Bimal Jha)

 * fix: remove deletion of local var, issue #610 (Bimal Jha)

 * fix: to insert null value using dynamic sql #604 (Bimal Jha)

 * fix: allocate one more byte for output params (Bimal Jha)


2019-12-11, Version 2.6.3
=========================

 * update windows binary using latest code (Bimal Jha)

 * fea: check electron version during installation, issue #598 (Bimal Jha)

 * fix: ignore sqlcode 100 by query APIs, issue #573 (Bimal Jha)

 * fix: for issues #598, #599 (Bimal Jha)

 * test: adding new test case of perf testing (Bimal Jha)


2019-12-09, Version 2.6.2
=========================

 * doc: update for issue #593 (Bimal Jha)

 * fix: update windows binary using latest code (Bimal Jha)

 * fix: for memory leak issue #576 (Bimal Jha)

 * fix: update windows binary for vscode 1.40.x (Bimal Jha)

 * update windows binaries (Bimal Jha)

 * fix: add executeNonQuerySync in odbc.js issue #595 (Bimal Jha)

 * fix: ignore sqlcode 100 by executeNonQuery, issue #591 (Bimal Jha)

 * fix: use v8::Isolate for nodev >= 11 (Bimal Jha)

 * fix: update unzipper version, issue #588 (Bimal Jha)

 * Support for node v12 on z/OS, and fix some test cases for z/OS (#586) (alexcfyung)

 * force push connection to the queue when poolSize breaches maxPool boundry (#581) (ashutoshrnjn)

 * doc: update doc for executeNonQuerySync API, issue #583 (Bimal Jha)

 * fix: blob data corruption issue #582 (Bimal Jha)

 * support for install --debug option on windows (Priyanka Manoharan)

 * doc: Docker Linux Container instructions (Bimal Jha)


2019-09-03, Version 2.6.1
=========================

 * fix: macos install issue (Bimal Jha)

 * doc: update (Bimal Jha)

 * Update windows binary in build.zip file (Priyanka Manoharan)

 * fix: for issue #569 (Bimal Jha)

 * fix: for memleak issue #567 (Bimal Jha)

 * update windows binary in build.zip file (Priyanka Manoharan)

 * Support for ODBCStatement.close() async method (#325) (PriyankaManoharan04)

 * fix: for issue #542 (Bimal Jha)

 * fix: return execute warning before rowCount warning (Bimal Jha)

 * fix: for issue #551 (Bimal Jha)

 * message update (Bimal Jha)

 * update test files (Priyanka Manoharan)

 * fix: for issue #567 (Bimal Jha)

 * fix: for issue #557 (Bimal Jha)


2019-08-19, Version 2.6.0
=========================

 * update windows binary in build.zip file (Priyanka Manoharan)

 * fix: use nan converters (Bimal Jha)

 * fix for issue #563 (Bimal Jha)

 * fix memory leak issue #452 (Bimal Jha)

 * Added section on configuration keywords available (#560) (Dariusz Danielewski)

 * support for executeFile Api (#558) (PriyankaManoharan04)

 * Node 12.x Support (#564) (Bimal Kumar Jha)

 * Add vscode binary for 4.2.5 (Bimal Jha)

 * electron 4.2.5 version is supported (#556) (Akhil Ravuri)


2019-07-19, Version 2.5.3
=========================

 * update windows binary in build.zip file (Priyanka Manoharan)

 * Fix for issue #512 (Priyanka Manoharan)

 * support for fs-extra package (#554) (PriyankaManoharan04)

 * fix security alert for lodash (Bimal Jha)

 * Fix for the issue #541 (#549) (PriyankaManoharan04)

 * support for executeFileSync Api (#547) (PriyankaManoharan04)

 * Fix undefined availablePool issue (Bimal Jha)

 * Fix error when pool is not initialized: DBC-4375 (Bimal Jha)

 * update test file (Bimal Jha)

 * Fix for issue #497, remote stale connection from pool (Bimal Jha)

 * Update fstream to 1.0.12 to fix vulnerability (#539) (Vinícius Silva)

 * Update z/OS documentation (#540) (Joran Siu)

 * Add bindings for Electron f/w (#536) (Vyshakh)

 * Add support for DBCLOB data types. Issue #431 (Priyanka Manoharan)

 * Do not delete build direcotry for z/OS. #530 (Bimal Jha)

 * fix test case issue (Bimal Jha)

 * renaming the vscode extension directory as per new extension name (#529) (Vyshakh)

 * update windows binary in build.zip file. (Priyanka Manoharan)

 * clean build directory before rebuild during installation (Bimal Jha)

 * DBC-4156:Add support for SQLRowCount() API on result object returned by stmt.execute (Bimal Jha)

 * Add support for BOOLEAN data type (Bimal Jha)

 * test case for XML datatype (Priyanka Manoharan)

 * Fix for issue #517 (Priyanka Manoharan)

 * Vscode extension detection during install (#518) (Vyshakh)

 * Enable electron build for use with VS Code (#515) (Vyshakh)


2018-12-05, Version 2.5.0
=========================

 * Update README.md (Bimal Kumar Jha)

 * Adding windows binary support for Node V9.X, V10.X, V11.X (#492) (Rohit Pandey)

 * Fix STEPLIB typos for SDSNLOD2 library (#488) (Joran Siu)

 * Revert "Fix ApiDocumentation.md to use Github Markdown anchors" (#491) (Bimal Kumar Jha)

 * Remove extraneous comma in zos test config (#479) (Joran Siu)

 * Node.js Version 9.X and 10.X support. (#398) (Rohit Pandey)

 * Change links to add user-content- prefix, trim whitespace (#477) (Michael Varrieur)

 * fix test case (Bimal Jha)


2018-09-26, Version 2.4.1
=========================

 * Update dependent package versions (Bimal Jha)

 * doc update and add test file (Bimal Jha)

 * Add API Documentation and document fetchAll #409 (Bimal Jha)

 * adding latest binaries for windows support (Rohit Pandey)

 * Fix security vulnerability of manitest files. (Bimal Jha)

 * V11.1clidriver support (#456) (Bimal Kumar Jha)

 * fix typo (Bimal Jha)

 * Add support for result.getSQLErrorSync() API (Bimal Jha)

 * Bug fixes and #440 (Bimal Jha)

 * Update basic + async call tests for z/OS compatibility (#441) (Joran Siu)

 * Update query() API to return sqlcode and state in third parameter of callback function (Bimal Jha)

 * update test file for issue #415 (Bimal Jha)

 * update unzipper version #417 (Bimal Jha)

 * fix for issue #415 (Bimal Jha)

 * DB2 z/OS installation difficulties noted (#408) (Robert Penny)

 * Fix for issue #375 (Bimal Jha)


2018-06-04, Version 2.4.0
=========================

 * adding latest binaries, support for node 9.X on windows (Rohit Pandey)

 * Fix for issue #405, #404, #389 (Rohit Pandey)

 * Document setIsolationLevel #154 (Bimal Jha)

 * Rename getColumnMetadata to getColumnMetadataSync (Bimal Jha)

 * Add queryResult APIs. #246 (Bimal Jha)

 * Doc change for issue #386 (Bimal Jha)

 * Add debug logs (Bimal Jha)

 * Fix for international characters. #388, #399, #401, #402 (Bimal Jha)

 * Restrict createDbSync/dropDbSync on z/OS (#403) (Joran Siu)

 * Update test file for issue #388 (Bimal Jha)

 * update test file with getColumnMetadata (Bimal Jha)

 * adding latest binaries for windows (Rohit Pandey)

 * Correct formatting issue (Bimal Jha)

 * Fix for issue #389 (Bimal Jha)

 * Accessor for column metadata (#391) (Michael Colavita)


2018-04-19, Version 2.3.1
=========================

 * Updated test case for issue #357 (#380) (Rohit Pandey)

 * adding latest code binaries till node 8.X (Rohit Pandey)

 * Set theme jekyll-theme-cayman (Bimal Kumar Jha)

 * Fix for issue #377 and #342 (Bimal Jha)

 * CreateDatabaseSync() and dropDatabaseSync() api support. (#341) (Rohit Pandey)

 * add logs (Bimal Jha)

 * Add GRANT EXECUTE permissions step for z/OS ODBC (#378) (Joran Siu)

 * Fix for issue #329, pull #330, #368 (Bimal Jha)

 * Update tests for z/OS compatiability (Joran Siu)

 * Add z/OS specific updates (Joran Siu)

 * Add z/OS specific configuration and installation steps (Joran Siu)

 * Update README + INSTALL with z/OS details (Joran Siu)

 * Added Fix for issue #253 #331 (rhtpandeyIN)

 * Fix for issue #324 - Calling conn.close twice causes pool.close not to complete (bimalkjha)

 * fix for issue #323 (bimalkjha)


2017-09-29, Version 2.2.1
=========================

 * enhance db2connect license info - issue #317 (bimalkjha)

 * enhance test file to test issue #318 (bimalkjha)

 * Fix for issue #313 (bimalkjha)

 * correct dependency versions in pakcage.json - fix for issue #315 (bimalkjha)

 * Fix for issue #288 - Stringify Pool Object in debug log. (bimalkjha)


2017-09-18, Version 2.2.0
=========================

 * update windows binary (bimalkjha)

 * Windows: Latest pre-compiled binary support (rhtpandeyIN)

 * Windows: Auto Installation support (Compile and build) (#310) (Rohit Pandey)

 * update readme (bimalkjha)

 * New contribution guidelines for node-ibm_db (rhtpandeyIN)

 * test case changes (bimalkjha)

 * Fix for issue #297 - After DB Connection Failure, Available and Used Pool gets blank but poolSize still has value as max pool size (bimalkjha)

 * Fix for issue #307 (bimalkjha)

 * fix for issue #305 (rhtpandeyIN)

 * Fix for issue #293 (bimalkjha)


2017-07-10, Version 2.1.0
=========================

 * fix installer bug (bimalkjha)

 * fix test case issue (bimalkjha)

 * add test script for windows (unknown)

 * error message console warning format correction (rhtpandeyIN)

 * node v8.x support for windows platform with the latest code changes (rhtpandeyIN)

 * clidriver permission issue fix. (#284) (Rohit Pandey)

 * inserted missing escape sequence. (#279) (Rohit Pandey)

 * unzipper finish console message change (Rohit Pandey)

 * issue related to clidriver permission and bindings (rhtpandeyIN)

 * README update for clidriver downloading information #255 (#266) (Rohit Pandey)

 * Fix and Test case for issue #253 (#261) (Rohit Pandey)

 * IBM_DB_INSTALLER_URL detailed information updated- issue #255 (rhtpandeyIN)

 * For issue #256 (bimalkjha)

 * Fix for issue #249 - Invalid cursor state (bimalkjha)

 * Fix for issue #238 (bimalkjha)

 * For issue #230 (bimalkjha)

 * fix test case issues and error handling (bimalkjha)

 * Fix for issue #231 (bimalkjha)

 * remove downloaded clidriver zip file on windows (bimalkjha)

 * fix for issue #229 (bimalkjha)

 * doc updated (bimalkjha)


2017-02-22, Version 2.0.0
=========================

 * fix warning and installer update (bimalkjha)

 * latest node binaries for windows and nodejs version < 0.12.x support for node-ibm-db discontinued. (#228) (Rohit Pandey)

 * For issue #227 (bimalkjha)

 * Add support for i5/OS system naming (bimalkjha)

 * For issue #205 (bimalkjha)

 * Fix for issue #222, downloading driver using "request" module, also Code refactoring in "installer.js" (#226) (Rohit Pandey)

 * Delete dead dynodbc code. (#225) (Ben Noordhuis)

 * fix for connection timeout (bimalkjha)

 * fix for issue #210 (bimalkjha)

 * log enhancement (bimalkjha)

 * update test files (bimalkjha)

 * version update (bimalkjha)

 * test file update (bimalkjha)

 * Rebase (Quentin Presley)

 * typeof NULL is object causing tests to fail (Quentin Presley)

 * udpate test files (bimalkjha)

 * update test file (bimalkjha)

 * update test file for issue #215 (bimalkjha)

 * update about node.js v0.10.x (bimalkjha)

 * adding node V7 support for windows (rhtpandeyIN)

 * Remove unnecessary locking (Quentin Presley)

 * fix for issue #211 (bimalkjha)

 * Add support for OUTPUT parameters of SP in execute() and executeSync() APIs. (bimalkjha)

 * adding unzipper support removing unzip (rhtpandeyIN)

 * add check for dynamically allocated memory (bimalkjha)

 * fix for issue #208 (bimalkjha)

 * fix for issue #207 (bimalkjha)

 * add support for multiple resultset returned by query and querySync methods. (bimalkjha)

 * fix for issue #200 (bimalkjha)

 * fix for #198 (bimalkjha)

 * doc update (bimalkjha)

 * remove stale code related to loginTimeout (bimalkjha)

 * Fix for issue #196 (bimalkjha)

 * fix for issue #192 (bimalkjha)

 * add example for single row fetch at once (bimalkjha)

 * add support for Object in query APIs (bimalkjha)

 * adding CLA contribution licence agreement. (rhtpandeyIN)

 * Add stream support and address pull request #184, #189 and #190. (bimalkjha)

 * update windows binary (bimalkjha)

 * connection pool related changes (bimalkjha)

 * doc update with new pool apis (bimalkjha)

 * Add setIsolationLevel function (Quentin Presley)

 * Code comments (Quentin Presley)

 * fix  for pull request #183 (bimalkjha)

 * Fix up no results code (Quentin Presley)

 * Connectio0n Pooling Enhancement to support MaxPoolSize (bimalkjha)

 * fix test-querySync-select-with-execption.js on windows (George Adams)

 * force test-call-stmt and test-call-async to use the default schema (George Adams)

 * Show stdout and stderr when a test fails (Gibson Fahnestock)

 * update version (bimalkjha)

 * call stmt (bimalkjha)

 * update doc (bimalkjha)

 * update doc with bind info (bimalkjha)

 * test file merge (bimalkjha)

 * Fix for issue #159 (bimalkjha)

 * added try-catch for droping table (bimalkjha)

 * merge pull request #157 (bimalkjha)

 * test files to test OUT and INOUT params in SP (bimalkjha)

 * code changes to supprort OUT param in SP (bimalkjha)

 * used targz instead of tar.gz (Jeremiah Akpotohwo)

 * installed dependencies for ibm_db tests (Jeremiah Akpotohwo)

 * Downgraded tar.gz module to 1.0.2 (Akpotohwo)

 * add support for OUT and INPUT Param in Stored Procedure (bimalkjha)

 * Update white space formatting (Quentin Presley)

 * Added 2 test cases and fixed a bug related to promises (matpasha)

 * Reverting test case (matpasha)

 * added promise support to all methods (matpasha)

 * sample web app using ibm_db (bimalkjha)

 * add logs (bimalkjha)

 * for issue #141 (bimalkjha)

 * begining to add support for promises (IBM_ADMIN)


2016-07-08, Version 1.0.0
=========================

 * Added node V6.X and BLOB/CLOB Support for Windows platform (rhtpandeyIN)

 * fix for issue #139 (bimalkjha)

 * fix for issue #120 (bimalkjha)

 * fix for issue #131 (bimalkjha)

 * query api updated to handle LOB data (bimalkjha)

 * updating README for new APIs (rhtpandeyIN)

 * updating README for API information (rhtpandeyIN)

 * Fix for issue #120 and issue #132 (bimalkjha)

 * fix for segfault (bimalkjha)

 * Added V6 Support and LOB Support (rhtpandeyIN)

 * remove node.js v0.8 support (bimalkjha)

 * update for BLOB supprot (bimalkjha)

 * fix for #119 (bimalkjha)

 * udpated test files (bimalkjha)

 * Fix issues related to v0.10.x (bimalkjha)

 * Add BLOB/CLOB Support (bimalkjha)

 * Added BLOB/CLOB Support (bimalkjha)

 * add build steps (bimalkjha)

 * correct examples (bimalkjha)

 * udpate readme file (bimalkjha)

 * add support for proxy authentication using auth (bimalkjha)

 * update test script for MacOS (bimalkjha)

 * commit (rhtpandeyIN)

 * updating link for INSTALL.md (rhtpandeyIN)

 * updating readme (rhtpandeyIN)

 * updating readme and install.md (rhtpandeyIN)

 * updating INSTALL.md (rhtpandeyIN)

 * Adding all installation steps for INSTALL.md (rhtpandeyIN)

 * added steps for Linux for Power PC (rhtpandeyIN)

 * adding space b/w headings INSTALL.md (rhtpandeyIN)

 * added steps for linux on z in INSTALL.md (rhtpandeyIN)

 * modified AIX steps in INSTALL.md (rhtpandeyIN)

 * modified INSTALL.md (rhtpandeyIN)

 * added steps for AIX in INSTALL.md (rhtpandeyIN)

 * updating INSTALL.md file text format (rhtpandeyIN)

 * updating INSTALL.md file (rhtpandeyIN)

 * test commit (rhtpandeyIN)

 * old commit push for INSTALL.md (rhtpandeyIN)

 * updated INSTALL.md file (rhtpandeyIN)

 * adding INSTALL.md for installation guide (rhtpandeyIN)

 * Fix for issue #93, included code from pull request #94 (rhtpandeyIN)

 * adding support for v5 on windows (rhtpandeyIN)

 * Fix for issue #100, included code from pull request #101 (bimalkjha)

 * fix for compile issue on node.js v0.10.x (bimalkjha)

 * update v4 related comment (bimalkjha)

 * Fix for linux compile issue (bimalkjha)

 * Code changes to support node.js v4.x (bimalkjha)

 * fix for issue #81 (bimalkjha)

 * update doc with AIX install issue (bimalkjha)

 * update about v4 support on windows (bimalkjha)

 * new version with AIX support (bimalkjha)

 * Update installer to support V4 binary on ntx64 (bimalkjha)

 * Including windows binary for v4 (rhtpandeyIN)

 * Adding AIX support for node.js v0.12.x (bimalkjha)

 * information updated (Rohit Pandey)

 * Update: node-ibm_db is now supported on Windows using NodeJS V4.x. (Rohit Pandey)

 * adding latest odbc_bindings.node file (unknown)

 * add debug method (Bimal Kumar Jha)

 * Fix for error preventing compilation without UNICODE support (Bimal Kumar Jha)

 * correct the spelling of platform in installer (Bimal Jha)

 * Replace usage of `timelocal` with `mktime` (Matt Hamann)

 * Adding support for AIX platform. (Bimal Jha)

 * removing v4.x related code form master (Bimal Jha)

 * Updating license text (Bimal Jha)

 * Updated windows limitation (Bimal Jha)

 * update nodejs v4.x info (Bimal Jha)

 * correct formatting (Bimal Jha)

 * add nodejs v4.x info (Bimal Jha)

 * fix code issue (Bimal Jha)

 * fixed code issues (Bimal Jha)

 * fixed nan releated code issue (Bimal Jha)

 * update files for nodejs-v4.1.1 support (Bimal Jha)

 * added supported platforms name (Bimal Jha)

 * Add support for LinuxPPC64LE platfrom (Bimal Jha)

 * fix a js bug, call console.log (Bimal Jha)

 * New binary to reflect cpp code changes. (unknown)

 * update test files. (Bimal Jha)

 * Fix for issue #38 (Bimal Jha)

 * update test files (Bimal Jha)

 * Regroup nodeEE test files. (Bimal Jha)

 * Added support for Linux on z Systems (Bimal Jha)

 * Fix for issue #60 (Bimal Jha)

 * code fix (elkorep)

 * Additional Test Cases (elkorep)

 * Fix for issue #55 (Bimal Jha)

 * Use the fixed versions of dependent modules (Regression Account)

 * Fixes [#50](https://github.com/ibmdb/node-ibm_db/issues/50) (Matt Pelland)

 * fix for issue #45 (Bimal Jha)

 * Revert "0.0.12 fails to install on IBM Bluemix #43" (bimalkjha)

 * 0.0.12 fails to install on IBM Bluemix #43 (Matt Pelland)

 * Fix for issue #42 (Bimal Jha)

 * fix installer issue for windows (bimaljha)

 * Update build.zip and defautl connection timeout. (Bimal Jha)

 * Exit with error code on installation errors, to allow failure detection by automated build tools (Michael Szlapa)

 * Add pool related test files. (Bimal Jha)

 * Update conn.close() for Pool to avoid reconnection before moving to available pool. Add pool related test files and update existing test files. (Bimal Jha)

 * update examples (Bimal Jha)

 * Updating the new build.zip for windows (Bimal Jha)

 * delete build.zip file on ntx64 after unzip (bimaljha)

 * add build.zip (Bimal Jha)

 * add build.zip for windows, add ibm_db.close method to remove members of Database(), update package.jso with new version for release. (Bimal Jha)

 * update package.json that node version should be less than v0.12.0 (Bimal Jha)

 * fix for issue #35 and #36 (Bimal Jha)

 * note about 0.12.x support (ibmdb)

 * Adding Mac OS, other platform support (Avinash K)

 * Fixed a few typos of 'funtion'->'function' (Christopher Bebry)

 * Removing build.zip (Avinash K)

 * formatting macos section (ibmdb)

 * macos (mariobriggs)

 * Intermediate_Fixes branch merge (Avinash K)

 * Fix for pipe stream issue while downloading DSDriver zip file (Avinash K)

 * Fixed error in lib path set in binding.gyp for DB2 installation (Avinash K)

 * Removing unnecessary print statements (Avinash K)

 * Code review changes, removed unnecessary conditionals from binding.gyp (Avinash K)

 * Code review fixes, test-case  clean up (Avinash K)

 * Documenatation changes, Changes after code review (PriyaRanjan)

 * driver install automation on linux, test cases (PriyaRanjan)

 * Automated driver installation for linux (PriyaRanjan)

 * Driver installation automated on linux (PriyaRanjan)

 * Automated linux drivier installation (PriyaRanjan)

 * Commit windows build binary (avinashk)

 * fixes for issue #4, #14, #17 (PriyaRanjan)

 * Update document to note requirements can use a DB2 Server, not just Data Server Driver. (Ian Bjorhovde)

 * Modify to add support for building on Mac OS X Add comments about possibility of using a DB2 Server (e.g., DB2 Express-C) instead of the Data Server Driver only. (Ian Bjorhovde)


2014-10-30, Version -0.0.3
==========================

 * fixes linux new line char (PriyaRanjan)


2014-10-22, Version -0.0.2
==========================

 * test-suite clean up for DB2, fixes for issue #14, bug while retrieving time data type (avinashk)


2014-01-24, Version -0.0.1
==========================

 * First release!

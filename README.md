# node-ibm_db

An asynchronous/synchronous interface for node.js to IBM DB2 and IBM Informix.
Async APIs return promises if callback function is not used. Async APIs supports async-await programming style.

- **Supported Platforms** - Windows64, MacOS64, MacARM64, Linuxx64, Linuxia32, AIX, Linux on IBM Z, Linux on Power PC and z/OS.

- **MacOS with Silicon Chip** - Supported from ibm_db@3.3.0 onwards using v12.x clidriver.

- **MacOS with Intel Chip** - Supported using v11.x clidriver only. By default v11.5.9 clidriver will get downloaded.

- **Electron Binaries** - From electron v34 to v38. Hence, no support for vscode older than v1.98.

- ibm_db v3.2.5 is the last version to use clidriver of version 11.5.9 by default. **ibm_db@3.3.0** onwards v12.1.x clidriver get autodownloaded by default and hence v12.1 db2connect license is required. However, user can use older db2connect license when ibm_db get installed using `npm install ibm_db -clidriver="v11.5.9"` command.

- **SQL1598N Error** - It is expected in absence of valid db2connect license. Please click [here](#sql1598n) and read instructions.

- **GSKit Error** - Check instructions as documented [here](https://github.com/ibmdb/node-ibm_db/blob/master/APIDocumentation.md#SSLConnection).

## API Documentation

- For complete list of ibm_db APIs and example, please check [APIDocumentation.md](https://github.com/ibmdb/node-ibm_db/blob/master/APIDocumentation.md)

- For Secure Database connection using **SSL/TSL**: Check [here](https://github.com/ibmdb/node-ibm_db/blob/master/APIDocumentation.md#SSLConnection).

- Got an issue? Need Help? : Check common issues and suggestion [here](#for-aix-install-issue).

## Prerequisite

- Make sure your system has C++ compiler installed that support C++11 standard.

- **For Linuxx64, MacARM64, Mac-x64 and Windows-x64 platforms**: If a compiler or required dependency is missing, ibm_db automatically falls back to a precompiled binary (for Node.js v16.x and above), ensuring successful installation without native compilation. Note that compilation of native code is attempted first.

- Below prerequisite is applicable only if you need to compile the native C++ code of ibm_db or download of precompiled binary fail.

- **For non-windows platforms**: gcc compiler version >= 8.4, python3, nodejs and `make` is required to install `ibm_db`. Default compiler on RHEL 6 does not have the required support. Install a newer compiler or upgrade older one.

- Other required system libraries for Linux OS are: `libcrypt, libpam, libxml2, libstdc++, libc, libgcc, libaudit, libeconf, libz, liblzma and libcap-ng`.
  These libraries should come by default with OS. If it is not present in your system, please install it.

- UBI9 has deprecated libcrypt. You need to install libcrypt and libxcrypt-compat on UBI9 manually.

- **For Windows**: compiler is optional as `ibm_db` comes with pre-compiled binary on Windows64 for node.js version >= 14.x. To compile code on Windows, VC++ 2015.3 v14.00 (v140) or Visual Studio version >= 2017 is required.

- Python version >= 3.8.0 is required by node-gyp.

- **For Docker Linux Container:** make sure you have installed **make, gcc, g++(gcc-c++), python3.9 and node** before installing `ibm_db`. For `root` user, use `npm install --unsafe-perm ibm_db` to install `ibm_db`.

- While installing `ibm_db` under a container, if you get libcrypt error, add a line in your scrip to install `libcrypt` and `libxcrypt-compat` as this library is not present in most of the container OS. f.e. add a line like below in docker script:
  `RUN apt-get update && apt-get install gcc g++ libcrypt libxcrypt-compat python3 make -y`

- **For Windows Subsystem for Linux (WSL):** Install `build-essentials` package before installing `ibm_db`.

- **For MacOS:** Install XCode from appstore before installing `ibm_db`. Also, gcc@12 and `make` is required.

- On distributed platforms, you do need not to install any Db2 ODBC client driver for connectivity. `ibm_db` itself downloads and installs an odbc/cli driver from IBM website during installation. Just install `ibm_db` and it is ready for use.

- On **z/OS**, ODBC driver support is part of IBM Db2 for z/OS 11.0 and 12.0. Please ensure IBM Db2 for z/OS 11.0 or 12.0 is installed on your given LPAR. Ensure you follow the instructions to configure your ODBC driver [here](#configure-odbc-driver-on-zos).

- On **z/OS** and other non-Windows platform, `GNU make` is required to install `ibm_db`. Execute `make -v` command before installing `ibm_db` to make sure you have correct `make` set in PATH.

- On **z/OS** only certain versions of node-gyp are supported. This was tested with:<br>
  z/OS v2.4
  Db2 v12.0
  node-gyp 3.4.0<br>
  npm 3.10.10<br>
  ibm_db: 2.8.1

- Recommended version of node.js is >= V16.X. For node.js version < 16.X and `ibm_db` version > 2.4.1, Visual Studio is required to install `ibm_db` on Windows.

- The latest node.js version using which `ibm_db` is tested: **24.9.0**

## Install

You may install the package using npm install command:

```
npm install ibm_db
```

You may install `ibm_db` in `quiet` mode using either of below commands:

```
npm install --quiet ibm_db
npm install -q ibm_db
```

You may install `ibm_db` in `silent` mode using either of below commands:

```
npm install --silent ibm_db
npm install -s ibm_db
```

When using `ibm_db` in a package.json file, you can set below environment variables to install `ibm_db` in `--quiet` or `--silent` mode:

```
export npm_config_loglevel=warn   => For quiet mode installation.
export npm_config_loglevel=silent => For silent mode installation.
```

For **ELECTRON** or **VSCode** Installation:

```
npm install ibm_db -electron=<electron_version>
npm install ibm_db -electron="35.5.1"
npm install ibm_db -vscode
ELECTRON=37.1.0 npm install ibm_db
```

You can also set environment variable `ELECTRON` to instruct ibm_db to install it for that version of electron for electron applications like vscode.

```
export ELECTRON=37.1.0
npm install ibm_db
```

To install using **specific version of clidriver** from https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/:

```
npm install ibm_db -clidriver=<version>
npm install ibm_db -clidriver=v11.1.4
npm install ibm_db -clidriver=v11.5.6
```

You can also set system level environment variable `CLIDRIVER_DOWNLOAD_VERSION` to instruct ibm_db to download that version of clidriver.

```
export CLIDRIVER_DOWNLOAD_VERSION=v11.5.9
npm install ibm_db
```

You can set the version of "clidriver", "electron", IBM_DB_DOWNLOAD_URL or "cafile" in config section of your package.json file.
In that case, no need to pass these options with `npm install` command or no need to set in .npmrc to avoid warning message with npm v11.x

```
package.json:
{
  "name": "foo",
  "config": {
    "clidriver": "v11.5.9",
    "electron": "36.4.0"
  }
}
```

For **Docker Linux Container**, use below commands:

```
yum install make gcc gcc-c++ kernel-devel openssl-devel bzip2-devel
install python3.x
install node.js
npm install --unsafe-perm ibm_db
```

**Alpine Linux** is not supported by `ibm_db` as it is an arm64 architecture.

- `npm install ibm_db` internally downloads and install platform specific clidriver of recent release from [here](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/).
  To avoid this download, you can manually download clidriver from this location or install any verison of IBM Data Server Driver Package or Db2 Client or Sever in your system and point the install directory using `IBM_DB_HOME` environment variable. If `IBM_DB_HOME` or `IBM_DB_INSTALLER_URL` is set, `npm install ibm_db` do not download clidriver.

- `ibm_db` works with all supported versions of Db2 Client and Server. Instead of using open source driver specific [clidriver](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/) for `ibm_db`, you may download and install DSDRIVER or DB2Client from [IBM Fix Central](https://www.ibm.com/support/fixcentral/) or [IBM Passport Advantage](https://www.ibm.com/support/pages/what-passport-advantage-and-how-do-i-access-it) of Db2 V11.1.0.0 onwards.

- If `IBM_DB_HOME` or `IBM_DB_INSTALLER_URL` is not set, `ibm_db` always downloads [open source driver specific clidriver](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/) and use it. Ignores any other installation.

> For more installation details please refer: [INSTALLATION GUIDE](https://github.com/ibmdb/node-ibm_db/blob/master/INSTALL.md)

> If installation fails while downloading [clidriver](#downloadCli), follow instructions as documented [here](#downloadCliFailed).

### Validate your installation

- Update database connection info in `node_modules\ibm_db\test\config.json` file.
- Run `node node_modules\ibm_db\test\test-basic-test.js` command.

### Important Environment Variables and Download Essentials

`IBM_DB_HOME :`

- USE:

  - On distributed platforms, set this environment variable if you want to avoid downloading of clidriver from the [IBM Hosted URL](#downloadCli) or from the internet.
  - On z/OS, set this environment variable to the High Level Qualifier (HLQ) of your Db2 datasets. During `npm install`, the module will automatically reference ODBC driver header files under: `$IBM_DB_HOME.SDSNC.H` and the sidedeck definitions in `$IBM_DB_HOME.SDSNMACS(DSNAO64C)` to build the node binding.

- HOW:
  - On distributed platforms, set **IBM_DB_HOME** environment variable to a pre-installed **db2 client or server installation directory**.
  - On z/OS, set **IBM_DB_HOME** environment variable to the High Level Qualifier (HLQ) of your Db2 datasets. For example, if your Db2 datasets are located as `DSNC10.SDSNC.H` and `DSNC10.SDSNMACS`, you need to set `IBM_DB_HOME` environment variable to `DSNC10` with the following statement (can be saved in `~/.profile`):

`IBM_DB_INSTALLER_URL :`

- USE:

  - Set this environment variable to by-pass the IBM Hosted URL for downloading odbc/clidriver.

- HOW:

  - Set **IBM_DB_INSTALLER_URL** environment variable with alternate odbc/clidriver downloading URL link or with locally downloaded "tar/zipped clidriver's parent directory path.

- TIP:
  - If you don't have alternate hosting URL then, you can download the tar/zipped file of clidriver from the [IBM Hosted URL](#downloadCli) and can set the **IBM_DB_INSTALLER_URL** environment variable to the downloaded "tar/zipped clidriver's" parent directory path. No need to untar/unzip the clidriver and do not change the name of downloaded file.

`DOWNLOAD_CLIDRIVER :`

- USE:

  - Set this environment variable to force downloading of odbc/clidriver. If clidriver is already present, existing clidriver will get deleted. If IBM_DB_HOME environment variable is set, still clidriver will get downloaded by ignoring value of IBM_DB_HOME.

- HOW:
  - On distributed platforms, Set **DOWNLOAD_CLIDRIVER** environment variable to `true` to force downloading of clidriver from IBM hosted URL. Never set DOWNLOAD_CLIDRIVER on z/OS system.

`CLIDRIVER_DOWNLOAD_VERSION :`

- USE:

  - Set this environment variable for downloading of specific version of odbc/clidriver at install time if clidriver do not exist under `ibm_db/installer` directory or not pointed by environment variable IBM_DB_HOME. If clidriver is already present, existing clidriver will not get deleted unless `DOWNLOAD_CLIDIVER` is set.

- HOW:
  - On distributed platforms, Set **DOWNLOAD_CLIDRIVER** environment variable to `true` and set environment variable **CLIDRIVER_DOWNLOAD_VERSION** to force downloading of clidriver from IBM hosted URL. Never set CLIDRIVER_DOWNLOAD_VERSION on z/OS system.

### <a name="downloadCli"></a> Download clidriver ([based on your platform & architecture](#systemDetails)) from the below IBM Hosted URL:

> [DOWNLOAD CLI DRIVER](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/)

#### <a name="systemDetails"></a> Cli Drivers for Specific Platform and Architecture

| Platform | Architecture |                   Cli Driver                    |           Supported            |
| :------: | :----------: | :---------------------------------------------: | :----------------------------: |
|   AIX    |     ppc      |              aix32_odbc_cli.tar.gz              |              Yes               |
|          |    others    |              aix64_odbc_cli.tar.gz              |              Yes               |
|  Darwin  |     x64      |             macos64_odbc_cli.tar.gz             |          Till v11.5.9          |
|  Darwin  |    arm64     |            macarm64_odbc_cli.tar.gz             |          From v12.1.0          |
|  Linux   |     x64      |            linuxx64_odbc_cli.tar.gz             |              Yes               |
|          |    s390x     |             s390x64_odbc_cli.tar.gz             |              Yes               |
|          |     s390     |              s390_odbc_cli.tar.gz               |              Yes               |
|          |  ppc64 (LE)  |             ppc64le_odbc_cli.tar.gz             |              Yes               |
|          |    ppc64     |              ppc64_odbc_cli.tar.gz              |              Yes               |
|          |    ppc32     |              ppc32_odbc_cli.tar.gz              |              Yes               |
|          |    others    |            linuxia32_odbc_cli.tar.gz            |              Yes               |
| Windows  |     x64      |               ntx64_odbc_cli.zip                |              Yes               |
|          |     x32      |                nt32_odbc_cli.zip                | Not supported with node-ibm_db |
|   z/OS   |    s390x     | ODBC support from IBM Db2 for z/OS 11.0 or 12.0 |              Yes               |

### Configure ODBC driver on z/OS

Please refer to the [ODBC Guide and References](https://www.ibm.com/support/knowledgecenter/SSEPEK/pdf/db2z_12_odbcbook.pdf) cookbook for how to configure your ODBC driver. Specifically, you need to:

1.  Apply Db2 on z/OS PTF [UI60551](https://www-01.ibm.com/support/docview.wss?uid=swg1PH05953) to pick up new ODBC functionality to support Node.js applications.

2.  Bind the ODBC packages. A sample JCL is provided in the `SDSNSAMP` dataset in member `DSNTIJCL`. Customize the JCL with specifics to your system.

3.  Ensure users that should be authorized have authority to execute the DSNACLI plan. Included are samples granting authority to public (all users), or specific groups via SQL GRANT statements, or alternately via RACF. The security administrator can use these samples as a model and customize/translate to your installation security standards as appropriate.

    **Examples using SQL GRANT statement**:

    _Example 1:_ Grant the privilege to execute plan DSNACLI to RACF group, DBCLIGRP.

        GRANT EXECUTE ON PLAN DSNACLI TO DBCLIGRP;

    _Example 2:_ Grant the privilege to execute plan DSNACLI to all users at the current server.

        GRANT EXECUTE ON PLAN DSNACLI TO PUBLIC;

    **Examples using Access Control Authorization Exit for Db2 authorization**:

    Define profile for plan DSNACLI execute privilege check

        RDEFINE MDSNPN DB2A.DSNACLI.EXECUTE UACC(NONE) OWNER(DB2OWNER)

    _Example 1:_ PERMIT the privilege to execute plan DSNACLI to RACF group, DBCLIGRP

        PERMIT DB2A.DSNACLI.EXECUTE ID(DBCLIGRP) ACCESS(READ) CLASS(MDSNPN)

    _Example 2:_ PERMIT the privilege to execute plan DSNACLI to all users at the current server

        PERMIT DB2A.DSNACLI.EXECUTE ID(*) ACCESS(READ) CLASS(MDSNPN)

    Issue SETROPTS command to refresh in-storage profile lists

         SETR RACLIST(MDSNPN) REFRESH

4.  Update the `STEPLIB` environment variable to include the Db2 SDSNEXIT, SDSNLOAD and SDSNLOD2 data sets. You can set the `STEPLIB `environment variable in your `.profile` with the following statement, after defining `IBM_DB_HOME` to the high level qualifier of your Db2 datasets as instructed above:

    ```sh
    # Assumes IBM_DB_HOME specifies the HLQ of the Db2 datasets.
    export STEPLIB=$STEPLIB:$IBM_DB_HOME.SDSNEXIT:$IBM_DB_HOME.SDSNLOAD:$IBM_DB_HOME.SDSNLOD2
    ```

5.  Configure an appropriate _Db2 ODBC initialization file_ that can be read at application time. You can specify the file by using either a DSNAOINI data definition statement or by defining a `DSNAOINI` z/OS UNIX environment variable. For compatibility with `ibm_db`, the following properties must be set:

    In COMMON section:

    ```
    MULTICONTEXT=2
    CURRENTAPPENSCH=ASCII
    FLOAT=IEEE
    ```

    In SUBSYSTEM section:

    ```
    MVSATTACHTYPE=RRSAF
    ```

    Here is a sample of a complete initialization file:

    ```
    ; This is a comment line...
    ; Example COMMON stanza
    [COMMON]
    MVSDEFAULTSSID=VC1A
    CONNECTTYPE=1
    MULTICONTEXT=2
    CURRENTAPPENSCH=ASCII
    FLOAT=IEEE
    ; Example SUBSYSTEM stanza for VC1A subsystem
    [VC1A]
    MVSATTACHTYPE=RRSAF
    PLANNAME=DSNACLI
    ; Example DATA SOURCE stanza for STLEC1 data source
    [STLEC1]
    AUTOCOMMIT=1
    CURSORHOLD=1
    ```

6.  Please make sure tagging of your odbc.ini file is "binary" or "mixed":

    ```
    chtag -b $DSNAOINI
    or
    chtag -m -c IBM-1047 $DSNAOINI
    ```

    If file is tagged text (chtag -t -c IBM1047 $DSNAOINI) the S0C4 abend occurs.

    Reference Chapter 3 in the [ODBC Guide and References](https://www.ibm.com/support/knowledgecenter/SSEPEK/pdf/db2z_12_odbcbook.pdf) for more instructions.

## How to get ibm_db instance?

The simple api is based on the instances of `Database` class. You may get an
instance by one of the following ways:

```javascript
require("ibm_db").open(connectionString, function (err, conn) {
  //conn is already open now if err is falsy
});
```

or by using the helper function:

```javascript
var ibmdb = require("ibm_db")();
```

or by creating an instance with the constructor function:

```javascript
var Database = require("ibm_db").Database,
  ibmdb = new Database();
```

## Quick Example

```javascript
var ibmdb = require("ibm_db");
var connStr =
  "DATABASE=<dbname>;HOSTNAME=<myhost>;UID=db2user;PWD=password;PORT=<dbport>;PROTOCOL=TCPIP";

ibmdb.open(connStr, function (err, conn) {
  if (err) return console.log(err);

  conn.query("select 1 from sysibm.sysdummy1", function (err, data) {
    if (err) console.log(err);
    else console.log(data);

    conn.close(function () {
      console.log("done");
    });
  });
});
```

```javascript
ibmdb.open(connStr).then(
  (conn) => {
    conn.query("select 1 from sysibm.sysdummy1").then(
      (data) => {
        console.log(data);
        conn.closeSync();
      },
      (err) => {
        console.log(err);
      }
    );
  },
  (err) => {
    console.log(err);
  }
);
```

```javascript
main();
async function main() {
  try {
    let conn = await ibmdb.open(cn);
    await conn.query("drop table mytab").catch((e) => {
      console.log(e);
    });
    await conn.query("create table mytab(c1 int, c2 varchar(10))");
    await conn.query("insert into mytab values (?, ?)", [3, "ibm"]);
    let stmt = await conn.prepare("select * from mytab");
    let result = await stmt.execute();
    data = await result.fetchAll();
    console.log("result = ", data);
    await result.close();
    await stmt.close();
    await conn.close();
  } catch (e) {
    console.log(e);
  }
}
```

## Un-Install

```
npm uninstall ibm_db
```

OR, just delete the `node_modules\ibm_db` directory manually from your system.

## <a name="sql1598n"></a>For z/OS and iSeries Connectivity and SQL1598N error

- Connection to `Db2 for z/OS` or `Db2 for i`(AS400) Server using `ibm_db` driver from distributed platforms (Linux, Unix, Windows and MacOS) is not free.

- Connection to `Db2 for LUW` or `Informix` Server using `ibm_db` driver is free.

- `ibm_db` returns SQL1598N error in absence of a valid db2connect license. SQL1598N error is returned by the Db2 Server to client.
  To suppress this error, Db2 server must be activated with db2connectactivate utility OR a client side db2connect license file must exist.

- Db2connect license can be applied on database server or client side. A **db2connect license of version 12.1** is required for ibm_db.

- If you have db2connect license of **Db2 V11.5**, then install ibm_db using `npm install ibm_db -clidriver=v11.5.9` command or use below commands:

```
npm config set clidriver=v11.5.9
npm install ibm_db
```

- Ask your DBA to run db2connectactivate utility on Server to activate db2connect license.

- If database Server is enabled for db2connect, no need to apply client side db2connect license.

- If Db2 Server is not db2connectactivated to accept unlimited number of client connection, you must need to apply client side db2connect license.

- db2connectactivate utility and client side db2connect license both comes together from IBM in a single zip file.

- Client side db2connect license is a `db2con*.lic` file that must be copied under `clidriver\license` directory.

- User running application must have write permission for `clidriver\cfgcache` and `clidriver\license` directories as clidriver need to create binary files to store licensing info in these directories at runtime. Lack of permission to create file too causes SQL1598N error.

- If you have a `db2jcc_license_cisuz.jar` file, it will not work for ibm_db. `db2jcc_license_cisuz.jar` is a db2connect license file for Java Driver. For non-Java Driver, client side db2connect license comes as a file name `db2con*.lic`.

- If environment variable `IBM_DB_HOME` or `IBM_DB_INSTALLER_URL` is not set, `ibm_db` automatically downloads [open source driver specific clidriver](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/) from https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli and save as `node_modules\ibm_db\installer\clidriver`. Ignores any other installation.

- If `IBM_DB_HOME` or `IBM_DB_INSTALLER_URL` is set, you need to have same version of db2connect license as installed db2 client. Check db2 client version using `db2level` command to know version of db2connect license required. The license file should get copied under `$IBM_DB_HOME\license` directory.

- If you do not have db2connect license, contact [IBM Customer Support](https://www.ibm.com/mysupport/s/?language=en_US) to buy db2connect license. Find the `db2con*.lic` file in the db2connect license shared by IBM and copy it under `.../node_modules/ibm_db/installer/clidriver/license` folder to be effective.

- To know more about license and purchasing cost, please contact [IBM Customer Support](https://www.ibm.com/mysupport/s/?language=en_US).

## For AIX install issue

If `npm install ibm_db` aborts with "Out Of Memory" error on AIX, first run `ulimit -d unlimited` and then `npm install ibm_db`.

## If installation fails while downloading clidriver

<a name="downloadCliFailed"></a>

- If installation of `ibm_db` fails because ibm*db fails to download *\_odbc*cli.tar.gz or *\_odbc_cli.zip file, it is most likely that your firewall or VPN configuration is blocking download of this file from internet. In this case, manually download the platform specific clidriver from [DOWNLOAD CLI DRIVER](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/) or [github repo](https://github.com/ibmdb/db2drivers/tree/main/clidriver); untar/unzip it, you'll get clidriver directory.

- Set a system level environment variable `IBM_DB_HOME=<full path of clidriver directory>` and then run `npm install ibm_db`.

- Example for non-Windows:

```
export IBM_DB_HOME=/home/$USER/clidriver
npm install ibm_db
```

- Example for Windows:

```
set IBM_DB_HOME=C:\myproject\clidriver
npm install ibm_db
```

## For Missing Package/Binding issue

If your application is able to connect to IBM Database Server but query execution is throwing SQL0805N error, run below commands to fix the package related issues:

```
cd .../ibm_db/installer
source setenv.sh
db2cli bind $IBM_DB_HOME/bnd/@db2cli.lst -database <dbname>:<hostname>:<port> -user <dbuser> -passwd <passwd> -options "grant public action replace blocking no"
```

If above command prints 0 error at end, then you can proceed to run query. If
it reports non-zero error, open a new issue on github and share the output
of above `db2cli bind` command along with query execution error.

Alternatively, if you have any other DB2 client with CLP, you can bind packages using db2 bind command too. f.e. use below command against DB2 for z/OS server:

```
db2 bind .../sqllib/bnd/@ddcsmvs.lst action replace grant public sqlerror continue messages msg.txt
```

Note: "db2cli bind" command print the logs on output prompt, so you need to redirect output to some file to capture it.
To capture logs of "db2 bind" command, you need to use `messages` option as in above example.
Note: "db2cli bind" does not work with DB2 z/OS if the CLI packages (SYSSH\*) were bound the DB2 subsystem is configured with APPLCOMPAT and SQLLEVEL set to V12R1M502 or higher. Tested with APPLCOMPAT=V12R1M500

## Troubleshooting on z/OS

Some errors on z/OS are incomplete, so, to debug, add the following to your _Db2 ODBC initialization file_:

```
 APPLTRACE=1
 APPLTRACEFILENAME=/u/<username>/odbc_trace.txt
```

### Db2 z/OS: UnicodeDecodeError Exception

- Make sure you have set `CURRENTAPPENSC=ASCII` or `UNICODE` in ODBC initialization file.
- Make sure tagging of your ODBC ini file is "binary" or "mixed".

## Usage within VS Code

If you are using ibm_db to develop extension for VS Code, then ibm_db has to be rebuilt with Electron libraries. This can be achieved by running:

```
npm install ibm_db -vscode
```

ibm_db would automatically be rebuilt with Electron if your installation directory path contains 'db2connect' as a sub-string. This has the same effect as running with '-vscode' flag.

If you know the electron version or get it from `Help->About` option of vscode; you can build native add-on code for specific version of electron using below command:

```
npm install ibm_db -electron="32.2.5"
```

## How to verify database connectivity using ODBC and generate db2trace?

cd to `ibm_db/installer` directory and update database connection information for `db2cli validate` command in `testODBCConnection.sh` file for non-Windows platform and execute it.
For Windows platform, update connection info for `db2cli validate` command in `testODBCConnection.bat` file and execute it from Administrator Command Prompt.
Script `testODBCConnection` set the required environment variables, validate database connectivity and gerate db2trace files.

## How to get db2trace for any node.js test file?

```
copy test.js to ibm-db/test directory
cd ibm_db/installer
source ./setenv.sh
cd ibm_db/test
./trace test.js
```

**trace** script works only on non-windows platform. For Windows, use `testODBCConnection.bat` script. You can replace `db2cli validate` command with `node test.js`in `testODBCConnection.bat` script and execute it.

## Issues while connecting to Informix Server

While using ibm_db against Informix server, you may get few issues if
server is not configured properly. Also, ibm_db connects to only DRDA port.
So, make sure drsoctcp of Informix is configured.

### SQL1042C Error

If ibm_db is returning SQL1042C error while connecting to server, use
"Authentication=SERVER" in connection string. It should avoid the error.
Alternatively, you can set Authentication in db2cli.ini file or db2dsdriver.cfg file too.
This error mostly comes due to presence of multiple copy of db2 client or gskit library in the system.
Run db2level command and it shows path other than `ibm_db\installer\clidriver`, set IBM_DB_HOME to this path and reinstall ibm_db.
If you have `C:\Program Files\IBM\gsk8` directory on windows, remove it from PATH env var and if possible, rename it.

### code-set conversion error

If Informix server is not enabled for UNICODE clients or some code-set object
file is missing on server; server returns this error to ibm_db:
[IBM][CLI Driver][IDS/UNIX64] Error opening required code-set conversion object file.

To avoid this error, remove UNICODE from binding.gyp file and rebuild the ibm_db.

Also to avoid above issues, you can run [ibm_db/installer/ifx.sh](https://github.com/ibmdb/node-ibm_db/blob/master/installer/ifx.sh) script on non-windows system.

## Need Help?

If you encountered any issue with ibm_db, first check for existing solution or
work-around under `issues` or on google groups forum. Links are:

https://github.com/ibmdb/node-ibm_db/issues
https://groups.google.com/forum/#!forum/node-ibm_db

If no solution found, you can open a new issue on github.

### Getting SQL30081N error occasionally, after some time of inactivity : Check issue#810

### Want to configure db2dsdrivre.cfg file to avoid SQL30081N error: Check issue#808

## Build Options

### Debug

If you would like to enable debugging messages to be displayed you can add the
flag `DEBUG` to the defines section of the `binding.gyp` file and then execute
`node-gyp rebuild`.

```javascript
<snip>
'defines' : [
  "DEBUG"
],
<snip>
```

OR,

```
cd ibm_db
npm install --debug
```

Above two method will enable debugging messages from C++ code. You can enable
debugging messages from nodejs code by calling [ibmdb.debug(true)](https://github.com/ibmdb/node-ibm_db/blob/master/APIDocumentation.md#-38-debugvalue)
api from your application.

### Unicode

By default on distributed platforms, UNICODE suppport is enabled. This should
provide the most accurate way to get Unicode strings submitted to your database.
For best results, you may want to put your Unicode string into bound parameters.

On z/OS, UNICODE is disabled by default.

However, if you experience issues or you think that submitting UTF8 strings will
work better or faster, you can remove the `UNICODE` define in `binding.gyp`

```javascript
<snip>
'defines' : [
  "UNICODE"
],
<snip>
```

### timegm vs timelocal

When converting a database time to a C time one may use `timegm` or `timelocal`. See
`man timegm` for the details of these two functions. By default the node-ibm_db bindings
use `timelocal`. If you would prefer for it to use `timegm` then specify the `TIMEGM`
define in `binding.gyp`

```javascript
<snip>
'defines' : [
  "TIMEGM"
],
<snip>
```

### Strict Column Naming

When column names are retrieved from DB2 CLI, you can request by SQL_DESC_NAME or
SQL_DESC_LABEL. SQL_DESC_NAME is the exact column name or none if there is none
defined. SQL_DESC_LABEL is the heading or column name or calculation.
SQL_DESC_LABEL is used by default and seems to work well in most cases.

If you want to use the exact column name via SQL_DESC_NAME, enable the `STRICT_COLUMN_NAMES`
define in `binding.gyp`

```javascript
<snip>
'defines' : [
  "STRICT_COLUMN_NAMES"
],
<snip>
```

## Tips

### Using more than 4 threads on Linux

Be aware that through node v0.9 the uv_queue_work function, which is used to
execute the ODBC functions on a separate thread, uses libeio for its thread
pool. This thread pool by default is limited to 4 threads.

This means that if you have long running queries spread across multiple
instances of ibmdb.Database() or using odbc.Pool(), you will only be able to
have 4 concurrent queries.

You can increase the thread pool size by using @developmentseed's [node-eio]
(https://github.com/developmentseed/node-eio).

#### install:

```bash
npm install eio
```

#### usage:

```javascript
var eio = require("eio");
eio.setMinParallel(threadCount);
```

## Contributor

- Dan VerWeire (dverweire@gmail.com)
- Lee Smith (notwink@gmail.com)
- Bruno Bigras
- Christian Ensel
- Yorick
- Joachim Kainz
- Oleg Efimov
- paulhendrix
- IBM

## Contributing to the node-ibm_db

[Contribution Guidelines](https://github.com/ibmdb/node-ibm_db/blob/master/contributing/CONTRIBUTING.md)

```
Contributor should add a reference to the DCO sign-off as comment in the pull request(example below):
DCO Signed-off-by: Random J Developer <random@developer.org>
```

## License

Copyright (c) 2013 Dan VerWeire <dverweire@gmail.com>

Copyright (c) 2010 Lee Smith <notwink@gmail.com>

Copyright (c) 2014 IBM Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

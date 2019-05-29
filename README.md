# node-ibm_db

An asynchronous/synchronous interface for node.js to IBM DB2 and IBM Informix.

**Supported Platforms** - Windows64, MacOS64, Linuxx64, Linuxia32, AIX, Linux on IBM Z, Linux on Power PC and z/OS.

## API Documentation

> For complete list of ibm_db APIs and example, please check [APIDocumentation.md](https://github.com/ibmdb/node-ibm_db/blob/master/APIDocumentation.md)

## Prerequisite

- For higher versions of node (When building with Node 4 onwards) the compiler must support
C++11. Note the default compiler on RHEL 6 does not have the required support.
Install a newer compiler or upgrade older one.

- Python 2.7 is needed by node-gyp. On z/OS, Python 2.7.13 or higher, but lower than Python 3.0, is required.

- On distributed platforms, you do need not to install any Db2 ODBC client driver for connectivity. `ibm_db` itself downloads and installs an odbc/cli driver from IBM website during installation. Just install `ibm_db` and it is ready for use.

- On z/OS, ODBC driver support is part of IBM Db2 for z/OS 11.0 and 12.0.  Please ensure IBM Db2 for z/OS 11.0 or 12.0 is installed on your given LPAR.  Ensure you follow the instructions to configure your ODBC driver [here](#configure-odbc-driver-on-zos).

- On z/OS only certain versions of node-gyp are supported. This was tested with:<br>
node-gyp 3.4.0<br>
npm 3.10.10<br>
ibm_db: 2.3.0

- Recommended version of node.js is >= V4.X. For node.js version < 4.X and `ibm_db` version > 2.4.1, Visual Studio is required to install `ibm_db` on Windows. 

## Install

You may install the package using npm install command:

```
npm install ibm_db
```

> For more installation details please refer:  [INSTALLATION GUIDE](https://github.com/ibmdb/node-ibm_db/blob/master/INSTALL.md)


### Important Environment Variables and Download Essentials 

`IBM_DB_HOME :`

- USE:
	- On distributed platforms, set this environment variable if you want to avoid downloading of clidriver from the [IBM Hosted URL](#downloadCli) or from the internet.
	- On z/OS, set this environment variable to the High Level Qualifier (HLQ) of your Db2 datasets. During `npm install`, the module will automatically reference ODBC driver header files under: `$IBM_DB_HOME.SDSNC.H` and the sidedeck definitions in `$IBM_DB_HOME.SDSNMACS(DSNAO64C)` to build the node binding.

- HOW:
	- On distributed platforms, set **IBM_DB_HOME** environment variable to a pre-installed **db2 client or server installation directory**.
	- On z/OS, set **IBM_DB_HOME** environment variable to the High Level Qualifier (HLQ) of your Db2 datasets.  For example, if your Db2 datasets are located as `DSNC10.SDSNC.H` and `DSNC10.SDSNMACS`, you need to set `IBM_DB_HOME` environment variable to `DSNC10` with the following statement (can be saved in `~/.profile`):


`IBM_DB_INSTALLER_URL :`

- USE:
	- Set this environment variable to by-pass the IBM Hosted URL for downloading odbc/clidriver.

- HOW:
	- Set **IBM_DB_INSTALLER_URL** environment variable with alternate odbc/clidriver downloading URL link or with locally downloaded "tar/zipped clidriver's parent directory path.

- TIP:
	- If you don't have alternate hosting URL then, you can download the tar/zipped file of clidriver from the [IBM Hosted URL](#downloadCli) and can set the **IBM_DB_INSTALLER_URL** environment variable to the downloaded "tar/zipped clidriver's" parent directory path. No need to untar/unzip the clidriver and do not change the name of downloaded file.

### <a name="downloadCli"></a> Download clidriver ([based on your platform & architecture](#systemDetails)) from the below IBM Hosted URL:
> [DOWNLOAD CLI DRIVER](https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/)

#### <a name="systemDetails"></a> Cli Drivers for Specific Platform and Architecture

|Platform      |Architecture    |Cli Driver               |Supported     |
| :---:        |  :---:         |  :---:                  |  :---:       |
|AIX           |  ppc           |aix32_odbc_cli.tar.gz    |  Yes         |
|              |  others        |aix64_odbc_cli.tar.gz    |  Yes         |
|Darwin        |  x64           |macos64_odbc_cli.tar.gz  |  Yes         |
|Linux         |  x64           |linuxx64_odbc_cli.tar.gz |  Yes         |
|              |  s390x         |s390x64_odbc_cli.tar.gz  |  Yes         |
|              |  s390          |s390_odbc_cli.tar.gz     |  Yes         |
|              |  ppc64  (LE)   |ppc64le_odbc_cli.tar.gz  |  Yes         |
|              |  ppc64         |ppc64_odbc_cli.tar.gz    |  Yes         |
|              |  ppc32         |ppc32_odbc_cli.tar.gz    |  Yes         |
|              |  others        |linuxia32_odbc_cli.tar.gz|  Yes         |
|Windows       |  x64           |ntx64_odbc_cli.zip       |  Yes         |
|              |  x32           |nt32_odbc_cli.zip        |  Not supported with node-ibm_db          |
|z/OS          |  s390x         |ODBC support from IBM Db2 for z/OS 11.0 or 12.0 | Yes  |


### Configure ODBC driver on z/OS

Please refer to the [ODBC Guide and References](https://www.ibm.com/support/knowledgecenter/SSEPEK/pdf/db2z_12_odbcbook.pdf) cookbook for how to configure your ODBC driver.   Specifically, you need to:

1. Apply Db2 on z/OS PTF [UI60551](https://www-01.ibm.com/support/docview.wss?uid=swg1PH05953) to pick up new ODBC functionality to support Node.js applications.

2. Bind the ODBC packages.  A sample JCL is provided in the `SDSNSAMP` dataset in member `DSNTIJCL`.  Customize the JCL with specifics to your system.

3. Ensure users that should be authorized have authority to execute the DSNACLI plan.  Included are samples granting authority to public (all users), or specific groups via SQL GRANT statements, or alternately via RACF.  The security administrator can use these samples as a model and customize/translate to your installation security standards as appropriate.

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


4. Update the `STEPLIB` environment variable to include the Db2 SDSNEXIT, SDSNLOAD and SDSNLOD2 data sets. You can set the `STEPLIB `environment variable in your `.profile` with the following statement, after defining `IBM_DB_HOME` to the high level qualifier of your Db2 datasets as instructed above:

    ```sh
    # Assumes IBM_DB_HOME specifies the HLQ of the Db2 datasets.
    export STEPLIB=$STEPLIB:$IBM_DB_HOME.SDSNEXIT:$IBM_DB_HOME.SDSNLOAD:$IBM_DB_HOME.SDSNLOD2
    ```

5. Configure an appropriate _Db2 ODBC initialization file_ that can be read at application time. You can specify the file by using either a DSNAOINI data definition statement or by defining a `DSNAOINI` z/OS UNIX environment variable.  For compatibility with ibm_db, the following properties must be set:

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

    Reference Chapter 3 in the [ODBC Guide and References](https://www.ibm.com/support/knowledgecenter/SSEPEK/pdf/db2z_12_odbcbook.pdf) for more instructions.


## Quick Example

```javascript
var ibmdb = require('ibm_db');

ibmdb.open("DATABASE=<dbname>;HOSTNAME=<myhost>;UID=db2user;PWD=password;PORT=<dbport>;PROTOCOL=TCPIP", function (err,conn) {
  if (err) return console.log(err);
  
  conn.query('select 1 from sysibm.sysdummy1', function (err, data) {
    if (err) console.log(err);
    else console.log(data);

    conn.close(function () {
      console.log('done');
    });
  });
});
```

## Un-Install

To uninstall node-ibm_db from your system, just delete the node-ibm_db or ibm_db directory.


## For z/OS and iSeries Connectivity

For connectivity against DB2 for LUW or Informix Server using node-ibm_db, 
no license file is required. However, if you want to use node-ibm_db 
against DB2 for z/OS or DB2 for i(AS400) Servers, you must have db2connect 
license of version 11.1 if server is not db2connectactivated to accept
unlimited number of client connection. You can buy db2connect license from IBM.
The connectivity can be enabled either on server using db2connectactivate
utility or on client using client side license file. If you have client side
license file, just copy it under `.../ibm_db/installer/clidriver/license` folder to be effective.

To know more about license and purchasing cost, please contact [IBM Customer Support](http://www-05.ibm.com/support/operations/zz/en/selectcountrylang.html).

## For AIX install issue

If `npm install ibm_db` aborts with "Out Of Memory" error on AIX, first run `ulimit -d unlimited` and then `npm install ibm_db`.

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
Note: "db2cli bind" does not work with DB2 z/OS if the CLI packages (SYSSH*) were bound the DB2 subsystem is configured with APPLCOMPAT and SQLLEVEL set to V12R1M502 or higher. Tested with APPLCOMPAT=V12R1M500

## Troubleshooting on z/OS
Some errors on z/OS are incomplete, so, to debug, add the following to your _Db2 ODBC initialization file_:
APPLTRACE=1
APPLTRACEFILENAME=/u/<username>/odbc_trace.txt

## Usage within VS Code
If you are using ibm_db to develop extension for VS Code, then ibm_db has to be rebuilt with Electron libraries. This can be achieved by running:
```
npm install ibm_db -vscode
```
ibm_db would automatically be rebuilt with Electron if your installation directory path contains 'db2connect' as a sub-string. This has the same effect as running with '-vscode' flag.

## Need Help?

If you encountered any issue with ibm_db, first check for existing solution or
work-around under `issues` or on google groups forum. Links are:   
    
https://github.com/ibmdb/node-ibm_db/issues    
https://groups.google.com/forum/#!forum/node-ibm_db   
   
If no solution found, you can open a new issue on github or start a new topic in google groups.

## How to get ibm_db instance?

The simple api is based on the instances of `Database` class. You may get an 
instance by one of the following ways:

```javascript
require("ibm_db").open(connectionString, function (err, conn){
  //conn is already open now if err is falsy
});
```

or by using the helper function:

```javascript
var ibmdb = require("ibm_db")();
``` 

or by creating an instance with the constructor function:

```javascript
var Database = require("ibm_db").Database
  , ibmdb = new Database();
```

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

### Using node < v0.10 on Linux

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
var eio = require('eio'); 
eio.setMinParallel(threadCount);
```

### Issues while connecting to Informix Server

While using ibm_db against Informix server, you may get few issues if
server is not configured properly. Also, ibm_db connects to only DRDA port.
So, make sure drsoctcp of Informix is configured.

#### SQL1042C Error
If ibm_db is returning SQL1042C error while connecting to server, use
"Authentication=SERVER" in connection string. It should avoid the error.
Alternatively, you can set Authentication in db2cli.ini file or db2dsdriver.cfg file too.

#### code-set conversion error
If Informix server is not enabled for UNICODE clients or some code-set object
file is missing on server; server returns this error to ibm_db:
[IBM][CLI Driver][IDS/UNIX64] Error opening required code-set conversion object file.

To avoid this error, remove UNICODE from binding.gyp file and rebuild the ibm_db.

Also to avoid above issues, you can run [ibm_db/installer/ifx.sh](https://github.com/ibmdb/node-ibm_db/blob/master/installer/ifx.sh) script on non-windows system.

## Contributor

* Dan VerWeire (dverweire@gmail.com)
* Lee Smith (notwink@gmail.com)
* Bruno Bigras
* Christian Ensel
* Yorick
* Joachim Kainz
* Oleg Efimov
* paulhendrix
* IBM

## Contributing to the node-ibm_db

[Contribution Guidelines](https://github.com/ibmdb/node-ibm_db/blob/master/contributing/CONTRIBUTING.md)

```
Contributor should add a reference to the DCO sign-off as comment in the pull request(example below):
DCO 1.1 Signed-off-by: Random J Developer <random@developer.org>
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

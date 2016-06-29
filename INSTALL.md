# Installing node-ibm_db

*Copyright (c) 2014 IBM Corporation and/or its affiliates. All rights reserved.*

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, 
merge, publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, subject to the 
following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.

## Contents

1. [Overview](#Installation)
2. [Node-ibm_db Installation on Linux](#inslnx)
3. [Node-ibm_db Installation on AIX on Power Systems](#insaix_p)
4. [Node-ibm_db Installation on Linux on System z](#inslnx_z)
5. [Node-ibm_db Installation on Linux on Power System](#inslnx_p) 
6. [Node-ibm_db Installation on MacOS](#insmac)
7. [Node-ibm_db Installation on Windows](#inswin)
8. [Node-ibm_db How to Manually Build on Windows](#inswinbld)

## <a name="overview"></a> 1. Overview

The [*node-ibm_db*](https://github.com/ibmdb/node-ibm_db) is an asynchronous/synchronous interface for node.js to IBM DB2 and IBM Informix.

Following are the steps to create a Node.js installation for testing.

This node-ibm_db release has been tested with Node 0.10.36, 0.12.7, 4.2.1, 4.2.2, 4.2.6 LTS versions
and 5.0.0, 5.3.0, 5.10.1 Stable versions on 64-bit/32-bit IBM Linux, IBM AIX, MacOS, Linux on z, 
Linux on Power System and Windows.


## <a name="inslnx"></a> 2. Node-ibm_db Installation on Linux.

### 2.1 Install Node.js

Download the
[Node.js Linux binaries](http://nodejs.org) or [Node.js Latest binaries](https://nodejs.org/dist/latest/) and
extract the file, for example into `/mynode`:

```
cd /mynode
tar -xzf node-v4.2.2-linux-x64.tar.gz
```

Set PATH to include Node.js:

```
export PATH=/mynode/node-v4.2.2-linux-x64/bin:$PATH
```

### 2.2 Install node-ibm_db

Following are the steps to install [*node-ibm_db*](https://github.com/ibmdb/node-ibm_db) from github or npm.
using directory `/nodeapp` for example.

#### 2.2.1 Direct Installation.

```
1. mkdir nodeapp
2. cd nodeapp
```

```
3. npm install ibm_db
```
or
```
npm install git+https://git@github.com/ibmdb/node-ibm_db.git
```
or
```
export IBM_DB_INSTALLER_URL=<local path or url of folder contains odbc_cli.tar.gz file>
npm install ibm_db
```
ibm_db will use the odbc_cli driver from IBM_DB_INSTALLER_URL env variable and will not download from default location used by installer/driverInstall.js

4. Update config.testConnectionStrings.json with your credentials and run test.
```
cd ibm_db/test/
vi config.testConnectionStrings.json
node run-tests.js
```

It's Done.

#### 2.2.2 Manual Installation by using git clone.

```
1. mkdir nodeapp
2. cd nodeapp
3. git clone https://github.com/ibmdb/node-ibm_db/
```

```
4. Set "IBM_DB_HOME" with 'clidriver' path, for example clidriver path is : /home/mysystem/clidriver

export IBM_DB_HOME=/home/mysystem/clidriver
```

```
5. Install node-gyp and other dependencies (refer package.json)

npm install -g node-gyp
npm install moment
npm install nan@2.2.0
npm install bindings@1.0.0
etc...
```

```
6. Set "/node-ibm_db/node_modules/" path into system PATH.

export PATH=/home/mysystem/nodeapp/node-ibm_db/node_modules/.bin:$PATH
```

```
7. Run node-gyp configure build command.

node-gyp configure build --IBM_DB_HOME=$IBM_DB_HOME  --IS_DOWNLOADED=false --verbose
```

```
8. Update config.testConnectionStrings.json with your credentials and run test.

cd node-ibm_db/test/
vi config.testConnectionStrings.json
node run-tests.js
```

It's Done.


## <a name="insaix_p"></a> 3. Node-ibm_db Installation on AIX on Power Systems.

### 3.1 Install Node.js for AIX

Download the
[Node.js AIX binaries](https://developer.ibm.com/node/sdk/#overview) from IBM SDK for Node.js and
execute the binary file, for example into `/mynode`:

```
cd /mynode
./ibm-4.4.3.0-node-v4.4.3-aix-ppc64.bin
```

### 3.2 Install node-ibm_db

Follow the same steps mentioned in [Node-ibm_db Installation on Linux](#inslnx).


## <a name="inslnx_z"></a> 4. Node-ibm_db Installation on Linux on System z.

### 4.1 Install Node.js for Linux on System z

Download the
[Node.js Linux on System z binaries](https://developer.ibm.com/node/sdk/#overview) from IBM SDK for Node.js and
execute the binary file, for example into `/mynode`:

```
cd /mynode
./ibm-4.4.3.0-node-v4.4.3-linux-s390x.bin
```

### 4.2 Install node-ibm_db

Follow the same steps mentioned in [Node-ibm_db Installation on Linux](#inslnx).


## <a name="inslnx_p"></a> 5. Node-ibm_db Installation on Linux on Power System.

### 5.1 Install Node.js for Linux on Power System

Download the
[Node.js Linux on Power System binaries](https://developer.ibm.com/node/sdk/#overview) from IBM SDK for Node.js and
execute the binary file, for example into `/mynode`:

```
cd /mynode
./ibm-4.4.3.0-node-v4.4.3-linux-ppc64.bin
```

### 5.2 Install node-ibm_db

Follow the same steps mentioned in [Node-ibm_db Installation on Linux](#inslnx).


## <a name="insmac"></a> 6. Node-ibm_db Installation on MacOS.

### 6.1 Install Node.js for Mac

Download the
[Node.js MacOS binaries](http://nodejs.org) or [Node.js Latest binaries](https://nodejs.org/dist/latest/) and
extract the file.

### 6.2 Install node-ibm_db

Follow the same steps mentioned in [Node-ibm_db Installation on Linux](#inslnx).


## <a name="inswin"></a> 7. Node-ibm_db Installation on Windows.

### 7.1 Install Node.js for Windows

Download the
[Node.js Windows binary/installer](http://nodejs.org) or [Node.js Latest binaries](https://nodejs.org/dist/latest/) and
install it.

### 7.2 Install node-ibm_db

Following are the steps to install [*node-ibm_db*](https://github.com/ibmdb/node-ibm_db) from github or npm.
using directory `/nodeapp` for example.

```
1. mkdir nodeapp
2. cd nodeapp
```

```
3. npm install ibm_db
```
or
```
npm install git+https://git@github.com/ibmdb/node-ibm_db.git
```

```
4. Update config.testConnectionStrings.json with your credentials and run test.

cd ibm_db/test/
vi config.testConnectionStrings.json
node run-tests.js
```

It's Done.


## <a name="inswinbld"></a> 8. Node-ibm_db How to Manually Build on Windows.

### 8.1 Install Node.js for Windows

Download the
[Node.js Windows binary/installer](http://nodejs.org) or [Node.js Latest binaries](https://nodejs.org/dist/latest/) and
install it.

### 8.2 Make Build node-ibm_db

Following are the steps to make build of [*node-ibm_db*](https://github.com/ibmdb/node-ibm_db).
using directory `/nodeapp` for example.

```
1. mkdir nodeapp
2. cd nodeapp
3. git clone https://github.com/ibmdb/node-ibm_db/
```

```
4. Set `IBM_DB_HOME` environmental variable with 'clidriver' path, for example clidriver path is : `/home/mysystem/clidriver`
   and then set `IBM_DB_HOME` to `PATH` variable.
```

```
5. Install node-gyp and other dependencies (refer package.json)

npm install -g node-gyp
npm install moment
npm install nan@2.2.0
npm install bindings@1.0.0
etc...
```

```
6. Set the msbuild path into `PATH` environment variable.
   ie.: `C:\Program Files (x86)\MSBuild\14.0\bin\`.
```

```
7. Run node-gyp configure build command.

node-gyp configure build --IBM_DB_HOME=$IBM_DB_HOME  --IS_DOWNLOADED=false --verbose
```

If `node-gyp configure build` command succeeded then Follow these steps.

```
8. Delete `build\Release folder`.
9. Open `build\odbc_bindings.vcxproj` file and edit following enteries:

Step1: Search AdditionalDependencies tag in file.

<AdditionalDependencies>kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;DelayImp.lib;&quot;C:\Users\IBM_ADMIN\.node-gyp\5.3.0\$(Configuration)\node.lib&quot;;$(IBM_DB_HOME)\lib\db2cli64.lib;$(IBM_DB_HOME)\lib\db2app64.lib</AdditionalDependencies>

Step2: Delete `kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;DelayImp.lib` from all tag entries.

<AdditionalDependencies>;&quot;C:\Users\IBM_ADMIN\.node-gyp\5.3.0\$(Configuration)\node.lib&quot;;$(IBM_DB_HOME)\lib\db2cli64.lib;$(IBM_DB_HOME)\lib\db2app64.lib</AdditionalDependencies>
```

```
10. Finally run below msbuild command:

msbuild build/binding.sln /clp:Verbosity=minimal /nologo /p:Configuration=Release;Platform=x64

Note: This command will run only in windows command prompt.
```

```
11. Check for `\node-ibm_db\build\Release\odbc_bindings.node` file, If it's there
    Well Done :)
```

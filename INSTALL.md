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
2. [ibm_db Installation on Linux](#inslnx)
3. [ibm_db Installation on AIX on Power Systems](#insaix_p)
4. [ibm_db Installation on Linux on System z](#inslnx_z)
5. [ibm_db Installation on Linux on Power System](#inslnx_p) 
6. [ibm_db Installation on MacOS](#insmac)
7. [ibm_db Installation on Windows](#inswin)
8. [ibm_db Installation on z/OS](#inszos)
9. [ibm_db How to Manually Build on Windows](#inswinbld)
10. [ibm_db installation on MacOS M1/M2 Chip System](#m1chip)

## <a name="overview"></a> 1. Overview

The [*node-ibm_db*](https://github.com/ibmdb/node-ibm_db) is an asynchronous/synchronous interface for node.js to IBM DB2 and IBM Informix.

Following are the steps to create a Node.js installation for testing.

This node-ibm_db driver has been tested on 64-bit/32-bit IBM Linux, IBM AIX, MacOS, Linux on z, Linux on Power System and Windows.


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

4. Install other packages mentioned in package.json as devDependencies
```
npm install async bluebird moment
```

5. Update config.json with your credentials and run test.
```
cd node_modules/ibm_db/test/
vi config.json => Update connection string in this file.
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
npm install moment async bluebird
npm install nan bindings adm-zip q request targz
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
8. Update config.json with your credentials and run test.

cd node-ibm_db/test/
vi config.json  => Update database connection info.
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

4. Install other packages mentioned in package.json as devDependencies
```
npm install async bluebird moment
```

5. Update config.json with your credentials and run test.
```
cd node_modules/ibm_db/test/
vi config.json => Update connection string in this file.
node run-tests.js
```

It's Done.


## <a name="inszos"></a> 8. Node-ibm_db How to Manually Build on z/OS

### 8.1 Install Node.js for z/OS

Download and install the IBM SDK for Node.js - z/OS from [Marketplace](https://www.ibm.com/ca-en/marketplace/sdk-nodejs-compiler-zos/purchase).  Given that ibm_db includes native add-on code, ensure that you have the pre-requisite Python and Make tools installed as detailed in the SDK installation instructions.

### 8.2 Configure ODBC driver on z/OS

Please refer to the [ODBC Guide and References](https://www.ibm.com/support/knowledgecenter/SSEPEK/pdf/db2z_12_odbcbook.pdf) cookbook for how to configure your ODBC driver.   Specifically, you need to ensure you have:

1. Apply Db2 on z/OS PTF [UI60551](https://www-01.ibm.com/support/docview.wss?uid=swg1PH05953) to pick up new ODBC functionality to support Node.js applications.

2. Binded the ODBC packages.  A sample JCL is provided in the `SDSNSAMP` dataset in member `DSNTIJCL`.  Customize the JCL with specifics to your system.

3. Set the `IBM_DB_HOME` environment variable to the High Level Qualifier (HLQ) of your Db2 datasets.  For example, if your Db2 datasets are located as `DSNC10.SDSNC.H` and `DSNC10.SDSNMACS`, you need to set `IBM_DB_HOME` environment variable to `DSNC10` with the following statement (can be saved in `~/.profile`):

    ```sh
    # Set HLQ to Db2 datasets.
    export IBM_DB_HOME="DSNC10"
    ```

4. Update the `STEPLIB` environment variable to include the Db2 SDSNEXIT, SDSNLOAD and SDSNLOD2 data sets. You can set the `STEPLIB` environment variable with the following statement, after defining `IBM_DB_HOME` to the high level qualifier of your Db2 datasets as instructed above:

    ```sh
    # Assumes IBM_DB_HOME specifies the HLQ of the Db2 datasets.
    export STEPLIB=$STEPLIB:$IBM_DB_HOME.SDSNEXIT:$IBM_DB_HOME.SDSNLOAD:$IBM_DB_HOME.SDSNLOD2
    ```

5. Configured an appropriate _Db2 ODBC initialization file_ that can be read at application time. You can specify the file by using either a DSNAOINI data definition statement or by defining a `DSNAOINI` z/OS UNIX environment variable.  For compatibility with ibm_db, the following properties must be set:


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

### 8.3 Install node-ibm_db

The following are the steps to install [*node-ibm_db*](https://github.com/ibmdb/node-ibm_db) from GitHub or npm into a subdirectory `nodeapp` as an example.

From Unix System Services (USS), create a subdirectory `nodeapp` to host your Node.js application:

```sh
mkdir nodeapp
cd nodeapp
```

#### 8.3.1 Direct Installation

Install ibm_db from npm, or from the GitHub repository (requires [Git on z/OS](http://www.rocketsoftware.com/product-categories/mainframe/git-for-zos)):

```sh
# Install via npm repository
npm install ibm_db

# --- OR -----

# Install from GitHub repository
npm install git+ssh://git@github.com/ibmdb/node-ibm_db.git
```

#### 8.3.2 Manual Installation by using git clone

Using [Git on z/OS](http://www.rocketsoftware.com/product-categories/mainframe/git-for-zos), clone the GitHub respository.

```sh
git clone git://github.com/ibmdb/node-ibm_db
cd node-ibm_db
```

Install the ibm_db module dependencies with:

```sh
npm install
```

### 8.4 Running Verification Tests

To run the validation tests, update `node-ibm_db/test/config.zos.json` with your database credentials (DSN, UID, PWD).

Set the `IBM_DB_SERVER_TYPE` environment variable to `"ZOS"` if you are connecting to a z/OS Db2 database.

```sh
export IBM_DB_SERVER_TYPE="ZOS"
```

Execute the tests with:

```sh
cd node-ibm_db
npm test
```

## <a name="inswinbld"></a> 9. Node-ibm_db How to Manually Build on Windows.

### 9.1 Install Node.js for Windows

Download the
[Node.js Windows binary/installer](http://nodejs.org) or [Node.js Latest binaries](https://nodejs.org/dist/latest/) and
install it.

### 9.2 Make Build node-ibm_db

Following are the steps to make build of [*node-ibm_db*](https://github.com/ibmdb/node-ibm_db).
using directory `/nodeapp` for example.

```
1. mkdir nodeapp
2. cd nodeapp
3. git clone https://github.com/ibmdb/node-ibm_db/
```

4. Download platform specific clidriver from https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/ , untar/unzip it and set `IBM_DB_HOME` environmental variable to full path of extracted 'clidriver' directory, for example if clidriver is extracted as: `/home/mysystem/clidriver`, then set system level environment variable `IBM_DB_HOME=/home/mysystem/clidriver`.

5. Set the msbuild path into `PATH` environment variable.
   ie.: `C:\Program Files (x86)\MSBuild\14.0\bin\`.

6. Build odbc_bindings.node using npm command
```
cd node-ibm_db
npm install
```

7. Check for `\node-ibm_db\build\Release\odbc_bindings.node` file, If it's there
    Well Done :)

## <a name="m1chip"></a> 10. Steps to install ibm_db on MacOS M1/M2 Chip system (arm64 architecture)
>
> **Important: We do not have clidriver/dsdriver for ARM64 architecture yet. The driver for IBM DB2 is not compatible with Apple Silicon and will have to run x86_64 version of clidriver in emulated mode.**
> Please support [this request for an Apple Silicon version of the driver](https://ibm-data-and-ai.ideas.ibm.com/ideas/DB2CON-I-92) to show IBM that you are interested in a native solution.
>

**Warning:** If you use the ARM version of homebrew (as recommended for M1/M2 chip systems) you will get the following error message:
```
$ brew install gcc-12
Error: Cannot install in Homebrew on ARM processor in Intel default prefix (/usr/local)!
Please create a new installation in /opt/homebrew using one of the
"Alternative Installs" from:
  https://docs.brew.sh/Installation
You can migrate your previously installed formula list with:
  brew bundle dump
```
Install `gcc@12` using homebrew `(note: the x86_64 version of homebrew is needed for this, not the recommended ARM based homebrew)`. The clearest instructions on how to install and use the `x86_64` version of `homebrew` is by following below steps:
*	My arm64/M1 brew is installed here:
```
	$ which brew
	/opt/homebrew/bin/brew
```
*	Step 1. Install x86_64 brew under /usr/local/bin/brew
	`arch -x86_64 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"`
*	Step 2. Create an alias for the x86_64 brew
	I added this to my ~/.bashrc as below:
```
	# brew hack for x86_64
	alias brew64='arch -x86_64 /usr/local/bin/brew'
```
* Then install gcc@12 using the x86_64 homebrew:
```
	brew64 install gcc@12
```
* Now find location of `lib/gcc/12/libstdc++.6.dylib` file in your system. It should be inside `/usr/local/homebrew/lib/gcc/12` or `/usr/local/lib/gcc/12` or `/usr/local/homebrew/Cellar/gcc@12/12.2.0/lib/gcc/12` or something similar. You need to find the correct path.
Suppose path of gcc lib is `/usr/local/homebrew/lib/gcc/12`. Then update your .bashrc/.zshrc file with below line
```
export DYLD_LIBRARY_PATH=/usr/local/homebrew/lib/gcc/12:$DYLD_LIBRARY_PATH
```

## Steps to Install Intel Python after verifying setup
* Open new terminal and run command `arch -x86_64 /bin/bash    or arch -x86_64 /bin/zsh`.
*  Verify the output of  `gcc -v` command. It should show `Target: x86_64-apple-darwin21` in output.
* Install Intel Version of Python like: https://www.python.org/ftp/python/3.9.11/python-3.9.11-macosx10.9.pkg
* Check output of commands `python --version` , `which python` and `file /usr/local/bin/python` commands. It should show `/usr/local/bin/python: Mach-O 64-bit executable x86_64`

## Steps to Install x64 version of Node.js
* Open new terminal and run command `arch -x86_64 /bin/bash    or arch -x86_64 /bin/zsh`.
*  Verify the output of  `gcc -v` command. It should show `Target: x86_64-apple-darwin21` in output.
* Install x64 Version of NodeJS like: https://nodejs.org/dist/v18.16.0/node-v18.16.0-darwin-x64.tar.gz
* Check output of commands `node -v` , `which node` and `file node` commands. It should show `node: Mach-O 64-bit executable x86_64`

## Install ibm_db with x86_64 version of gcc12 and node.js on M1/M2 Chip System

* Open a new terminal and run below commands:
```
gcc -v   => Output should have x86_64
python --version   or python3 --version
file `which python` or file `which python3`   => Output should have x86_64
node -v
file `which node` => Output should have x86_64
npm uninstall ibm_db
npm install ibm_db
cd node_modules/ibm_db/installer
source ./setenv.sh
run your test program to verify.
```

### Getting error and unable to run test file
* If connection fails with SQL30081N error: means `ibm_db` installation is correct and you need to provide correct connection string.

* If `import ibm_db` fails with `Symbol not found: ___cxa_throw_bad_array_new_length` error or `malloc` error:
  You need to find the correct location of lib/gcc/12 directory and add it to DYLD_LIBRARY_PATH environment variable.
  Also, execute below commands from terminal with correct location of `lib/gcc/12/libstdc++.6.dylib` library.
  ```
  cd node_modules/ibm_db/installer/clidriverd/lib
  install_name_tool -change /usr/local/lib/gcc/8/libstdc++.6.dylib <full path of libstdc++.6.dylib> libdb2.dylib
  f.e.
  install_name_tool -change /usr/local/lib/gcc/8/libstdc++.6.dylib /usr/local/homebrew/lib/gcc/12/libstdc++.6.dylib libdb2.dylib
  ```
Now run your test program and verify.

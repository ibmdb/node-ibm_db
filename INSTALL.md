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
3. [Node-ibm_db Installation on AIX](#insaix)
4. [Node-ibm_db Installation on Linux on z](#inslnx_z)
5. [Node-ibm_db Installation on Linux on Power PC](#inslnx_p) 
6. [Node-ibm_db Installation on MacOS](#insmac)
7. [Node-ibm_db Installation on Windows](#inswin)

## <a name="overview"></a> 1. Overview

The [*node-ibm_db*](https://github.com/ibmdb/node-ibm_db) is an asynchronous/synchronous interface for node.js to IBM DB2 and IBM Informix.

Following are the steps to create a Node.js installation for testing.

This node-ibm_db release has been tested with Node 0.10.36, 0.12.7, 4.2.1, 4.2.2, 4.2.6 LTS versions
and 5.0.0, 5.3.0, 5.10.1 Stable versions on 64-bit/32-bit IBM Linux, IBm AIX, MacOS, Linux on z, 
Linux on Power PC and Windows.

### Prerequisites

For higher versions of node (When building with Node 4 onwards) the compiler must support
C++11. Note the default compiler on RHEL 6 does not have the required support.
Install a newer compiler or upgrade older one.

Python 2.7 is needed by node-gyp.

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
3. npm install git+https://git@github.com/ibmdb/node-ibm_db.git
```
or
```
npm install ibm_db
```

Update config.testConnectionStrings.json with your credentials

```
4. cd ibm_db/test/
5. vi config.testConnectionStrings.json
```

It's Done.

#### 2.2.2 Manual Installation by using git clone.

```
1. mkdir nodeapp
2. cd nodeapp
```

```
3. git clone https://github.com/ibmdb/node-ibm_db/
```

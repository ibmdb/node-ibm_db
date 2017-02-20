#!/bin/ksh

# Script to remove UNICODE from binding.gyp and compile ibm_db.
# Also, add Authentication=SERVER in db2cli.ini and db2dsdriver.cfg file
# to avoid SQL1042C error from security layer.

if [ "$IBM_DB_HOME" == "" ]
then
  IBM_DB_HOME=`pwd`/clidriver
fi
OS=`uname`

export PATH=$IBM_DB_HOME/bin:$IBM_DB_HOME/adm:$IBM_DB_HOME/lib:$PATH
if [ "$OS" == "Darwin" ]
then
  export DYLD_LIBRARY_PATH=$IBM_DB_HOME/lib:$DYLD_LIBRARY_PATH
else
  export LD_LIBRARY_PATH=$IBM_DB_HOME/lib:$LD_LIBRARY_PATH
fi

cd ..
npm install nan bindings
npm install -g node-gyp

grep -v UNICODE binding.gyp > binding1.gyp
mv binding1.gyp binding.gyp

node-gyp rebuild --IBM_DB_HOME=$IBM_DB_HOME  --IS_DOWNLOADED=false

db2cli writecfg add -parameter Authentication=SERVER
echo "writeini common Authentication SERVER" > .t.cli
echo "" >> .t.cli
db2cli < .t.cli
rm .t.cli


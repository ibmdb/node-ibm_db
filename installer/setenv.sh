# Script file to set environment variables to use db2cli executable from 
# ibm_db/installer/clidriver/bin folder
# This script is only for non-Windows platform.

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

export INCLUDE=$IBM_DB_HOME/include:$INCLUDE


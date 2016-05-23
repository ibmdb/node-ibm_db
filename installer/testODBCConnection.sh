# Script file to try connection to target server using ODBC driver
# and collect db2trace files for this connection to diagnose the
# connection related issue and make sure setup is proper.
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

rm -rf 1.trc 1.flw 1.fmt 1.cli
if [ "$OS" == "Darwin" ]
then
  db2trc on -l 2m
else
  db2trc on -f 1.trc
fi
sleep 5

db2cli validate -database "sample:hotel.torolab.ibm.com:21169" -connect -user newton -passwd serverpass
# You can use either above db2cli command or below node command to run .js file.
# Keep only one and comment other. Better to use above validate command first.
#node ../defect/t.js

if [ "$OS" == "Darwin" ]
then
db2trc dump 1.trc
fi
db2trc off
db2trc flw 1.trc 1.flw
db2trc fmt 1.trc 1.fmt
db2trc fmt -cli 1.trc 1.cli


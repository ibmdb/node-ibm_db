# Script file to try connection to target server using ODBC driver
# and collect db2trace files for this connection to diagnose the
# connection related issue and make sure setup is proper.
# This script is only for non-Windows platform.

if [ "$IBM_DB_HOME" = "" ]
then
  IBM_DB_HOME=`pwd`/clidriver
fi
OS=`uname`

export PATH=$IBM_DB_HOME/bin:$IBM_DB_HOME/adm:$IBM_DB_HOME/lib:$PATH
if [ "$OS" = "Darwin" ]
then
  export DYLD_LIBRARY_PATH=$IBM_DB_HOME/lib:$DYLD_LIBRARY_PATH
else if [ "$OS" = "AIX" ]
then
  export LIBPATH=$IBM_DB_HOME/lib:$LIBPATH
else
  export LD_LIBRARY_PATH=$IBM_DB_HOME/lib:$LD_LIBRARY_PATH
fi

rm -rf 1.trc 1.flw 1.fmt 1.cli
if [ "$OS" = "Darwin" ]
then
  db2trc on -t -l 2m
else
  db2trc on -t -f 1.trc
fi
sleep 5

# Use below command if your clidriver version is older than 11.5.9
# db2cli validate -database "sample:hotel.torolab.ibm.com:21169" -connect -user newton -passwd serverpass

# From clidriver version db2 v11.5.9 onwards, use below command
# Update database connection string in below command and run using "./testconnection.sh" command

connStr="DATABASE=sample;HOSTNAME=dbserver.host.com;PORT=dbport;UID=dbuser;PWD=dbpasswd;"
serverIsZOSoriSeries=0

if [ $serverIsZOSoriSeries -eq 1 ]
then
  db2cli validate -connstring "$connStr" -connect -displaylic | tee 1.txt
else
  db2cli validate -connstring "$connStr" -connect | tee 1.txt
fi

# You can use either above db2cli command or below node command to run .js file.
# Keep only one and comment other. Better to use above validate command first.
#node ../defect/t.js

if [ "$OS" = "Darwin" ]
then
db2trc dump 1.trc
fi
db2trc off
db2trc flw -t 1.trc 1.flw
db2trc fmt 1.trc 1.fmt
db2trc fmt -c 1.trc 1.fmtc
db2trc fmt -cli 1.trc 1.cli


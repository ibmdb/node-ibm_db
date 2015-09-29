export PATH=$IBM_DB_HOME/bin:$IBM_DB_HOME/adm:$IBM_DB_HOME/lib:./clidriver/bin:./clidriver/adm:./clidriver/lib:$PATH
export LD_LIBRARY_PATH=$IBM_DB_HOME/lib:./clidriver/lib
export DYLD_LIBRARY_PATH=$IBM_DB_HOME/lib:./clidriver/lib:./clidriver/lib/icc

rm -rf 1.trc 1.flw 1.fmt 1.cli
db2trc on -f 1.trc
sleep 5

db2cli validate -database "sample:hotel.torolab.ibm.com:21169" -connect -user newton -passwd serverpass

db2trc off
db2trc flw 1.trc 1.flw
db2trc fmt 1.trc 1.fmt
db2trc fmt -cli 1.trc 1.cli


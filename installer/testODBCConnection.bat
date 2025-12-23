:: Script file to try connection to target server using ODBC driver
:: and collect db2trace files for this connection to diagnose the
:: connection related issue and make sure setup is proper.
:: This script is only for Windows platform.

@ECHO OFF

:: Check for Administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script requires Administrator privileges.
    echo Please run this script from an Administrator Command Prompt.
    echo Right-click on Command Prompt and select "Run as administrator"
    exit /b 1
)

if "%IBM_DB_HOME%" == "" (SET IBM_DB_HOME=%cd%\clidriver)

SET PATH=%IBM_DB_HOME%\bin;%IBM_DB_HOME%\adm;%IBM_DB_HOME%\lib;%PATH%
SET LIB=%IBM_DB_HOME%\lib;%LIB%

DEL /F 1.trc 1.flw 1.fmt 1.fmtc 1.cli
db2trc on -t -f 1.trc

:: Use below command if your clidriver version is older than 11.5.9
:: db2cli validate -database "sample:hotel.torolab.ibm.com:21169" -connect -user newton -passwd serverpass

:: From clidriver version db2 v11.5.9 onwards, use below command
:: Update your database connection string in below command and run this file:
set connStr="DATABASE=sample;HOSTNAME=db2server.host.com;PORT=dbport;UID=dbuser;PWD=dbpass;protocol=TCPIP"
set serverIsZOSoriSeries=0

if %serverIsZOSoriSeries%==1 (
    db2cli validate -connstring "%connStr%" -connect -displaylic > 1.txt
) else (
    db2cli validate -connstring "%connStr%" -connect > 1.txt
)

:: You can use either above db2cli command or below node command to run .js file.
:: Keep only one and comment other. Better to use above validate command first.
:: node ../defect/t.js

db2trc off
db2trc flw -t 1.trc 1.flw
db2trc fmt 1.trc 1.fmt
db2trc fmt -c 1.trc 1.fmtc
db2trc fmt -cli 1.trc 1.cli

:END
ECHO ON
@EXIT /B 0

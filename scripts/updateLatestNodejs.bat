@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM Install the latest Node.js, rebuild ibm_db, and update the optional binary repo.
REM Usage: updateLatestNodejs.bat

set "CURR_DIR=%~dp0"
if "!CURR_DIR:~-1!"=="\" set "CURR_DIR=!CURR_DIR:~0,-1!"
for %%I in ("!CURR_DIR!\..") do set "IBMDB_DIR=%%~fI"
for %%I in ("!IBMDB_DIR!\..") do set "BINARY_DIR=%%~fI\ibmdb-binaries"
for %%I in ("!CURR_DIR!\..\..\..") do set "NODEWORK=%%~fI"
set "NODE_EXE="
set "PATH_NODE_VERSION="
for /f "delims=" %%N in ('where node 2^>nul') do if not defined NODE_EXE set "NODE_EXE=%%~fN"
if defined NODE_EXE (
    for /f "tokens=*" %%v in ('"!NODE_EXE!" -v 2^>nul') do set "PATH_NODE_VERSION=%%v"
    if defined PATH_NODE_VERSION (
        for %%I in ("!NODE_EXE!") do set "NODEJS_DIR=%%~dpI"
        if "!NODEJS_DIR:~-1!"=="\" set "NODEJS_DIR=!NODEJS_DIR:~0,-1!"
        for %%I in ("!NODEJS_DIR!") do set "NODEJS_DIR_NAME=%%~nxI"
        if /i "!NODEJS_DIR_NAME!"=="nodejs" (
            for %%I in ("!NODEJS_DIR!\..") do set "NODEWORK=%%~fI"
        )
    )
)
echo Node work directory = !NODEWORK!

if not exist "!IBMDB_DIR!\installer\driverInstall.js" (
    echo Error: unable to find ibm_db directory at !IBMDB_DIR!
    exit /b 1
)

where curl >nul 2>nul
if errorlevel 1 (
    echo Required command 'curl' not found.
    exit /b 1
)
where powershell >nul 2>nul
if errorlevel 1 (
    echo Required command 'powershell' not found.
    exit /b 1
)

set "BASE_URL=https://nodejs.org/download/release/latest/"
set "ARCHIVE_PATTERN=win-x64.zip"
set "TARBALL="
for /f "usebackq delims=" %%a in (`powershell -NoProfile -Command "$ProgressPreference='SilentlyContinue'; $r = (Invoke-WebRequest -UseBasicParsing '%BASE_URL%').Content; if ($r -match 'node-v\d+\.\d+\.\d+-%ARCHIVE_PATTERN%') { $Matches[0] }"`) do set "TARBALL=%%a"
if "!TARBALL!"=="" (
    echo Failed to detect latest Node.js version.
    exit /b 1
)
for /f "usebackq delims=" %%a in (`powershell -NoProfile -Command "'!TARBALL!' -replace '^node-(v\d+\.\d+\.\d+)-.*','$1'"`) do set "LATEST_VERSION=%%a"
set "NODEDIR=!NODEWORK!\node!LATEST_VERSION!"
set "UNZIP_NAME=!TARBALL:.zip=!"
set "ACTIVE_DIR=!NODEWORK!\nodejs"
set "ACTIVE_VERSION="
if exist "!ACTIVE_DIR!\node.exe" (
    for /f "tokens=*" %%v in ('"!ACTIVE_DIR!\node.exe" -v') do set "ACTIVE_VERSION=%%v"
)

echo Latest Node.js = !LATEST_VERSION!
if "!ACTIVE_VERSION!"=="!LATEST_VERSION!" goto node_ready

if exist "!NODEDIR!\node.exe" (
    echo Found !NODEDIR!
    goto candidate_ready
)

echo Downloading !BASE_URL!!TARBALL!
cd /d "!NODEWORK!" || exit /b 1
curl -fLO "!BASE_URL!!TARBALL!"
if errorlevel 1 (
    echo Download failed!
    exit /b 1
)
powershell -NoProfile -Command "Expand-Archive -Path '!TARBALL!' -DestinationPath '!NODEWORK!' -Force"
if errorlevel 1 (
    echo Extraction failed!
    exit /b 1
)
if not exist "!NODEWORK!\!UNZIP_NAME!\node.exe" (
    echo Extracted Node.js archive does not contain node.exe.
    exit /b 1
)
move "!NODEWORK!\!UNZIP_NAME!" "!NODEDIR!" >nul
if errorlevel 1 (
    echo Failed to install Node.js into !NODEDIR!
    exit /b 1
)
del "!TARBALL!"

:candidate_ready
if not exist "!ACTIVE_DIR!" goto activate_node
if "!ACTIVE_VERSION!"=="" (
    echo Error: cannot determine the version of !ACTIVE_DIR!
    exit /b 1
)

set "PRESERVED_DIR=!NODEWORK!\node!ACTIVE_VERSION!"
if not exist "!PRESERVED_DIR!" goto preserve_node
set "PRESERVED_DIR=!NODEWORK!\node!ACTIVE_VERSION!.previous"
set /a SUFFIX=1

:choose_preserved_dir
if not exist "!PRESERVED_DIR!" goto preserve_node
set "PRESERVED_DIR=!NODEWORK!\node!ACTIVE_VERSION!.previous.!SUFFIX!"
set /a SUFFIX+=1
goto choose_preserved_dir

:preserve_node
move "!ACTIVE_DIR!" "!PRESERVED_DIR!" >nul
if errorlevel 1 (
    echo Failed to preserve !ACTIVE_DIR! as !PRESERVED_DIR!
    exit /b 1
)
echo Moved old nodejs to !PRESERVED_DIR!

:activate_node
move "!NODEDIR!" "!ACTIVE_DIR!" >nul
if errorlevel 1 (
    echo Failed to activate latest Node.js from !NODEDIR!
    exit /b 1
)
echo Latest node !LATEST_VERSION! is now !ACTIVE_DIR!

:node_ready
set "NODEDIR=!ACTIVE_DIR!"
set "CHECK_VERSION="
for /f "tokens=*" %%v in ('"!NODEDIR!\node.exe" -v') do set "CHECK_VERSION=%%v"
if not "!CHECK_VERSION!"=="!LATEST_VERSION!" (
    echo Error: expected Node.js !LATEST_VERSION!, got !CHECK_VERSION!.
    exit /b 1
)
set "PATH=!NODEDIR!;!PATH!"
echo Using node !CHECK_VERSION!

set "IBM_DB_HOME="
set "DOWNLOAD_CLIDRIVER=true"
cd /d "!IBMDB_DIR!" || exit /b 1
call npm install
if errorlevel 1 (
    echo npm install failed!
    exit /b 1
)

set "BUILT=!IBMDB_DIR!\build\Release\odbc_bindings.node"
if not exist "!BUILT!" (
    echo Error: !BUILT! was not created.
    exit /b 1
)

if not exist "!BINARY_DIR!\" (
    echo Binary repository not found at !BINARY_DIR!; skipping binary copy.
    goto done
)

set "BINARY_NAME=odbc_bindings_win.node"
set "VERSION_FILE=!BINARY_DIR!\binaryVersions.txt"
set "PATTERN=Windows x86_64 NAPI Node Version = "
copy /y "!BUILT!" "!BINARY_DIR!\!BINARY_NAME!" >nul
if errorlevel 1 (
    echo Failed to copy !BUILT! to !BINARY_DIR!\!BINARY_NAME!
    exit /b 1
)
echo Copied !BUILT! to !BINARY_DIR!\!BINARY_NAME!

findstr /b /c:"!PATTERN!" "!VERSION_FILE!" >nul 2>nul
if errorlevel 1 (
    >>"!VERSION_FILE!" echo !PATTERN!!LATEST_VERSION!
) else (
    powershell -NoProfile -Command "$f='!VERSION_FILE!'; $p='!PATTERN!'; $v='!LATEST_VERSION!'; (Get-Content -LiteralPath $f) | ForEach-Object { if ($_.StartsWith($p)) { $p + $v } else { $_ } } | Set-Content -LiteralPath $f"
    if errorlevel 1 (
        echo Failed to update !VERSION_FILE!
        exit /b 1
    )
)

:done
echo Done!
endlocal

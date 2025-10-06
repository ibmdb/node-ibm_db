/**
 * Node-ibm_db Installer file.
 */

var fs = require('fs');
var url = require('url');
var https = require('https');
var os = require('os');
var path = require('path');
var exec = require('child_process').exec;
var execSync = require('child_process').execSync;
var axios = require('axios');

//IBM provided URL for downloading clidriver.
var installerURL = 'https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli';
var githubURL = 'https://github.com/ibmdb/db2drivers/raw/main/clidriver/';
var license_agreement = '\n****************************************\nYou are downloading a package which includes the Node.js module for IBM DB2/Informix.  The module is licensed under the Apache License 2.0. The package also includes IBM ODBC and CLI Driver from IBM, which is automatically downloaded as the node module is installed on your system/device. The license agreement to the IBM ODBC and CLI Driver is available in ';
var license_agreement2 = '. Check for additional dependencies, which may come with their own license agreement(s). Your use of the components of the package and dependencies constitutes your acceptance of their respective license agreements. If you do not accept the terms of any license agreement(s), then delete the relevant component(s) from your device.\n****************************************\n';

var CURRENT_DIR = process.cwd();
var DOWNLOAD_DIR = path.resolve(CURRENT_DIR, 'installer');
var INSTALLER_FILE; 
var deleteInstallerFile = false;
var platform = os.platform();
var arch = os.arch();

var vscode_build = false;
var electron_version = '37.3.1';
var downloadProgress = 0;
var silentInstallation = false;

if(process.env.npm_config_loglevel == 'warn') { // -quiet option
    downloadProgress = 1;
}
if(process.env.npm_config_loglevel == 'silent') { // -silent option
    downloadProgress = 4;
    silentInstallation = true;
}
if(downloadProgress == 0) {
  printMsg("INFO: platform = " + platform + ", arch = " + arch +
           ", node.js version = " + process.version);
}
if(platform != 'darwin' && arch == 'arm64') {
    console.log('ERROR: ARM64 processor is not supported on ' + platform + ' platform.\n' +
                'Only MacOS with arm64 processor is supported.\n');
    process.exit(1);
}

var httpsAgent;
if (process.env.npm_config_cafile) {
    const ca = fs.readFileSync(process.env.npm_config_cafile);
    httpsAgent = new https.Agent({ ca });
}
else if (process.env.npm_package_config_cafile) {
    const ca = fs.readFileSync(process.env.npm_package_config_cafile);
    httpsAgent = new https.Agent({ ca });
}

/* Read specific version of clidriver specified with install command
 * npm install ibm_db -clidriver=v11.5.9
 */
var clidriverVersion="v12.1.0";
if(process.env.npm_config_clidriver && process.env.npm_config_clidriver != true) {
    clidriverVersion = process.env.npm_config_clidriver;
}

/* Read specific version of clidriver specified in config of package.json file
 * npm v11.x has deprecated npm_config_clidriver and throwing warning
 * "config" : { "clidriver" : "v11.5.0" }
 */
else if(process.env.npm_package_config_clidriver && process.env.npm_package_config_clidriver != true) {
    clidriverVersion = process.env.npm_package_config_clidriver;
}

/* User can specify version of clidriver to be downloaded using
 * export CLIDRIVER_DOWNLOAD_VERSION=v11.5.9
 */
else if(process.env.CLIDRIVER_DOWNLOAD_VERSION) {
    clidriverVersion = process.env.CLIDRIVER_DOWNLOAD_VERSION;
}

if(clidriverVersion && !clidriverVersion.startsWith("v1")) {
    printMsg("WARNING: Ignoring invalid clidriver version " + clidriverVersion);
    clidriverVersion = "";
}

if(platform == 'darwin') {
  if(arch == 'arm64' && clidriverVersion.startsWith("v11")) {
    // v11 do not have arm64 clidriver, use default one.
    clidriverVersion="";
  }
  else if(arch == 'x64' && clidriverVersion.startsWith("v12")) {
    // v12 do not have x64 clidriver, use default one.
    clidriverVersion="";
  }
}
if(clidriverVersion) {
    installerURL = installerURL + "/" + clidriverVersion;
    githubURL = githubURL + clidriverVersion + "/";
}

/* Show make version on non-windows platform, if installed. */
printMakeVersion();

/* Find electron version to use if ibm_db requires electron headers. */
findElectronVersion();

/*
 * "process.env.IBM_DB_INSTALLER_URL"
 * USE: to by-pass the IBM provided URL for downloading clidriver.
 * HOW: set environment variable with alternate downloading URL link.
 *      or locally downloaded "tar/zipped clidriver's" parent directory path.
 *      You can add IBM_DB_INSTALLER_URL in .npmrc file too.
 */
installerURL = process.env.npm_config_IBM_DB_INSTALLER_URL ||
               process.env.npm_package_config_IBM_DB_INSTALLER_URL ||
               process.env.IBM_DB_INSTALLER_URL || installerURL;
installerURL = installerURL + "/";

//Function to download clidriver and install node-ibm_db
var install_node_ibm_db = function(file_url) {
    var endian = os.endianness();
    var installerfileURL;
    var installerfileURL2;
    var odbc_driver;
    var gitDownload = false;

    var AdmZip = require('adm-zip');

    var IBM_DB_HOME, IBM_DB_INCLUDE, IBM_DB_LIB, IBM_DB_DIR;

    /*
     * Installer steps: Generic for all platforms :
     * 1: Check IBM_DB_HOME path first, if present then install accordingly.
     * 2: If IBM_DB_HOME is not set, then download "clidriver" and then install.
     * 
     * Installer Steps: For windows only :
     * Step 1 and Step 2 are same.
     * There are two kinds of windows installation now:
     * 1: Auto Installation (Compilation and building - required Visual Studio).
     * 2: Pre-compiled Binary Installation.
     * 
     * If in any case "Auto Installation" fails, then the Installer will
     * automatically pick up the "Pre-compiled Binary Installation"" process. 
     * 
     */

    //If building for supporting VSCode Extn, then remove Clidriver folder and get it freshly
    //If environment variable DOWNLOAD_CLIDRIVER is set to true, then remove Clidriver folder and get it freshly
    if((vscode_build || process.env.DOWNLOAD_CLIDRIVER == "true") &&
       fs.existsSync(path.join(DOWNLOAD_DIR,'clidriver'))){
        deleteFolderRecursive(path.join(DOWNLOAD_DIR,'clidriver'));
    }

    //If environment variable DOWNLOAD_CLIDRIVER is set to true, then ignore setting of IBM_DB_HOME
    if(process.env.DOWNLOAD_CLIDRIVER == "true"){
        process.env.IBM_DB_HOME = '';
        printMsg('INFO: DOWNLOAD_CLIDRIVER environment variable is set, ' +
                 'proceeding to download clidriver.\n');
    }

    // Check if env var IBM_DB_HOME is set
    var IS_ENVIRONMENT_VAR;
    var clidriverFound = false;

    if (process.env.IBM_DB_HOME) {
        if (fs.existsSync(process.env.IBM_DB_HOME) || platform == "os390") {
          IBM_DB_HOME = process.env.IBM_DB_HOME;
          IS_ENVIRONMENT_VAR = true;
          clidriverFound = true;
        } else {
          printMsg(process.env.IBM_DB_HOME + " directory does not exist. Please" +
                " check if you have set the IBM_DB_HOME environment" +
                " variable\'s value correctly.\n");
        }
    }

    if (clidriverFound == false && fs.existsSync(DOWNLOAD_DIR + "/clidriver/include/sqlcli.h")){
        IBM_DB_HOME = path.resolve(DOWNLOAD_DIR, 'clidriver');
        process.env.IBM_DB_HOME = IBM_DB_HOME.replace(/\s/g,'\\ ');
        IS_ENVIRONMENT_VAR = false;
        clidriverFound = true;
    }

    // Check for the existence of include and lib directories
    if (IBM_DB_HOME) {
        if(platform != 'os390') {
            IBM_DB_INCLUDE = path.resolve(IBM_DB_HOME, 'include');
            if (fs.existsSync(IBM_DB_HOME + "/lib64")) {
              IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib64');
            } else if (fs.existsSync(IBM_DB_HOME + "/lib32")) {
              IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib32');
            } else {
              IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib');
            }
            clidriverFound = true;

            if (!fs.existsSync(IBM_DB_LIB)) {
              console.log(IBM_DB_LIB, " directory does not exist. Please ",
                  "check if you have set the IBM_DB_HOME environment ",
                  "variable\'s value correctly.\n");
              clidriverFound = false;
            }
            else if(!(platform == 'win32' && IS_ENVIRONMENT_VAR == false)) {
              if (!fs.existsSync(IBM_DB_INCLUDE)) {
                printMsg(IBM_DB_INCLUDE + " directory does not exist. " +
                        "Please check if you have set the IBM_DB_HOME " +
                        "environment variable\'s value correctly.\n");
                clidriverFound = false;
              }
            }
            if( clidriverFound == false ) {
                printMsg("Discarding IBM_DB_HOME setting.\n");
                IS_ENVIRONMENT_VAR = undefined;
                IBM_DB_HOME = undefined;
                process.env.IBM_DB_HOME = undefined;
            }
        }
    }

    /*
     * IF: IBM_DB_HOME path is set ->
     * CASE 1: If "IBM_DB_HOME" environment variable path is set.
     * CASE 2: If "npm rebuild" and clidriver exists at DOWNLOAD_DIR location.
     * clidriver will not be download from remote location
     * node-ibm_db will use local clidriver package stored in-
     * IBM_DB_HOME path location.
     * ELSE: platform specific compressed clidriver package will be download
     * and then extract for further use.
     */
    if(clidriverFound == true)
    {
        if (platform == 'os390') {
          // On z/OS, we need to extract the include header files from
          // SDSNC.H, and the sidedeck definition from SDSNMACS(DSNAO64C)
          var buildDir = CURRENT_DIR + '/build';
          if (!fs.existsSync(buildDir)) {
             fs.mkdirSync(buildDir, 0744);
          }
          var includeDir = buildDir + '/include';
          if (!fs.existsSync(includeDir)) {
             fs.mkdirSync(includeDir, 0744);
          }
          // Copy the header files from SDSNC.H
          execSync("cp \"//'" + IBM_DB_HOME + ".SDSNC.H'\" " + includeDir);

          // Add .h suffix to header files.
          var headers = fs.readdirSync(includeDir);
          for (var i in headers) {
            var pattern = /\.h$/i;
            var headerFile = includeDir + "/" + headers[i];
            if (!headerFile.match(pattern)) {
               fs.renameSync(headerFile, headerFile + ".h");
            }
          }

          // Copy the sidedeck definition to USS
          // Need to use TSO OPUT command to retain the FB80.
          execSync("tso \"oput '" + IBM_DB_HOME + ".SDSNMACS(DSNAO64C)' '" + buildDir + "/dsnao64c.x'\" | cat");
          // Build the binary
          buildBinary(!IS_ENVIRONMENT_VAR);
        } else {
            if(IS_ENVIRONMENT_VAR) {
              printMsg('INFO: IBM_DB_HOME environment variable have already ' +
                  'set to -> ' + IBM_DB_HOME);
              printMsg('INFO: Downloading of clidriver skipped - build is in progress...\n');
            } else {
              printMsg('INFO: Rebuild Process: Found clidriver at -> ' + IBM_DB_HOME);
              printMsg('INFO: Downloading of clidriver skipped - build is in progress...\n');
            }

            if( platform != 'win32') {
              if(!fs.existsSync(IBM_DB_HOME + "/lib"))
                fs.symlinkSync(IBM_DB_LIB, path.resolve(IBM_DB_HOME, 'lib'));

              if((platform == 'linux') || (platform =='aix') || (platform == 'darwin')) {
                  removeWinBuildArchive();
                  buildBinary(!IS_ENVIRONMENT_VAR);
              }
            }
            else if(platform == 'win32' && arch == 'x64') {
              buildBinary(!IS_ENVIRONMENT_VAR);
            } else {
              console.log('Building binaries for node-ibm_db. This platform ' +
                  'is not completely supported, you might encounter errors. ' +
                  'In such cases please open an issue on our repository, ' +
                  'https://github.com/ibmdb/node-ibm_db. \n');
            }
        }
    }
    else
    {
        if(platform == 'win32') {
            if(arch == 'x64') {
                odbc_driver = 'ntx64_odbc_cli.zip';
            }
        }
        else if(platform == 'linux') 
        {
            if(arch == 'x64') {
                odbc_driver = 'linuxx64_odbc_cli.tar.gz';
            } else if(arch == 's390x') {
                odbc_driver = 's390x64_odbc_cli.tar.gz';
            } else if(arch == 's390') {
                odbc_driver = 's390_odbc_cli.tar.gz';
            } else if(arch == 'ppc64') {
                if(endian == 'LE')
                    odbc_driver = 'ppc64le_odbc_cli.tar.gz';
                else
                    odbc_driver = 'ppc64_odbc_cli.tar.gz';
            } else if(arch == 'ppc32') {
                odbc_driver = 'ppc32_odbc_cli.tar.gz';
            } else {
                odbc_driver = 'linuxia32_odbc_cli.tar.gz';
            }
        } 
        else if(platform == 'darwin') 
        {
            if(arch == 'x64') {
                odbc_driver = 'macos64_odbc_cli.tar.gz';
            } else if(arch == 'arm64') {
                odbc_driver = 'macarm64_odbc_cli.tar.gz';
            } else {
                console.log('Mac OS 32 bit not supported. Please use ' +
                            '64 bit MacOS system.\n');
                process.exit(1);
            }
        } 
        else if(platform == 'aix')
        {
            if(arch == 'ppc')
            {
                odbc_driver = 'aix32_odbc_cli.tar.gz';
            }
            else
            {
                odbc_driver = 'aix64_odbc_cli.tar.gz';
            }
        }
        else if(platform == 'os390')
        {
            // zOS ODBC driver is part of Db2 installation.  Users need to
            // specify IBM_DB_HOME environment variable to the Db2 datasets
            // to allow the installer to access the necessary header files and
            // sidedeck definitions to build the node binding.
            console.log('Please set the environment variable IBM_DB_HOME to the ' + 
                        'High Level Qualifier (HLQ) of your Db2 libraries.\n');
            process.exit(1);
        }
        else
        {
            odbc_driver = platform + arch + '_odbc_cli.tar.gz';
        }

        if(!odbc_driver) {
            console.log('Unable to determine driver download file. Exiting the ' +
                        'install process.\n');
            process.exit(1);
        }

        INSTALLER_FILE = path.resolve(DOWNLOAD_DIR, odbc_driver);
        installerfileURL = installerURL + odbc_driver;
        installerfileURL2 = githubURL + odbc_driver;

        license_agreement += path.resolve(DOWNLOAD_DIR, 'clidriver') + license_agreement2;
        printMsg(license_agreement);
        printMsg('INFO: Downloading DB2 ODBC CLI Driver from ' +
                 installerfileURL + ' ...\n');

        fs.stat(installerfileURL, function (err, stats) {
            if (!err && stats.isFile()) {
                // If IBM_DB_INSTALLER_URL is set and file exists.
                INSTALLER_FILE = installerfileURL;
                return copyAndExtractCliDriver();
            }
            return downloadCliDriver(installerfileURL);
        });

    }  // * END OF EXECUTION */

    function copyAndExtractCliDriver() {
        IBM_DB_HOME = path.resolve(DOWNLOAD_DIR, 'clidriver');
        // Delete existing clidriver before unzip/untar
        deleteFolderRecursive(IBM_DB_HOME);

        if(platform == 'win32') {
            var zip = new AdmZip(INSTALLER_FILE);

            // Using the "adm-zip" module to extract the zipped "clidriver".
            zip.extractAllToAsync(DOWNLOAD_DIR, false, function(err) {
                if (err) {
                    console.log(err);
                } else {
                    printMsg('\n\nDownloading and extraction of DB2 ODBC ' +
                             'CLI Driver completed successfully. \n');

                    process.env.IBM_DB_HOME = IBM_DB_HOME.replace(/\s/g,'\\ ');
                    buildBinary(true);
                    if(deleteInstallerFile) removeInstallerFile();
                }
            });
        }
        else 
        {
            var targz = require('targz');
            var compress = targz.decompress({src: INSTALLER_FILE, dest: DOWNLOAD_DIR}, function(err){
              if(err) {
                console.log(err);
                process.exit(1);
              }
              else {
                printMsg('\n\nDownloading and extraction of DB2 ODBC ' +
                         'CLI Driver completed successfully.\n');
                IBM_DB_HOME = path.resolve(DOWNLOAD_DIR, 'clidriver');
                process.env.IBM_DB_HOME = IBM_DB_HOME.replace(/\s/g,'\\ ');
                buildBinary(true);
                removeWinBuildArchive();
                if(deleteInstallerFile) removeInstallerFile();
              }
            });
        }
    }

    function buildBinary(isDownloaded) 
    {
        var buildString = "node-gyp configure build ";

        if(isDownloaded) {
            buildString = buildString + " --IS_DOWNLOADED=true";
        } else {
            buildString = buildString + " --IS_DOWNLOADED=false";
        }

        // Clean existing build directory
        if (platform != 'os390') {
            removeDir('build');
        }

        //Build triggered from the VSCode extension
        if(vscode_build){
            buildString = buildString + " --target=" + electron_version + " --arch=" + arch +
            " --dist-url=https://electronjs.org/headers/";
        }

        // Windows : Auto Installation Process -> 1) node-gyp then 2) msbuild.
        if( platform == 'win32' && arch == 'x64')
        {
            var buildString = buildString + " --IBM_DB_HOME=\$IBM_DB_HOME";
            var msbuild = "msbuild";

            var childProcess = exec(buildString, function (error, stdout, stderr)
            {
                if( downloadProgress == 0 ) {
                    var lines = stderr.toString().split('\n');
                    printMsg(stdout);
                    for( var line of lines) {
                        if( line.match(/MSBuild.exe/) ) {
                            msbuild = '"' + line.slice(15) + '"';
                            break;
                        }
                    }
                }

                if (error !== null)
                {
                    // "node-gyp" FAILED: RUN Pre-compiled Binary Installation process.
                    if(!downloadProgress) {
                      console.log(error);
                      printMsg('\nnode-gyp build process failed! \n\n' +
                        'Proceeding with Pre-compiled Binary Installation. \n');
                    }
                    installPreCompiledBinary();
                    return;
                }    

                else
                {
                    // "node-gyp" PASSED: RUN "msbuild" command.
                    var msbuildString = msbuild + " /clp:Verbosity=minimal /nologo /p:Configuration=Release;Platform=x64 ";
                    if (fs.existsSync('build/Debug')) {
                        msbuildString = msbuild + " /clp:Verbosity=minimal /nologo /p:Configuration=Debug;Platform=x64 ";
                    }

                    // getting the "binding.sln" (project solution) file path for "msbuild" command.
                    if (fs.existsSync(CURRENT_DIR + "/build/binding.sln"))
                    {
                        var BINDINGS_SLN_FILE = path.resolve(CURRENT_DIR, 'build/binding.sln');
                        msbuildString = msbuildString + '"' + BINDINGS_SLN_FILE + '"';
                    }
                    else
                    {
                        //If binding.sln file is missing then msbuild will fail.
                        console.log('\nbinding.sln file is not available! \n\n' +
                        'Proceeding with Pre-compiled Binary Installation. \n');
                        installPreCompiledBinary();
                        return;
                    }

                    /*
                     * EDITING: build/odbc_bindings.vcxproj file because,
                     * We need to remove "kernel" dependencies from the <AdditionalDependecy> tag.
                     * Otherwise "msbuild" command will produce corrupt binaries.
                     */
                    if (fs.existsSync(CURRENT_DIR + "/build/odbc_bindings.vcxproj"))
                    {
                        var ODBC_BINDINGS_VCXPROJ_FILE = path.resolve(CURRENT_DIR, 'build/odbc_bindings.vcxproj');
                        
                        fs.readFile(ODBC_BINDINGS_VCXPROJ_FILE, 'utf8', function (err,data) {
                            if (err)
                            {
                                console.log('\nReading failure: can not read ' +
                                'build/odbc_bindings.vcxproj! \n' +
                                'Proceeding with Pre-compiled Binary Installation.\n');
                                installPreCompiledBinary();
                                return;
                            }

                            //Removing kernel dependencies from the file.
                            var result = data.replace(/kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;DelayImp.lib/g, '');
                            
                            fs.writeFile(ODBC_BINDINGS_VCXPROJ_FILE, result, 'utf8', function (err) {
                                if (err)
                                {
                                    console.log('\nWriting failure: can not write ' + 'build/odbc_bindings.vcxproj! \n' +
                                    'Proceeding with Pre-compiled Binary Installation. \n');
                                    installPreCompiledBinary();
                                    return;
                                }
                                else printMsg("\nKernel additional dependencies removed successfully!\n");
                            });
                        });
                    }
                    else
                    {
                        /*
                         * IF: build/odbc_bindings.vcxproj file is missing,
                         * THEN: "msbuild" will produce corrupt binary (NO FAILURE), so to stop this:
                         * RUN: Pre-compiled Binary Installation process.
                         */
                        installPreCompiledBinary();
                        return;
                    }

                    removeDir('build/Release');
                    removeDir('build/Debug');

                    var childProcess = exec(msbuildString, function (error, stdout, stderr)
                    {
                        if (error !== null)
                        {
                            // "msbuild" FAILED: RUN Pre-compiled Binary Installation process.
                            printMsg(error);
                            printMsg('\nmsbuild build process failed! \n\n' +
                            'Proceeding with Pre-compiled Binary Installation. \n');
                            installPreCompiledBinary();
                            return;
                        }
                        else
                        {
                            printMsg("\nibm_db installed successfully.\n");
                        }
                    });
                }
            });
        }

        else
        {
            var buildString = buildString + " --IBM_DB_HOME=\"$IBM_DB_HOME\"";
            var nodever = parseInt(process.versions.node.split('.')[0], 10);
            var childProcess = exec(buildString, function (error, stdout, stderr) {
                if( downloadProgress == 0 ) printMsg(stdout);
                if (error !== null) {
                  if ((vscode_build || nodever > 15) &&
                      (platform == 'darwin' || platform == 'linux')) {
                    // "node-gyp" FAILED: RUN Pre-compiled Binary Installation.
                    if(!downloadProgress) {
                      console.log(error);
                      printMsg('\nnode-gyp build process failed! \n\n' +
                        'Proceeding with Pre-compiled Binary Installation. \n');
                    }
                    return installPreCompiledBinary();
                  } else {
                    console.log(error);
                    process.exit(1);
                  }
                }

                if(platform == 'darwin') {
                    // Run the install_name_tool
                    var addonBinary = "./build/Release/odbc_bindings.node";
                    if (!fs.existsSync(addonBinary)) {
                      addonBinary = "./build/Debug/odbc_bindings.node";
                    }
                    var nameToolCommand = "install_name_tool -change libdb2.dylib \"$IBM_DB_HOME/lib/libdb2.dylib\" " + addonBinary;
                    if( isDownloaded || vscode_build) // For issue #329
                    {
                      nameToolCommand = "install_name_tool -change libdb2.dylib @loader_path/../../installer/clidriver/lib/libdb2.dylib " + addonBinary;
                    }
                    var nameToolCmdProcess = exec(nameToolCommand , 
                    function (error1, stdout1, stderr1) {
                        if (error1 !== null) {
                            console.log('Error setting up the lib path to ' +
                            'odbc_bindings.node file.Error trace:\n'+error1);
                            process.exit(1);
                        }
                    });
                }
                printMsg("ibm_db installed successfully.");
            });
        }
    } //buildBinary

    function removeWinBuildArchive() 
    {
        var WIN_BUILD_FILE = path.resolve(CURRENT_DIR, 'build.zip');
        fs.exists(WIN_BUILD_FILE, function(exists) 
        {
            if (exists) 
            {
                fs.unlinkSync(WIN_BUILD_FILE);
            }
        });
    }

    function removeInstallerFile()
    {
        // Delete downloaded odbc_cli.tar.gz file.
        fs.exists(INSTALLER_FILE, function(exists) 
        {
            if (exists) 
            {
                fs.unlinkSync(INSTALLER_FILE);
            }
        });
    }

    function installPreCompiledBinary()
    {
        if(arch.indexOf("32") > 0) {
            console.log('32 bit platform is not supported. Please ' +
                    'install 64 bit NodeJS on 64 bit OS for ibm_db.\n');
            process.exit(1);
        } else if(arch.indexOf("ppc") > 0 || arch.indexOf("s390") > 0) {
            console.log('There is no precompiled binary for platform = ' +
                platform + ', architecture = ' + arch + '\n');

        } else if(platform == 'win32' || vscode_build) {
            var BUILD_FILE = path.resolve(CURRENT_DIR, 'build.zip');
            var odbcBindingsNode;
            var fileName;
            var ODBC_BINDINGS = 'build\/Release\/odbc_bindings.node';

            if(vscode_build)
            {
                var electronVersion = (electron_version).split('.');
                if (platform == 'darwin') {
                  if (arch == 'arm64') {
                    fileName = "_macarm_" + electronVersion[0];
                  } else {
                    fileName = "_mac_" + electronVersion[0];
                  }
                } else if (platform == 'win32') {
                  fileName = "_win_" + electronVersion[0];
                } else {
                  fileName = "_linux_" + electronVersion[0];
                }
                odbcBindingsNode = 'build\/Release\/odbc_bindings' + fileName + '.node';
                if(electronVersion[0] < 32) {
                    console.log("No precompiled electron binary available"+
                                " for electron " + electron_version + "\n");
                    process.exit(1);
                }
            }
            else
            {
                //Windows node binary names should update here.
                var ODBC_BINDINGS_V14 = 'build\/Release\/odbc_bindings.node.14.21.3';
                var ODBC_BINDINGS_V15 = 'build\/Release\/odbc_bindings.node.15.14.0';
                var ODBC_BINDINGS_V16 = 'build\/Release\/odbc_bindings.node.16.20.2';
                var ODBC_BINDINGS_V17 = 'build\/Release\/odbc_bindings.node.17.9.1';
                var ODBC_BINDINGS_V18 = 'build\/Release\/odbc_bindings.node.18.20.8';
                var ODBC_BINDINGS_V19 = 'build\/Release\/odbc_bindings.node.19.9.0';
                var ODBC_BINDINGS_V20 = 'build\/Release\/odbc_bindings.node.20.19.5';
                var ODBC_BINDINGS_V21 = 'build\/Release\/odbc_bindings.node.21.7.3';
                var ODBC_BINDINGS_V22 = 'build\/Release\/odbc_bindings.node.22.20.0';
                var ODBC_BINDINGS_V23 = 'build\/Release\/odbc_bindings.node.23.11.1';

                // Windows add-on binary for node.js v0.10.x, v0.12.7, 4.x, 6.x to 14.x has been discontinued.
                if(Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 14.0) {
                    console.log('\nERROR: Did not find precompiled add-on binary for node.js version ' + process.version + ':' +
                        '\nibm_db does not provide precompiled add-on binary for node.js version ' + process.version +
                ' on Windows platform. Visual Studio is required to compile ibm_db with node.js versions < 14.X. ' +
                        'Otherwise please use the node.js version >= 14.X\n');
                    process.exit(1);
                }

                /*
                 * odbcBindingsNode will consist of the node binary-
                 * file name according to the node version in the system.
                 */
                odbcBindingsNode =
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 15.0) && ODBC_BINDINGS_V14 ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 16.0) && ODBC_BINDINGS_V15 ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 17.0) && ODBC_BINDINGS_V16 ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 18.0) && ODBC_BINDINGS_V17 ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 19.0) && ODBC_BINDINGS_V18 ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 20.0) && ODBC_BINDINGS_V19 ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 21.0) && ODBC_BINDINGS_V20 ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 22.0) && ODBC_BINDINGS_V21 ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 23.0) && ODBC_BINDINGS_V22 ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 24.0) && ODBC_BINDINGS_V23 ||
                                   ODBC_BINDINGS;
            }
            // We have correct bindings file in odbcBindingsNode for
            // installed node version now. Extract it from build.zip file.
            printMsg("Extracting " + odbcBindingsNode + " from build.zip");

            // Removing the "build" directory created by Auto Installation Process.
            // "adm-zip" will create a fresh "build" directory for extraction of "build.zip".
            removeDir('build');
            var zip = new AdmZip(BUILD_FILE);

            /*
             * adm-zip will parse the build.zip file content and
             * then it will check for the odbcBindingsNode
             * (node Binary), when it gets that binary file,
             * zip.extractEntryTo will write the same node binary
             * but the name will be odbc_bindings.node, and the other
             * binary files and build.zip will be discarded.
             */
            zip.forEach(function(entry) {
                if (entry.entryName === odbcBindingsNode) {
                    try {
                        zip.extractEntryTo(entry, CURRENT_DIR, true, false, false, ODBC_BINDINGS);
                    } catch (e) {
                        installationFailed(e);
                    }

                    printMsg("\n" +
                    "===================================\n"+
                    "ibm_db installed successfully.\n"+
                    "===================================\n");
                }
            });

            return 1;
        } else if(platform == 'linux' || platform == 'darwin') {
            // let binaryUrl = 'https://github.com/ibmdb/ibmdb-binaries/raw/refs/heads/main/';
            let binaryUrl = 'https://raw.githubusercontent.com/ibmdb/ibmdb-binaries/refs/heads/main/';
            let osName;
            const nodeVersion = process.versions.node.split('.')[0];
            const ODBC_BINDINGS = 'build\/Release\/odbc_bindings.node';
            const outputFile = path.resolve(CURRENT_DIR, ODBC_BINDINGS);

            if (platform == 'darwin') {
                if (arch == 'arm64') {
                    osName = "macarm64";
                } else {
                    osName = "macx64";
                }
            } else {
                osName = "linuxx64";
            }
            binaryUrl = binaryUrl + osName + '/odbc_bindings.node.' + nodeVersion;

            // Parse URL to use with https.request
            const { hostname, pathname } = new URL(binaryUrl);

            const options = {
              hostname,
              path: pathname,
              method: 'GET',
              agent: httpsAgent,
              headers: {
                'User-Agent': 'Node.js HTTPS Client'
              }
            };

            const req = https.request(options, (res) => {
              if (res.statusCode !== 200) {
                console.error(`Download failed: HTTP ${res.statusCode}`);
                res.resume();
                installationFailed();
              }

              const total_bytes = parseInt(res.headers['content-length'], 10);
              const outStream = fs.createWriteStream(outputFile);
              let received_bytes = 0;

              res.on('data', (chunk) => {
                  received_bytes += chunk.length;
                  showDownloadingProgress(received_bytes, total_bytes);
              });

              res.pipe(outStream);

              outStream.on('finish', () => {
                  printMsg('\n✅ Download complete:' + outputFile);
                  printMsg("\n" +
                  "===================================\n"+
                  "ibm_db installed successfully.\n"+
                  "===================================\n");
              });

              outStream.on('error', (err) => {
                  console.error('\n❌ File write error:', err.message);
                  installationFailed();
              });
            });

            req.on('error', (err) => {
                console.error('\n❌ HTTPS request error:', err.message);
                installationFailed();
            });

            req.end();
        }
    }

    // Function to download clidriver file using axios module.
    function downloadCliDriver(installerfileURL) {
        // Variable to save downloading progress
        var received_bytes = 0;
        var total_bytes = 0;

        var outStream = fs.createWriteStream(INSTALLER_FILE);

        axios.get(installerfileURL, {responseType: 'stream', httpsAgent})
             .then(function (response) {
                total_bytes = parseInt(response.headers['content-length']);
                response.data.on('data', (chunk) => {
                    received_bytes += chunk.length;
                    showDownloadingProgress(received_bytes, total_bytes);
                });
                response.data.pipe(outStream);
              })
             .catch(error => {
                if( error.code ) {
                  printMsg(error.code + " : " + installerfileURL);
                }
                if( error.response ) {
                  printMsg('Error ' + error.response.status + ': ' +
                      error.response.statusText + ' - ' + installerfileURL);
                }
                // Try download from github as download from IBM site failed.
                if(!gitDownload) {
                  gitDownload = true;
                  printMsg('Downloading DB2 ODBC CLI Driver from ' +
                           installerfileURL2 + ' ...\n');
                  downloadCliDriver(installerfileURL2);
                } else {
                  if(!error.code && !error.response) printMsg(error);
                  console.log('\n=====================================');
                  console.log('Error: Installation of ibm_db failed.');
                  console.log('=====================================\n');
                }
              });

        deleteInstallerFile = true;
        outStream.once('close', copyAndExtractCliDriver)
        .once('error', function (err) {
            console.log(err);
        });
    }

    function showDownloadingProgress(received, total) {
        var percentage = ((received * 100) / total).toFixed(2);
        if(downloadProgress > 0) {
          if(percentage > 0 && downloadProgress == 1) {
            printProgress(percentage, received, total);
            downloadProgress++;
          } else if(percentage > 50 && downloadProgress == 2) {
            printProgress(percentage, received, total);
            downloadProgress++;
          } else if(percentage == 100 && downloadProgress == 3) {
            printProgress(percentage, received, total);
            downloadProgress++;
          }
        } else {
            printProgress(percentage, received, total);
        }
    }

    function printProgress(percentage, received, total) {
        process.stdout.write((platform == 'win32') ? "\033[0G": "\r");
        process.stdout.write(percentage + "% | " + received + " bytes downloaded out of " + total + " bytes.");
    }

    function removeDir(dir) {
        var fullPath = path.resolve(CURRENT_DIR, dir);
        if (fs.existsSync(fullPath)) {
          if(platform == 'win32') {
            execSync( "rmdir /s /q " + '"' + fullPath + '"' );
          } else {
            execSync( "rm -rf " + '"' + fullPath + '"' );
          }
        }
    }

    function deleteFolderRecursive(p){
        if (fs.existsSync(p)) {
            fs.readdirSync(p).forEach(function(file, index){
                var curPath = path.join(p, file);
                if (fs.lstatSync(curPath).isDirectory()) { // recurse
                    deleteFolderRecursive(curPath);
                }else { // delete file
                    fs.unlinkSync(curPath);
                }
            });
            fs.rmdirSync(p);
        }
    }

}; //install_node_ibm_db

install_node_ibm_db();

function printMakeVersion() {
  if (platform != 'win32') {
    try {
      var makeVersion = execSync('make -v').toString();
      makeVersion = makeVersion.split('\n')[0];
      if( downloadProgress == 0 ) printMsg("INFO: make version = " + makeVersion);
    } catch (e) {
      printMsg("Unable to find 'make' in PATH. Installation may fail!");
    }
  }
}

function installationFailed(msg) {
    console.log('ERROR: Installation Failed!');
    if (msg) console.log(msg);
    process.exit(1);
}

/* Detect electron version to compile ibm_db by checking version of installed
   electron package, or version of installed VSCode in the system.
 */
function findElectronVersion() {
  if ((process.env.npm_config_vscode) || (process.env.npm_config_electron) ||
      (process.env.npm_package_config_vscode) ||
      (process.env.npm_package_config_electron) ||
      (process.env.ELECTRON) ||
      (__dirname.toLowerCase().indexOf('db2connect') != -1))
  {
    printMsg('\nProceeding to build IBM_DB for Electron framework...\n');
    vscode_build = true;
    var electronVer = null;

    try {
        if(process.env.npm_config_electron && process.env.npm_config_electron != true) {
          electronVer = process.env.npm_config_electron;
        }
        else if(process.env.npm_package_config_electron &&
                process.env.npm_package_config_electron != true) {
          electronVer = process.env.npm_package_config_electron;
        }
        else if(process.versions.electron) {
          electronVer = process.versions.electron;
        } else {
          var npmOut = execSync('npm ls electron').toString();
          if (npmOut != null) {
            npmOut = npmOut.split('\n');
            for (var i = 0; i < npmOut.length; i++) {
              if (npmOut[i].indexOf('-- electron@') >= 0) {
                electronVer = npmOut[i].split('@')[1];
                break;
              }
            }
          }
        }
    } catch (e) {
        printMsg("Unable to detect electon installation.");
    }
    if (process.env.ELECTRON && process.env.ELECTRON != "true") {
        electronVer = process.env.ELECTRON;
    }
    if (electronVer != null) {
        electron_version = electronVer;
        printMsg("Found electron version, will use Electron version " +
                 electron_version + " to install ibm_db.");
    } else {
        try {
          var codeOut = execSync('code --version').toString();
          vscodeVer = parseFloat(codeOut.split('\n')[0]);
          if(!isNaN(vscodeVer)) {
            if (vscodeVer >= 1.104){
                electron_version = "37.3.1";
            }
            else if (vscodeVer >= 1.103){
                electron_version = "37.2.3";
            }
            else if (vscodeVer >= 1.101){
                electron_version = "35.6.0";
            }
            else if (vscodeVer >= 1.100){
                electron_version = "34.5.1";
            }
            else {// vscode version older than 1.100 not supported
                electron_version = "34.5.1";
                printMsg("VSCode version " + vscodeVer + " is too old!");
            }
            printMsg("Detected VSCode version" + vscodeVer +
                    ", will use Electron version " + electron_version);
          }
		  else {
            printMsg("Unable to detect VSCode version," +
                    "will use Electron version " + electron_version);
          }
        }
        catch(e){
            printMsg("Unable to find VSCode version," +
                    "will use Electron version " + electron_version);
        }
    }
    printMsg("");
  }
}

function printMsg(msg) {
    if(!silentInstallation) {
        console.log(msg);
    }
}


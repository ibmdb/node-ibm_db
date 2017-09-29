/**
 * Node-ibm_db Installer file.
 */

var fs = require('fs');
var url = require('url');
var os = require('os');
var path = require('path');
var exec = require('child_process').exec;
var execSync = require('child_process').execSync;
var request = require('request');

//IBM provided URL for downloading clidriver.
var installerURL = 'https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli';
var license_agreement = '\n\n****************************************\nYou are downloading a package which includes the Node.js module for IBM DB2/Informix.  The module is licensed under the Apache License 2.0. The package also includes IBM ODBC and CLI Driver from IBM, which is automatically downloaded as the node module is installed on your system/device. The license agreement to the IBM ODBC and CLI Driver is available in '+DOWNLOAD_DIR+'   Check for additional dependencies, which may come with their own license agreement(s). Your use of the components of the package and dependencies constitutes your acceptance of their respective license agreements. If you do not accept the terms of any license agreement(s), then delete the relevant component(s) from your device.\n****************************************\n';

var CURRENT_DIR = process.cwd();
var DOWNLOAD_DIR = path.resolve(CURRENT_DIR, 'installer');
var INSTALLER_FILE; 
var deleteInstallerFile = false;

/*
 * "process.env.IBM_DB_INSTALLER_URL"
 * USE: to by-pass the IBM provided URL for downloading clidriver.
 * HOW: set environment variable with alternate downloading URL link.
 *      or locally downloaded "tar/zipped clidriver's" parent directory path.
 */
installerURL = process.env.IBM_DB_INSTALLER_URL || installerURL;
installerURL = installerURL + "/";

//Function to download clidriver and install node-ibm_db
var install_node_ibm_db = function(file_url) {
    var readStream;
    var writeStream;
    var platform = os.platform();
    var arch = os.arch();
    var endian = os.endianness();
    var installerfileURL;

    var fstream = require('fstream');
    var unzipper = require('unzipper');

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
    if(process.env.IBM_DB_HOME || fs.existsSync(DOWNLOAD_DIR + "/clidriver")) 
    {
        var IS_ENVIRONMENT_VAR;
        if(process.env.IBM_DB_HOME){
            IBM_DB_HOME = process.env.IBM_DB_HOME;
            IS_ENVIRONMENT_VAR = true;
        }
        else if (fs.existsSync(DOWNLOAD_DIR + "/clidriver")){
            IBM_DB_HOME = path.resolve(DOWNLOAD_DIR, 'clidriver');
            process.env.IBM_DB_HOME = IBM_DB_HOME.replace(/\s/g,'\\ ');
            IS_ENVIRONMENT_VAR = false;
        }

        IBM_DB_INCLUDE = path.resolve(IBM_DB_HOME, 'include');
        if (fs.existsSync(IBM_DB_HOME + "/lib64")) {
           IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib64');
        } else if (fs.existsSync(IBM_DB_HOME + "/lib32")) {
           IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib32');
        } else {
           IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib');
        }

        if(IS_ENVIRONMENT_VAR){
            console.log('IBM_DB_HOME environment variable have already been ' +
            'set to -> ' + IBM_DB_HOME +
            '\n\nDownloading of clidriver skipped - build is in progress...\n');
        }else{
            console.log('Rebuild Process: Found clidriver at -> '+ IBM_DB_HOME +
            '\n\nDownloading of clidriver skipped - build is in progress...\n');
        }

        if (!fs.existsSync(IBM_DB_HOME)) {
            console.log(IBM_DB_HOME + ' directory does not exist. Please check if you have ' + 
                        'set the IBM_DB_HOME environment variable\'s value correctly.\n');
        }

        if(!(platform == 'win32' && IS_ENVIRONMENT_VAR == false)){
            if (!fs.existsSync(IBM_DB_INCLUDE)) {
                console.log(IBM_DB_INCLUDE + ' directory does not exist. Please check if you have ' + 
                        'set the IBM_DB_HOME environment variable\'s value correctly.\n');
            }
        }

        if (!fs.existsSync(IBM_DB_LIB)) {
            console.log(IBM_DB_LIB + ' directory does not exist. Please check if you have ' + 
                        'set the IBM_DB_HOME environment variable\'s value correctly.\n');
        }
        if( platform != 'win32') {
            if(!fs.existsSync(IBM_DB_HOME + "/lib"))
                fs.symlinkSync(IBM_DB_LIB, path.resolve(IBM_DB_HOME, 'lib'));

            if((platform == 'linux') || (platform =='aix') || 
               (platform == 'darwin' && arch == 'x64')) {
                removeWinBuildArchive();
                buildBinary(!IS_ENVIRONMENT_VAR);
            }
        }
        else if(platform == 'win32' && arch == 'x64') {
            buildBinary(!IS_ENVIRONMENT_VAR);
        }
        else {
            console.log('Building binaries for node-ibm_db. This platform ' +
            'is not completely supported, you might encounter errors. ' +
            'In such cases please open an issue on our repository, ' +
            'https://github.com/ibmdb/node-ibm_db. \n');
        }
    }
    else
    {
        if(platform == 'win32') {
            if(arch == 'x64') {
                installerfileURL = installerURL + 'ntx64_odbc_cli.zip';
            }
        } 
        else if(platform == 'linux') 
        {
            if(arch == 'x64') {
                installerfileURL = installerURL + 'linuxx64_odbc_cli.tar.gz';
            } else if(arch == 's390x') {
                installerfileURL = installerURL + 's390x64_odbc_cli.tar.gz';
            } else if(arch == 's390') {
                installerfileURL = installerURL + 's390_odbc_cli.tar.gz';
            } else if(arch == 'ppc64') {
                if(endian == 'LE')
                    installerfileURL = installerURL + 'ppc64le_odbc_cli.tar.gz';
                else
                    installerfileURL = installerURL + 'ppc64_odbc_cli.tar.gz';
            } else if(arch == 'ppc32') {
                installerfileURL = installerURL + 'ppc32_odbc_cli.tar.gz';
            } else {
                installerfileURL = installerURL + 'linuxia32_odbc_cli.tar.gz';
            }
        } 
        else if(platform == 'darwin') 
        {
            if(arch == 'x64') {
                installerfileURL = installerURL + 'macos64_odbc_cli.tar.gz';
            } else {
                console.log('Mac OS 32 bit not supported. Please use an ' +
                            'x64 architecture.\n');
                return;
            }
        } 
        else if(platform == 'aix')
        {
            if(arch == 'ppc')
            {
                installerfileURL = installerURL + 'aix32_odbc_cli.tar.gz';
            }
            else
            {
                installerfileURL = installerURL + 'aix64_odbc_cli.tar.gz';
            }
        }
        else 
        {
            installerfileURL = installerURL + platform + arch + 
                               '_odbc_cli.tar.gz';
        }

        if(!installerfileURL) {
            console.log('Unable to fetch driver download file. Exiting the ' +
                        'install process.\n');
            process.exit(1);
        }

        var file_name = url.parse(installerfileURL).pathname.split('/').pop();
        INSTALLER_FILE = path.resolve(DOWNLOAD_DIR, file_name);

        console.log('Downloading DB2 ODBC CLI Driver from ' +
                    installerfileURL+'...\n');

        fs.stat(installerfileURL, function (err, stats) {
            if (!err && stats.isFile()) {
                INSTALLER_FILE = installerfileURL;
                return copyAndExtractCliDriver();
            }
            return downloadCliDriver(installerfileURL);
        });

    }  // * END OF EXECUTION */

    function copyAndExtractCliDriver() {
        if(platform == 'win32') {
            readStream = fs.createReadStream(INSTALLER_FILE);

            // Using the "unzipper" module to extract the zipped "clidriver",
            // and on successful close, printing the license_agreement
            var extractCLIDriver = readStream.pipe(unzipper.Extract({path: DOWNLOAD_DIR}));

            extractCLIDriver.on('close', function() {
                console.log(license_agreement);
                console.log('Downloading and extraction of DB2 ODBC ' +
                    'CLI Driver completed successfully... \n');

                IBM_DB_HOME = path.resolve(DOWNLOAD_DIR, 'clidriver');
                process.env.IBM_DB_HOME = IBM_DB_HOME.replace(/\s/g,'\\ ');
                buildBinary(true);
                if(deleteInstallerFile) removeInstallerFile();
            });

            extractCLIDriver.on('err', function() {
                console.log(err);
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
                console.log(license_agreement);
                console.log('Downloading and extraction of DB2 ODBC ' +
                            'CLI Driver completed successfully ...\n');
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

        // Windows : Auto Installation Process -> 1) node-gyp then 2) msbuild.
        if( platform == 'win32' && arch == 'x64')
        {
            var buildString = buildString + " --IBM_DB_HOME=\$IBM_DB_HOME";

            var childProcess = exec(buildString, function (error, stdout, stderr)
            {
                console.log(stdout);

                if (error !== null)
                {
                    // "node-gyp" FAILED: RUN Pre-compiled Binary Installation process.
                    console.log(error);
                    console.log('\nnode-gyp build process failed! \n' +
                    'Proceeding with Pre-compiled Binary Installation. \n');
                    installPreCompiledWinBinary();
                    return;
                }    

                else
                {
                    // "node-gyp" PASSED: RUN "msbuild" command.
                    var msbuildString = "msbuild /clp:Verbosity=minimal /nologo /p:Configuration=Release;Platform=x64 ";

                    // getting the "binding.sln" (project solution) file path for "msbuild" command.
                    if (fs.existsSync(CURRENT_DIR + "/build/binding.sln"))
                    {
                        var BINDINGS_SLN_FILE = path.resolve(CURRENT_DIR, 'build/binding.sln');
                        msbuildString = msbuildString + BINDINGS_SLN_FILE;
                    }
                    else
                    {
                        //If binding.sln file is missing then msbuild will fail.
                        console.log('\nbinding.sln file is not available! \n' +
                        'Proceeding with Pre-compiled Binary Installation. \n');
                        installPreCompiledWinBinary();
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
                                installPreCompiledWinBinary();
                                return;
                            }

                            //Removing kernel dependencies from the file.
                            var result = data.replace(/kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;DelayImp.lib/g, '');
                            
                            fs.writeFile(ODBC_BINDINGS_VCXPROJ_FILE, result, 'utf8', function (err) {
                                if (err)
                                {
                                    console.log('\nWriting failure: can not write ' + 'build/odbc_bindings.vcxproj! \n' +
                                    'Proceeding with Pre-compiled Binary Installation. \n');
                                    installPreCompiledWinBinary();
                                    return;
                                }
                                else console.log("\nKernel additional dependencies removed successfully!\n");
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
                        installPreCompiledWinBinary();
                        return;
                    }

                    if (fs.existsSync(CURRENT_DIR + "/build/Release"))
                    {
                        var RELEASE_DIRECTORY = path.resolve(CURRENT_DIR, 'build/Release');
                        execSync("rmdir /s /q " + RELEASE_DIRECTORY);
                    }

                    var childProcess = exec(msbuildString, function (error, stdout, stderr)
                    {
                        console.log(stdout);
                        if (error !== null)
                        {
                            // "msbuild" FAILED: RUN Pre-compiled Binary Installation process.
                            console.log(error);
                            console.log('\nmsbuild build process failed! \n' +
                            'Proceeding with Pre-compiled Binary Installation. \n');
                            installPreCompiledWinBinary();
                            return;
                        }
                        else
                        {
                            console.log("\nnode-ibm_db installed successfully!\n");
                        }
                    });
                }
            });
        }

        else
        {
            var buildString = buildString + " --IBM_DB_HOME=\"$IBM_DB_HOME\"";
            var childProcess = exec(buildString, function (error, stdout, stderr) {
                console.log(stdout);
                if (error !== null) {
                    console.log(error);
                    process.exit(1);
                }

                if(platform == 'darwin' && arch == 'x64') {
                    // Run the install_name_tool
                    var nameToolCommand = "install_name_tool -change libdb2.dylib \"$IBM_DB_HOME/lib/libdb2.dylib\" ./build/Release/odbc_bindings.node" ;
                    var nameToolCmdProcess = exec(nameToolCommand , 
                    function (error1, stdout1, stderr1) {
                        if (error1 !== null) {
                            console.log('Error setting up the lib path to ' +
                            'odbc_bindings.node file.Error trace:\n'+error1);
                            process.exit(1);
                        }
                    });
                }
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

    function installPreCompiledWinBinary()
    {
        if(platform == 'win32') {
            if(arch == 'x64') {
                var BUILD_FILE = path.resolve(CURRENT_DIR, 'build.zip');

                //Windows node binary names should update here.
                var ODBC_BINDINGS = 'build\/Release\/odbc_bindings.node';
                var ODBC_BINDINGS_V12 = 'build\/Release\/odbc_bindings.node.0.12.7';
                var ODBC_BINDINGS_V4 = 'build\/Release\/odbc_bindings.node.4.6.1';
                var ODBC_BINDINGS_V6 = 'build\/Release\/odbc_bindings.node.6.9.1';
                var ODBC_BINDINGS_V7 = 'build\/Release\/odbc_bindings.node.7.4.0';

                // Windows add-on binary for node.js v0.10.x and v0.12.7 has been discontinued.
                if(Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 0.12) {
                    console.log('\nERROR: Found unsupported node.js version ' + process.version + ':' +
                        '\nnode-ibm_db do not have precompiled add-on file "odbc_bindings.node" for\n' +
                        'node.js ' + process.version + ' on Widnows. Please use the latest version of node.js.\n');
                    process.exit(1);
                }

                /*
                 * odbcBindingsNode will consist of the node binary-
                 * file name according to the node version in the system.
                 */
                var odbcBindingsNode = (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 4.0) && ODBC_BINDINGS_V12   ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 5.0) && ODBC_BINDINGS_V4   ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 7.0) && ODBC_BINDINGS_V6   ||
                                   (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 8.0) && ODBC_BINDINGS_V7   || ODBC_BINDINGS ;

                // Removing the "build" directory created by Auto Installation Process.
                // "unzipper" will create a fresh "build" directory for extraction of "build.zip".
                if (fs.existsSync(CURRENT_DIR + "/build")) {
                    var BUILD_DIRECTORY = path.resolve(CURRENT_DIR, 'build');
                    execSync("rmdir /s /q " + BUILD_DIRECTORY);
                }

                readStream = fs.createReadStream(BUILD_FILE);

                /*
                 * unzipper will parse the build.zip file content and
                 * then it will check for the odbcBindingsNode
                 * (node Binary), when it gets that binary file,
                 * fstream.Writer will write the same node binary
                 * but the name will be odbc_bindings.node, and the other
                 * binary files and build.zip will be discarded.
                 */
                readStream.pipe(unzipper.Parse())
                    .on('entry', function (entry) {
                        if(entry.path === odbcBindingsNode) {
                            entry.pipe(fstream.Writer(ODBC_BINDINGS));
                        } else {
                            entry.autodrain();
                        }
                    })
                    .on('error', function(e) {
                        console.log('Installation Failed! \n',e);
                        process.exit(1);
                    })
                    .on('finish', function() {
                      console.log("\n" + 
                      "===================================\n"+
                      "node-ibm_db installed successfully!\n"+
                      "===================================\n");
                    });

                return 1;

            } else {
                console.log('Windows 32 bit not supported. Please use an ' +
                        'x64 architecture.\n');
                process.exit(1);
            }
        }
    }

    // Function to download clidriver file using request module.
    function downloadCliDriver(installerfileURL) {
        // Variable to save downloading progress
        var received_bytes = 0;
        var total_bytes = 0;

        var outStream = fs.createWriteStream(INSTALLER_FILE);

        request
            .get(installerfileURL)
                .on('error', function(err) {
                    console.log(err);
                })
                .on('response', function(data) {
                    total_bytes = parseInt(data.headers['content-length']);
                })
                .on('data', function(chunk) {
                    received_bytes += chunk.length;
                    showDownloadingProgress(received_bytes, total_bytes);
                })
                .pipe(outStream);

        deleteInstallerFile = true;
        outStream.once('close', copyAndExtractCliDriver)
        .once('error', function (err) {
            console.log(err);
        });
    };

    function showDownloadingProgress(received, total) {
        var percentage = ((received * 100) / total).toFixed(2);
        process.stdout.write((platform == 'win32') ? "\033[0G": "\r");
        process.stdout.write(percentage + "% | " + received + " bytes downloaded out of " + total + " bytes.");
    }

}; //install_node_ibm_db

install_node_ibm_db();

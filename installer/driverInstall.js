var fs = require('fs');
var url = require('url');
var request = require('request');
var os = require('os');
var path = require('path');
var exec = require('child_process').exec;

var license_agreement = '\n****************************************\nYou are downloading a package which includes the Node.js module for IBM DB2/Informix.  The module is licensed under the Apache License 2.0. The package also includes IBM ODBC and CLI Driver from IBM, which is automatically downloaded as the node module is installed on your system/device. The license agreement to the IBM ODBC and CLI Driver is available in '+DOWNLOAD_DIR+'   Check for additional dependencies, which may come with their own license agreement(s). Your use of the components of the package and dependencies constitutes your acceptance of their respective license agreements. If you do not accept the terms of any license agreement(s), then delete the relevant component(s) from your device.\n****************************************\n';
var installerURL = 'http://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/';
var CURRENT_DIR = process.cwd();
var DOWNLOAD_DIR = path.resolve(CURRENT_DIR, 'installer');
var INSTALLER_FILE, IBM_DB_HOME, IBM_DB_INCLUDE, IBM_DB_LIB, IBM_DB_DIR;
installerURL = process.env.IBM_DB_INSTALLER_URL || installerURL;
installerURL = installerURL + "/";

var platform = os.platform();
var arch = os.arch();
var endian = os.endianness();
var installerfileURL;

if (arch !== 'x64' && (platform === 'darwin' || platform === 'win32')) {
    console.log('32bit', (platform === 'darwin' ? 'Mac OS' : 'Windows'), 'not supported, exiting...');
    process.exit(1);
}

var fstream = require('fstream');
var unzip = require('unzip');

if (platform === 'win32') {
    var BUILD_FILE = path.resolve(CURRENT_DIR, 'build.zip');

    fs.createReadStream(BUILD_FILE)
      .pipe(unzip.Parse())
      .pipe(fstream.Writer(CURRENT_DIR)).on("unpipe", function () {
        fs.unlinkSync(BUILD_FILE);
        var ODBC_BINDINGS = path.resolve(CURRENT_DIR, 'build\\Release\\odbc_bindings.node');
        var ODBC_BINDINGS_V10 = path.resolve(CURRENT_DIR, 'build\\Release\\odbc_bindings.node.0.10.36');
        var ODBC_BINDINGS_V12 = path.resolve(CURRENT_DIR, 'build\\Release\\odbc_bindings.node.0.12.7');
        var ODBC_BINDINGS_V4 = path.resolve(CURRENT_DIR, 'build\\Release\\odbc_bindings.node.4.4.2');
        if (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 0.12) {
            fs.renameSync(ODBC_BINDINGS_V10, ODBC_BINDINGS);
            fs.unlinkSync(ODBC_BINDINGS_V12);
            fs.unlinkSync(ODBC_BINDINGS_V4);
        } else if (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 4.0) {
            fs.renameSync(ODBC_BINDINGS_V12, ODBC_BINDINGS);
            fs.unlinkSync(ODBC_BINDINGS_V10);
            fs.unlinkSync(ODBC_BINDINGS_V4);
        } else if (Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 5.0) {
            fs.renameSync(ODBC_BINDINGS_V4, ODBC_BINDINGS);
            fs.unlinkSync(ODBC_BINDINGS_V10);
            fs.unlinkSync(ODBC_BINDINGS_V12);
        } else {
            fs.unlinkSync(ODBC_BINDINGS_V10);
            fs.unlinkSync(ODBC_BINDINGS_V12);
            fs.unlinkSync(ODBC_BINDINGS_V4);
        }
    });
}

if (process.env.IBM_DB_HOME) {
    IBM_DB_HOME = process.env.IBM_DB_HOME;
    IBM_DB_INCLUDE = path.resolve(IBM_DB_HOME, 'include');
    if (fs.existsSync(IBM_DB_HOME + "/lib64")) {
       IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib64');
    } else if (fs.existsSync(IBM_DB_HOME + "/lib32")) {
       IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib32');
    } else {
       IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib');
    }
    console.log('IBM_DB_HOME environment variable have already been set to', IBM_DB_HOME);

    [IBM_DB_HOME, IBM_DB_INCLUDE, IBM_DB_LIB].forEach(function (dir) {
        if (!fs.existsSync(dir)) {
            console.log(dir, 'directory does not exist. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
        }
    });

    if (platform !== 'win32') {
        if (!fs.existsSync(IBM_DB_HOME + "/lib")) {
            fs.symlinkSync(IBM_DB_LIB, path.resolve(IBM_DB_HOME, 'lib'));
        }
        if (platform === 'linux' || platform === 'aix' || platform === 'darwin') {
            removeWinBuildArchive();
            buildBinary(false);
        } else {
            console.log('Building binaries for node-ibm_db. This platform is not completely supported, you might encounter errors. In such cases please open an issue on our repository, https://github.com/ibmdb/node-ibm_db.');
            buildBinary(false);
        }
    }
} else {
    if (platform === 'win32') {
        installerfileURL = installerURL + 'ntx64_odbc_cli.zip';
    } else if (platform === 'darwin') {
        installerfileURL = installerURL + 'macos64_odbc_cli.tar.gz';
    } else if (platform === 'linux') {
        if (arch === 'x64') {
            installerfileURL = installerURL + 'linuxx64_odbc_cli.tar.gz';
        } else if (arch === 's390x') {
            installerfileURL = installerURL + 's390x64_odbc_cli.tar.gz';
        } else if (arch === 's390') {
            installerfileURL = installerURL + 's390_odbc_cli.tar.gz';
        } else if (arch === 'ppc64') {
            if (endian === 'LE') installerfileURL = installerURL + 'ppc64le_odbc_cli.tar.gz';
            else installerfileURL = installerURL + 'ppc64_odbc_cli.tar.gz';
        } else if (arch === 'ppc32') {
            installerfileURL = installerURL + 'ppc32_odbc_cli.tar.gz';
        } else {
            installerfileURL = installerURL + 'linuxia32_odbc_cli.tar.gz';
        }
    } else if (platform === 'aix') {
        if (arch === 'ppc') {
            installerfileURL = installerURL + 'aix32_odbc_cli.tar.gz';
        } else {
            installerfileURL = installerURL + 'aix64_odbc_cli.tar.gz';
        }
    } else {
        installerfileURL = installerURL + platform + arch + '_odbc_cli.tar.gz';
    }

    var file_name = url.parse(installerfileURL).pathname.split('/').pop();
    INSTALLER_FILE = path.resolve(DOWNLOAD_DIR, file_name);

    console.log('Downloading DB2 ODBC CLI Driver from ' +
                installerfileURL+'...');

    fs.stat(installerfileURL, function (err, stats) {
        if (!err && stats.isFile()) {
            INSTALLER_FILE = installerfileURL;
            return copyAndExtractDriver();
        }
        return getInstallerFile(installerfileURL);
    });

}

function copyAndExtractDriver() {
    if (platform === 'win32') {
        fs.createReadStream(INSTALLER_FILE).pipe(unzip.Parse()).pipe(fstream.Writer(DOWNLOAD_DIR));
        console.log('Download and extraction of DB2 ODBC ' +
                    'CLI Driver completed successfully ...');
        console.log(license_agreement);
    } else {
        require('targz').decompress({src: INSTALLER_FILE, dest:  DOWNLOAD_DIR}, function (err) {
            if (err) {
                console.log(err);
                return process.exit(1);
            }
            console.log('Download and extraction of DB2 ODBC ' +
                        'CLI Driver completed successfully ...');
            console.log(license_agreement);
            IBM_DB_HOME = path.resolve(DOWNLOAD_DIR, 'clidriver');
            process.env.IBM_DB_HOME = IBM_DB_HOME;
            buildBinary(true);
            removeWinBuildArchive();
        });
    }
}

function buildBinary(isDownloaded) {
    var buildString = "node-gyp configure build --IBM_DB_HOME=$IBM_DB_HOME --IS_DOWNLOADED=" + isDownloaded.toString();
    if (platform === 'win32') {
        buildString += " --IBM_DB_HOME_WIN=%IBM_DB_HOME%";
    }
    exec(buildString, function (error, stdout, stderr) {
        console.log(stdout);
        if (error) {
            console.log(error);
            process.exit(1);
        }

        if (platform === 'darwin' && arch === 'x64') {
            // Run the install_name_tool
            var nameToolCommand = "install_name_tool -change libdb2.dylib $IBM_DB_HOME/lib/libdb2.dylib ./build/Release/odbc_bindings.node"
            var nameToolCmdProcess = exec(nameToolCommand, function (error1, stdout1, stderr1) {
                if (error1) {
                    console.log('Error setting up the lib path to ' +
                        'odbc_bindings.node file.Error trace:\n'+error1);
                    process.exit(1);
                }
            });
        }
    });
}

function removeWinBuildArchive() {
    var WIN_BUILD_FILE = path.resolve(CURRENT_DIR, 'build.zip');
    var exists = fs.existsSync(WIN_BUILD_FILE);
    if (exists) {
        fs.unlinkSync(WIN_BUILD_FILE);
    }
}

function getInstallerFile(installerfileURL) {
    var outStream = fs.createWriteStream(INSTALLER_FILE);
    request(installerfileURL).pipe(outStream);
    outStream.once('close', copyAndExtractDriver).once('error', function (err) {
        throw err;
    });
}


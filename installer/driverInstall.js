/**
 * New node file
 */

var fs = require('fs');
var url = require('url');
var http = require('http');
var os = require('os');
var path = require('path');
var exec = require('child_process').exec;
var sh = require('exec-sync');

var installerURL = 'http://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/';
var CURRENT_DIR = process.cwd();
var DOWNLOAD_DIR = path.resolve(CURRENT_DIR, 'installer');
var INSTALLER_FILE;

//Function to download file using HTTP.get
var download_file_httpget = function(file_url) {
	var readStream;
	var writeStream;
	var platform = os.platform();
	var arch = os.arch();
	var installerfileURL;

	var fstream = require('fstream');
	var unzip = require('unzip');

	var IBM_DB_HOME, IBM_DB_INCLUDE, IBM_DB_LIB, IBM_DB_DIR;

	if(platform == 'win32') {

		if(arch == 'x64') {
			var BUILD_FILE = path.resolve(CURRENT_DIR, 'build.zip');
			readStream = fs.createReadStream(BUILD_FILE);
			writeStream = fstream.Writer(CURRENT_DIR);

			readStream
			  .pipe(unzip.Parse())
			  .pipe(writeStream).on("unpipe", function () {
                fs.unlinkSync(BUILD_FILE);
                var ODBC_BINDINGS = path.resolve(CURRENT_DIR,
                                      'build\\Release\\odbc_bindings.node');
                var ODBC_BINDINGS_V10 = path.resolve(CURRENT_DIR,
                               'build\\Release\\odbc_bindings.node.0.10.36');
                fs.exists(ODBC_BINDINGS_V10, function() {
                  if(Number(process.version.match(/^v(\d+\.\d+)/)[1]) < 0.12) {
                      fs.renameSync(ODBC_BINDINGS_V10, ODBC_BINDINGS);
                  } else {
                      fs.unlinkSync(ODBC_BINDINGS_V10);
                  }
                });
			});

		} else {
			console.log('Windows 32 bit not supported. Please use an x64 architecture.');
			return;
		}
	}


	if(process.env.IBM_DB_HOME) {

		IBM_DB_HOME = process.env.IBM_DB_HOME;
		IBM_DB_INCLUDE = path.resolve(IBM_DB_HOME, 'include');
		IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib');
		console.log('IBM_DB_HOME environment variable have already been set to '+IBM_DB_HOME);

		if (!fs.existsSync(IBM_DB_HOME)) {
			console.log('Environment variable IBM_DB_HOME is not set to the correct directory. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
		}

		if (!fs.existsSync(IBM_DB_INCLUDE)) {
			console.log('Environment variable IBM_DB_HOME is not set to the correct directory. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
		}

		if (!fs.existsSync(IBM_DB_LIB)) {
			console.log('Environment variable IBM_DB_HOME is not set to the correct directory. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
		}
		if( platform != 'win32') {

			if(platform == 'linux' || (platform == 'darwin' && arch == 'x64')) {
				removeWinBuildArchive();
				buildBinary(false);
			} else {

				console.log('Building binaries for node-ibm_db. This platform is not completely supported, you might encounter errors. In such cases please open an issue on our repository, https://github.com/ibmdb/node-ibm_db.');
				buildBinary(false);
			}
		}
	} else {

		if(platform == 'win32') {

			if(arch == 'x64') {
				installerfileURL = installerURL + 'ntx64_odbc_cli.zip';
			}/* else {
				installerfileURL = installerURL + 'nt32_odbc_cli.zip';
			}*/
		} else if(platform == 'linux') {
			if(arch == 'x64') {

				installerfileURL = installerURL + 'linuxx64_odbc_cli.tar.gz';
			} else {

				installerfileURL = installerURL + 'linuxia32_odbc_cli.tar.gz';
			}
		} else if(platform == 'darwin') {
			if(arch == 'x64') {

				installerfileURL = installerURL + 'macos64_odbc_cli.tar.gz';
			} else {
				console.log('Mac OS 32 bit not supported. Please use an x64 architecture.');
				return;
			}
		} else {
			installerfileURL = installerURL + platform + arch + '_odbc_cli.tar.gz';
		}

		if(!installerfileURL) {
			console.log('Unable to fetch driver download file. Exiting the install process.');
			process.exit(1);
		}
		/*
		var options = {
		 host: url.parse(installerfileURL).host,
		 port: 80,
		 path: url.parse(installerfileURL).pathname
		};
		*/

		var options = buildHttpOptions(installerfileURL);
		var license_agreement = '\n****************************************\nYou are downloading a package which includes the Node.js module for IBM DB2/Informix.  The module is licensed under the Apache License 2.0. The package also includes IBM ODBC and CLI Driver from IBM, which is automatically downloaded as the node module is installed on your system/device. The license agreement to the IBM ODBC and CLI Driver is available in '+DOWNLOAD_DIR+'   Check for additional dependencies, which may come with their own license agreement(s). Your use of the components of the package and dependencies constitutes your acceptance of their respective license agreements. If you do not accept the terms of any license agreement(s), then delete the relevant component(s) from your device.\n****************************************\n';

		var file_name = url.parse(installerfileURL).pathname.split('/').pop();
		INSTALLER_FILE = path.resolve(DOWNLOAD_DIR, file_name);

		console.log('Downloading DB2 ODBC CLI Driver from '+installerfileURL+'...');

		http.get(options, function(res) {

			if( res.statusCode != 200 ) {
				log( "Unable to download IBM ODBC and CLI Driver from "+installerfileURL );
				process.exit(1);
			}

			//var file = fs.createWriteStream(INSTALLER_FILE);
			var fileLength = parseInt( res.headers['content-length'] );
			var buf = new Buffer( fileLength );
			var byteIndex = 0;

			res.on('data', function(data) {
				if( byteIndex + data.length > buf.length ) {
					log( "Error downloading IBM ODBC and CLI Driver from "+installerfileURL );
					process.exit(1);
				}
				data.copy( buf, byteIndex );
				byteIndex += data.length;

			 }).on('end', function() {

				 if( byteIndex != buf.length ) {
					log( "Error downloading IBM ODBC and CLI Driver from "+installerfileURL );
					process.exit(1);
				}

				var file = fs.openSync( INSTALLER_FILE, 'w');
				var len = fs.writeSync( file, buf, 0, buf.length, 0 );
				if( len != buf.length ) {

					log( "Error writing IBM ODBC and CLI Driver to a file" );
					process.exit(1);
				}
				fs.closeSync( file );

				if(platform == 'win32') {

					readStream = fs.createReadStream(INSTALLER_FILE);
					writeStream = fstream.Writer(DOWNLOAD_DIR);

					readStream
					  .pipe(unzip.Parse())
					  .pipe(writeStream);

					console.log('Download and extraction of DB2 ODBC CLI Driver completed successfully ...');
					console.log(license_agreement);

				}
				//CLI Driver file for all platforms other than windows is a tar.gz file
				else /*if(platform == 'linux' || (platform == 'darwin' && arch == 'x64'))*/ {

						var targz = require('tar.gz');
						var compress = new targz().extract(INSTALLER_FILE, DOWNLOAD_DIR, function(err){
						if(err) {
							console.log(err);
							process.exit(1);
						}
						console.log('Download and extraction of DB2 ODBC CLI Driver completed successfully ...');
						IBM_DB_HOME = path.resolve(DOWNLOAD_DIR, 'clidriver');
						process.env.IBM_DB_HOME = IBM_DB_HOME;
						buildBinary(true);
						removeWinBuildArchive();

					});
				}

			 });
		 });
	}

	function buildBinary(isDownloaded) {
		var buildString = "node-gyp configure build --IBM_DB_HOME=$IBM_DB_HOME --IBM_DB_HOME_WIN=%IBM_DB_HOME%";
		if(isDownloaded) {
			buildString = buildString + " --IS_DOWNLOADED=true";
		} else {
			buildString = buildString + " --IS_DOWNLOADED=false";
		}
		var childProcess = exec(buildString, function (error, stdout, stderr) {
			console.log(stdout);
			if (error !== null) {
				console.log(error);
				process.exit(1);
			}

			if(platform == 'darwin' && arch == 'x64') {

				// Run the install_name_tool
				var nameToolCommand = "install_name_tool -change libdb2.dylib $IBM_DB_HOME/lib/libdb2.dylib ./build/Release/odbc_bindings.node"
				var nameToolCmdProcess = exec(nameToolCommand , function (error1, stdout1, stderr1) {
					if (error1 !== null) {
						console.log('Error setting up the lib path to odbc_bindings.node file.Error trace:\n'+error1);
						process.exit(1);
					}
				});

				console.log(license_agreement);
			}
		});
	}

	function removeWinBuildArchive() {
		var WIN_BUILD_FILE = path.resolve(CURRENT_DIR, 'build.zip');
		fs.exists(WIN_BUILD_FILE, function(exists) {
			if (exists) {
				fs.unlinkSync(WIN_BUILD_FILE);
			}
		});
	}

	function buildHttpOptions(installerfileURL) {
		var options = {
			 host: url.parse(installerfileURL).host,
			 port: 80,
			 path: url.parse(installerfileURL).pathname
			};
		var proxyStr = sh('npm config get proxy');
		if(proxyStr === 'null' || proxyStr === '') {
			proxyStr = sh('npm config get https-proxy');
		}

		if(proxyStr === 'null' || proxyStr === '') {
			return options;
		}

		var splitIndex = proxyStr.toString().lastIndexOf(':');
                if(splitIndex > 0) {
			options = {
				host: url.parse(proxyStr.toString()).hostname,
				port: url.parse(proxyStr.toString()).port,
				path: url.parse(installerfileURL).href
				};
                }
		return options;
	}
};

download_file_httpget();
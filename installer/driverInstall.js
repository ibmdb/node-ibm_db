/**
 * New node file
 */

var fs = require('fs');
var url = require('url');
var http = require('http');
var os = require('os');
var path = require('path');
var exec = require('child_process').exec;

var installerURL = 'https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/';
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
			  .pipe(writeStream).on("end", function () {
			});
			
		}
	}
	
	
	if(process.env.IBM_DB_HOME) {
		
		IBM_DB_HOME = process.env.IBM_DB_HOME;
		IBM_DB_INCLUDE = path.resolve(IBM_DB_HOME, 'include');
		IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib');
		console.log('IBM_DB_HOME environment variable have already been set to '+IBM_DB_HOME);
		
		if (!fs.existsSync(IBM_DB_HOME)) {
			console.log(IBM_DB_HOME);
			console.log('Environment variable IBM_DB_HOME is not set to the correct directory. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
		}
		
		if (!fs.existsSync(IBM_DB_INCLUDE)) {
			console.log(IBM_DB_INCLUDE);
			console.log('Environment variable IBM_DB_HOME is not set to the correct directory. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
		}
		
		if (!fs.existsSync(IBM_DB_LIB)) {
			console.log(IBM_DB_LIB);
			console.log('Environment variable IBM_DB_HOME is not set to the correct directory. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
		}
		if(platform == 'linux') {
			removeWinBuildArchive();
			buildBinary(false);
		}
		
	} else {
	
		if(platform == 'win32') {
		
			if(arch == 'x64') {
				installerfileURL = installerURL + 'ntx64_odbc_cli.zip';
			} else {
				installerfileURL = installerURL + 'nt32_odbc_cli.zip';
			}
		} else if(platform == 'linux') {
			if(arch == 'x64') {
				
				installerfileURL = installerURL + 'linuxx64_odbc_cli.tar.gz';
			} else {
				
				installerfileURL = installerURL + 'linuxia32_odbc_cli.tar.gz';
			}
		}
		
		if(!installerfileURL) {
			console.log('Unable to fetch driver download file. Exiting the install process.');
			process.exit(0);
		}

		var options = {
		 host: url.parse(installerfileURL).host,
		 port: 80,
		 path: url.parse(installerfileURL).pathname
		};
		
		var license_agreement = '\n****************************************\nYou are downloading a package which includes the Node.js module for IBM DB2/Informix.  The module is licensed under the Apache License 2.0. The package also includes IBM ODBC and CLI Driver from IBM, which is automatically downloaded as the node module is installed on your system/device. The license agreement to the IBM ODBC and CLI Driver is available in '+DOWNLOAD_DIR+'   Check for additional dependencies, which may come with their own license agreement(s). Your use of the components of the package and dependencies constitutes your acceptance of their respective license agreements. If you do not accept the terms of any license agreement(s), then delete the relevant component(s) from your device.\n****************************************\n';

		var file_name = url.parse(installerfileURL).pathname.split('/').pop();
		INSTALLER_FILE = path.resolve(DOWNLOAD_DIR, file_name);
		var file = fs.createWriteStream(INSTALLER_FILE);
		
		console.log('Downloading DB2 ODBC CLI Driver from '+installerfileURL+'...');
		
		http.get(options, function(res) {
			 
			res.on('data', function(data) {
				 file.write(data);
			 }).on('end', function() {
				 file.end();
				 
				if(platform == 'win32') {
					
					readStream = fs.createReadStream(INSTALLER_FILE);
					writeStream = fstream.Writer(DOWNLOAD_DIR);

					readStream
					  .pipe(unzip.Parse())
					  .pipe(writeStream);
					
					console.log('Download and extraction of DB2 ODBC CLI Driver completed successfully ...');
					console.log(license_agreement);
					
				} else if(platform == 'linux') {

						var targz = require('tar.gz');
						var compress = new targz().extract(INSTALLER_FILE, DOWNLOAD_DIR, function(err){
						if(err) {
							console.log(err);
							process.exit(0);
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
				process.exit(0);
			}
			console.log(license_agreement);
		});
	}
	
	function removeWinBuildArchive() {
		var WIN_BUILD_FILE = path.resolve(CURRENT_DIR, 'build.zip');
		fs.unlinkSync(WIN_BUILD_FILE);
	}

};

download_file_httpget();

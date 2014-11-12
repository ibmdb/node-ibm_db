/**
 * New node file
 */

var fs = require('fs');
var url = require('url');
var http = require('http');
var os = require('os');
var path = require('path');

var installerURL = 'https://public.dhe.ibm.com/ibmdl/export/pub/software/data/db2/drivers/odbc_cli/';
var CURRENT_DIR = process.cwd();
var DOWNLOAD_DIR = path.resolve(CURRENT_DIR, 'installer');
var INSTALLER_FILE; 

//Function to download file using HTTP.get
var download_file_httpget = function(file_url) {
	
	var platform = os.platform();
	var arch = os.arch();
	var installerfileURL;
	
	var IBM_DB_HOME, IBM_DB_INCLUDE, IBM_DB_LIB, IBM_DB_DIR;
	
	console.log('Current working directory: '+process.cwd());
	
	if(process.env.IBM_DB_HOME) {
		
		IBM_DB_HOME = process.env.IBM_DB_HOME;
		IBM_DB_INCLUDE = path.resolve(IBM_DB_HOME, 'include');
		IBM_DB_LIB = path.resolve(IBM_DB_HOME, 'lib');
		console.log('IBM_DB_HOME set to '+IBM_DB_HOME);
		
		if (!fs.existsSync(IBM_DB_HOME)) {
			console.log('Environment variable IBM_DB_HOME is not set to the correct directory. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
		}
		
		if (!fs.existsSync(IBM_DB_INCLUDE)) {
			console.log('Environment variable IBM_DB_HOME is not set to the correct directory. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
		}
		
		if (!fs.existsSync(IBM_DB_LIB)) {
			console.log('Environment variable IBM_DB_HOME is not set to the correct directory. Please check if you have set the IBM_DB_HOME environment variable\'s value correctly.');
		}
		
		process.exit(0);
	}
	
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
	
	
	var file_name = url.parse(installerfileURL).pathname.split('/').pop();
	INSTALLER_FILE = path.resolve(DOWNLOAD_DIR, file_name);
	var file = fs.createWriteStream(INSTALLER_FILE);
	
	console.log('Downloading dsdriver...');
	
	http.get(options, function(res) {
		 
		res.on('data', function(data) {
	         file.write(data);
	     }).on('end', function() {
	         file.end();
	         
	         console.log(file_name + ' downloaded to ' + DOWNLOAD_DIR);
	         console.log('DSDriver download completed..');
	     	 console.log('Unzipping '+file_name+' ..');
	     	
	     	if(platform == 'win32') {
	     		
	     		var fstream = require('fstream');
		     	var unzip = require('unzip');
		     	var readStream = fs.createReadStream(INSTALLER_FILE);
		     	var writeStream = fstream.Writer(DOWNLOAD_DIR);

		     	readStream
		     	  .pipe(unzip.Parse())
		     	  .pipe(writeStream)
	     		
	     	} else if(platform == 'linux') {

	     			var targz = require('tar.gz');
					var compress = new targz().extract(INSTALLER_FILE, DOWNLOAD_DIR, function(err){
	     		    if(err)
	     		        console.log(err);

	     		    console.log('The extraction has ended!');
	     		});
	     	}
	     	
	     });
	 });
		 
};

download_file_httpget();

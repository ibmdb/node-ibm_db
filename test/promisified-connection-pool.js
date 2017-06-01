var common = require("./common")
	, Pool = require("../").Pool,
	, connectionString = common.connectionString;
var Promise = require("bluebird");
Promise.promisifyAll(require("../lib/odbc").prototype);
Promise.promisifyAll(require("../lib/odbc").Pool.prototype);
Promise.promisifyAll(require("../lib/odbc").Database.prototype);
var pool = new Pool();
function getSqlConnection()
{

  return pool.openAsync(connectionString).disposer(function (connection) { 
			});
  	 
  }
exports.getSqlConnection = getSqlConnection;

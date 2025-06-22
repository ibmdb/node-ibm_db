/*
  Copyright (c) 2013, Dan VerWeire <dverweire@gmail.com>
  Copyright (c) 2010, Lee Smith <notwink@gmail.com>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

// Setting SQLLIB bin path to the path env before load for windows
var os = require('os'),
    path = require('path');
var platform = os.platform();

// Set PATH and LIB env vars for clidriver.
var clidriver = path.resolve(__dirname, '../installer/clidriver');
if (process.env.IBM_DB_HOME) {
  clidriver = process.env.IBM_DB_HOME;
}
if(platform == 'win32')
{
  process.env.PATH = path.resolve(clidriver, 'bin') + ';' +
                      path.resolve(clidriver, 'bin/amd64.VC12.CRT') + ';' +
                      path.resolve(clidriver, 'bin/icc64') + ';' +
                      path.resolve(clidriver, 'lib') + ';' + process.env.PATH;
  process.env.LIB =  path.resolve(clidriver, 'bin') + ';' +
                      path.resolve(clidriver, 'bin/icc64') + ';' +
                      path.resolve(clidriver, 'lib') + ';' + process.env.LIB;
}
else if(platform == 'linux') {
  process.env.PATH = path.resolve(clidriver, 'bin') + ':' +
                      path.resolve(clidriver, 'lib') + ':' + process.env.PATH;
  process.env.LD_LIBRARY_PATH = path.resolve(clidriver, 'lib') + ':' +
                                path.resolve(clidriver, 'lib/icc') + ':' + process.env.LD_LIBRARY_PATH;
}
else if(platform == 'darwin') {
  process.env.PATH = path.resolve(clidriver, 'bin') + ':' +
                      path.resolve(clidriver, 'lib') + ':' + process.env.PATH;
  process.env.DYLD_LIBRARY_PATH = path.resolve(clidriver, 'lib') + ':' +
                                  path.resolve(clidriver, 'lib/icc') + ':' + process.env.DYLD_LIBRARY_PATH;
}
else if(platform == 'aix') {
  process.env.PATH = path.resolve(clidriver, 'bin') + ':' +
                      path.resolve(clidriver, 'lib') + ':' + process.env.PATH;
  process.env.LIBPATH = path.resolve(clidriver, 'lib') + ':' +
                        path.resolve(clidriver, 'lib/icc') + ':' + process.env.LIBPATH;
}

var odbc = require("bindings")("odbc_bindings")
  , SimpleQueue = require("./simple-queue")
  , macro = require("./climacros")
  , util = require("util")
  , Readable = require('stream').Readable
  , Q = require('q');


// Call of odbc.ODBC() loads odbc library and allocate environment handle.
// All calls of new Database() should use this same odbc unless passed as
// options.odbc. ENV will keep value of this odbc after first call of Database.
var ENV;
module.exports = function (options)
{
  return new Database(options);
};

module.exports.Database = Database;
module.exports.ODBC = odbc.ODBC;
module.exports.ODBCConnection = odbc.ODBCConnection;
module.exports.ODBCStatement = odbc.ODBCStatement;
module.exports.ODBCResult = odbc.ODBCResult;

exports.debug = false;
var logParams = false;
var ibmdbStartTime = new Date();
var getElapsedTime = function(){
            var tstamp = (new Date() - ibmdbStartTime)/1000.0;
            //process.stdout.write(tstamp + " :: ");
            return (tstamp + " :: ");
            };
module.exports.getElapsedTime = getElapsedTime;

module.exports.debug = function(x) {
    if(x) {
        exports.debug = true;
        console.log("node-ibm_db logs enabled.");
        if(x == 2) {
            logParams = true;
        }
    }
    else {
        exports.debug = false;
        console.log("node-ibm_db logs disabled.");
    }
};

var connError = {
    message: "Connection not open.",
    sqlstate: "08001",
    sqlcode: -30081
};

module.exports.open = function (connStr, options, cb)
{
  var db, deferred;

  var DBFactory = function(options) {
    return new Database(options);
  };

  if (!cb && typeof options !== 'function')
  {
    if(!options)
    {
      options = null;
    }
    db = DBFactory(options);
    deferred = Q.defer();
    db.open(connStr, function(err) {
      if (err)
      {
        deferred.reject(err);
      }
      else
      {
        deferred.resolve(db);
      }
    });
    return deferred.promise;
  }

  else if (typeof options === 'function')
  {
    cb = options;
    options = null;
  }

  db = DBFactory(options);

  db.open(connStr, function (err) {
    cb && cb(err, db);
  });
};  // ibmdb.open

module.exports.openSync = function (connStr, options)
{
  var db = new Database(options);
  db.openSync(connStr);
  return db;
}; // ibmdb.openSync

/*
 * Create Database.
 * (createDbSync)
 * args: dbName, codeSet, mode.
 * (codeSet, mode are optional and default values will be null)
 * 
 * return: TRUE on success or FALSE on failure.
 */
module.exports.createDbSync = function (dbName, connStr, options)
{
  // createDbSync API is not supported on z/OS:
  // Databases are implemented/used in Db2 for z/OS differently than
  // and Db2 for LUW.  A database in z/OS is simply a logical collection
  // of table/index spaces that you create using the "CREATE DATABASE"
  // SQL statement, while a database in LUW is conceptually equivalent
  // to a subsystem in z/OS.  Connecting to a Db2 on z/OS subsystem
  // entails a "database" is created already.  As such, this API is
  // not applicable.
  if (os.type() === "OS/390")
  {
    throw new Error("[node-ibm_db] createDbSync API is not supported on z/OS");
  }

  var result;
  var db = new Database(options);

  if (typeof(connStr) === "object")
  {
    var obj = connStr;
    connStr = "";

    Object.keys(obj).forEach(function (key) {
      connStr += key + "=" + obj[key] + ";";
    });
  }
  var conStr = connStr + ";" + "ATTACH=true";

  var connectionOpen = db.openSync(conStr);

  if (connectionOpen)
  {
    db.connected = true;
  }

  if (!db.connected)
  {
    throw ({ message : "Connection not open."});
  }
  else
  {
    if(dbName == null )
    {
      throw ({ message : "Database Name <db_Name> is required."});
    }
    else
    {
      // IF: argument "options" is not passed while calling.
      // THEN: db.codeSet and db.mode will be "null" (default value).
      result = db.conn.createDbSync(dbName, db.codeSet, db.mode);
      return result;
    }
  }
}; // ibmdb.createDbSync

/*
 * Drop Database.
 * (dropDbSync)
 * args: dbName
 * 
 * return: TRUE on success or FALSE on failure.
 */
module.exports.dropDbSync = function (dbName, connStr, options)
{
  // dropDbSync API is not supported on z/OS:
  // Databases are implemented/used in Db2 for z/OS differently than
  // and Db2 for LUW.  A database in z/OS is simply a logical collection
  // of table/index spaces that you create using the "CREATE DATABASE"
  // SQL statement, while a database in LUW is conceptually equivalent
  // to a subsystem in z/OS.  Connecting to a Db2 on z/OS subsystem
  // entails a "database" is created already.  As such, this API is
  // not applicable for Db2 on z/OS servers.
  if (os.type() === "OS/390")
  {
    throw new Error("[node-ibm_db] dropDbSync API is not supported on z/OS");
  }

  var result;
  var db = new Database(options);

  if (typeof(connStr) === "object")
  {
    var obj = connStr;
    connStr = "";

    Object.keys(obj).forEach(function (key) {
      connStr += key + "=" + obj[key] + ";";
    });
  }
  var conStr = connStr + ";" + "ATTACH=true";  

  var connectionOpen = db.openSync(conStr);

  if (connectionOpen)
  {
    db.connected = true;
  }

  if (!db.connected)
  {
    throw ({ message : "Connection not open."});
  }
  else
  {
    if(dbName == null )
    {
      throw ({ message : "Database Name <db_Name> is required."});
    }
    else
    {
      result = db.conn.dropDbSync(dbName);
      return result;
    }
  }
}; // ibmdb.dropDbSync

module.exports.close = function(db)
{
  if(db && typeof(db) === "object")
  {
    for(var key in db)
    {
      delete db[key];
    }
    db = undefined;
  }
}; // ibmdb.close

function Database(options)
{
  var self = this;

  options = options || {};

  self.odbc = (options.odbc) ? options.odbc : ((ENV) ? ENV : new odbc.ODBC());
  if(!ENV) ENV = self.odbc;
  self.queue = new SimpleQueue();
  self.fetchMode = options.fetchMode || null;
  self.connected = false;
  self.connectTimeout = options.connectTimeout || null;
  self.systemNaming = options.systemNaming;
  self.codeSet = options.codeSet || null;
  self.mode = options.mode || null;
  self.pool = options.pool || null;
  if( self.pool ) { // Its a pooled connection
    if( !self.realClose ) {
      self.realClose = self.close;
      self.close = self.poolClose;
      self.realCloseSync = self.closeSync;
      self.closeSync = self.poolCloseSync;
    }
    self.connStr = options.connStr || null;
  }
} // Database()

//Expose constants
Object.keys(odbc.ODBC).forEach(function (key) {
  if (typeof odbc.ODBC[key] !== "function")
  {
    //On the database prototype
    Database.prototype[key] = odbc.ODBC[key];

    //On the exports
    module.exports[key] = odbc.ODBC[key];
  }
});

Database.prototype.open = function (connStr, cb) {
  var self = this, deferred;

  if (typeof(connStr) === "object")
  {
    var obj = connStr;
    connStr = "";

    Object.keys(obj).forEach(function (key) {
      connStr += key + "=" + obj[key] + ";";
    });
  }

  if (!cb)
  {
    deferred = Q.defer();
  }

  self.odbc.createConnection(function (err, conn) {
    if(!cb)
    {
      if (err) deferred.reject(err);
    } else
    {
      if (err) return cb(err);
    }

    self.conn = conn;

    if (self.connectTimeout || self.connectTimeout === 0)
    {
      self.conn.connectTimeout = self.connectTimeout;
    }
    if (typeof(self.systemNaming) !== 'undefined')
    {
      self.conn.systemNaming = self.systemNaming;
    }

    self.conn.open(connStr, function (err, result)
    {
      if(cb)
      {
        if (err) cb(err);
        else {
          self.connected = true;
          cb(err, result);
        }
      } 
      else
      {
        if(err) deferred.reject(err);

        self.connected = true;
        deferred.resolve(result);
      }
    }); //conn.open
  }); // odbc.createConnection

  return deferred ? deferred.promise : null;
}; // Database.open function

Database.prototype.openSync = function (connStr)
{
  var self =  this;

  self.conn = self.odbc.createConnectionSync();

  if (self.connectTimeout || self.connectTimeout === 0)
  {
    self.conn.connectTimeout = self.connectTimeout;
  }
  if (typeof(self.systemNaming) !== 'undefined')
  {
    self.conn.systemNaming = self.systemNaming;
  }

  if (typeof(connStr) === "object")
  {
    var obj = connStr;
    connStr = "";

    Object.keys(obj).forEach(function (key) {
      connStr += key + "=" + obj[key] + ";";
    });
  }

  var result = self.conn.openSync(connStr);

  if (result)
  {
    self.connected = true;
  }

  return result;
}; // Database.openSync

Database.prototype.close = function (cb)
{
  var self = this, deferred;
  if(!cb) 
  {
    deferred = Q.defer();
  }

  self.queue.push(function (next) {
    if(self.conn)
    {
      self.conn.close(function (err) {
        self.connected = false;
        delete self.conn;

        if (err) {
          deferred ? deferred.reject(err) : cb(err);
        } else {
          deferred ? deferred.resolve(true) : cb(null);
        }
        return next();
      });
    }
    else
    {
      self.connected = false;
      deferred ? deferred.resolve(true) : cb(null);
      return next();
    }
  }); // self.queue.push

  return deferred ? deferred.promise : false;
}; // Database.close

Database.prototype.closeSync = function ()
{
  var self = this;

  var result;
  if(self.conn) result = self.conn.closeSync();

  self.connected = false;
  delete self.conn;

  return result;
}; // closeSync

Database.prototype.checkConnectionError = function (err)
{
  // For pooled connection, if we get SQL30081N, then close
  // the connection now only and then proceed.

  var self = this;
  if(self.realClose){
    if((err && err['message'] && err['message'].search("SQL30081N") != -1))
    {
      // Close all connections in availablePool as all are invalid now
      // It will be removed from available pool on next pool.open for connected=false
      self.closeSync();  //Move connection from usedPool to availablePool
      var availablePool = self.pool.availablePool[self.connStr];
      if (availablePool && availablePool.length) {
        for (var i = 0, len = availablePool.length; i < len; i++) {
          availablePool[i].realCloseSync();
        }
      }
      self.realCloseSync();
    }
  }
}

Database.prototype.query = function (query, params, cb)
{
  var self = this, deferred, sql, resultset = [], multipleResultSet = false;
  var sqlca = {sqlstate:"0", sqlcode: 0};

  exports.debug && console.log(getElapsedTime(), "odbc.js:query() => Entry");
  //support for promises
  if (!cb && typeof params !== 'function')
  {
    deferred = Q.defer();
    !params ? params = null : '';
  }

  if (typeof(params) === 'function')
  {
    cb = params;
    params = null;
  }

  self.queue.push(function (next) {
    function cbQuery (initialErr, result, outparams)
    {
      if(outparams) {
        resultset = outparams;
        multipleResultSet = true;
      }
      if (result && typeof(result) === 'object') {
        fetchMore();
      } else {
        if (initialErr) {
          if (initialErr.sqlstate) {
            sqlca.sqlstate = initialErr.sqlstate;
          }
          if (initialErr.sqlcode) {
            sqlca.sqlcode = initialErr.sqlcode;
          }
          self.checkConnectionError(initialErr);
          if(deferred) {
            let error = {};
            if(typeof initialErr === 'object') {
                error = initialErr;
            } else {
                error["message"] = initialErr;
            }
            error["resultset"] = resultset;
            error.sqlcode = sqlca.sqlcode;
            error.sqlstate = sqlca.sqlstate;
            deferred.reject(error);
          } else {
            cb(initialErr, resultset, sqlca);
          }
        }
        if (result) { result.closeSync(); }
        initialErr = null;
        return next();
      }

      function fetchMore()
      {
        if (self.fetchMode)
        {
          result.fetchMode = self.fetchMode;
        }

        result._fetchAll(function (err, data, colcount) {
          var moreResults = false, moreResultsError = null;

          // If there is any error, return it now only.
          if( err || initialErr )
          {
            self.checkConnectionError(err || initialErr);
            if(multipleResultSet) resultset.push(data);
            else resultset = data;
            err = initialErr || err;
            sqlca.sqlstate = err.sqlstate || sqlca.sqlstate;
            sqlca.sqlcode = err.sqlcode || sqlca.sqlcode;
            console.log(err, sqlca);
            if(deferred) {
              err["resultset"] = resultset;
              err.sqlcode = sqlca.sqlcode;
              err.sqlstate = sqlca.sqlstate;
              deferred.reject(err);
            } else {
              cb(err, resultset, sqlca);
            }
            if (result) { result.closeSync(); }
            initialErr = null;
            err = null;
            return next();
          }

          // Get the result data
          try
          {
            moreResults = result.moreResultsSync();
          }
          catch (e)
          {
            moreResultsError = e;
            moreResults = false;
            sqlca.sqlstate = e.sqlstate || sqlca.sqlstate;
            sqlca.sqlcode = e.sqlcode || sqlca.sqlcode;
          }

          //close the result before calling back
          //if there are not more result sets
          if (moreResults)
          {
            resultset.push(data);
            multipleResultSet = true;
            fetchMore();
          }
          else
          {
            result.closeSync();
            if( colcount ) {
              if(multipleResultSet) resultset.push(data);
              else resultset = data;
            }

            exports.debug && console.log(getElapsedTime(), "odbc.js:query() => Done.");
            // send exception error and/or data to callback function.
            // only once with all the results.
            if( !sqlca.sqlcode && colcount && !resultset.length ) {
                sqlca.sqlcode = 100;
            }
            if(deferred) {
              if(moreResultsError) {
                  moreResultsError["resultset"] = resultset;
                  moreResultsError.sqlcode = sqlca.sqlcode;
                  moreResultsError.sqlstate = sqlca.sqlstate;
                  deferred.reject(moreResultsError);
              } else {
                  deferred.resolve(resultset);
              }
            } else {
              cb(moreResultsError, resultset, sqlca);
            }
          }
          
          moreResultsError = null;
          return next();
        });
      } // function fetchMore
    } //function cbQuery

    if (!self.connected)
    {
      sqlca = {sqlstate: "08001", sqlcode : -30081};
      deferred ? deferred.reject(connError) : cb(connError, [], sqlca);
      return next();
    }

    if(typeof query === "object")
    {
      sql = query.sql;
      if(query.params) params = query.params;
    }
    else
    {
      sql = query;
    }
    exports.debug && console.log(getElapsedTime(), "odbc.js:query() => ", sql);

    try {
      if (params && params.length > 0)
      {
        if(Array.isArray(params))
        {
          var err = parseParams(params);
          if(err) deferred ? deferred.reject(err) : cb(err);
        }
        if(exports.debug && logParams) {
          console.log("%s odbc.js:query() params = ", getElapsedTime(), params);
        }
        if(typeof query === 'object')
        {
          query.params = params;
          self.conn.query(query, cbQuery);
        }
        else
          self.conn.query(query, params, cbQuery);
      }
      else
      {
        self.conn.query(query, cbQuery);
      }
    } catch (err)
    {
      self.checkConnectionError(err);
      deferred ? deferred.reject(err) : cb(err);
      return next();
    }
  }); //self.queue.push
  return deferred ? deferred.promise : false;
}; // Database.query

Database.prototype.queryResult = function (query, params, cb)
{
  var self = this, deferred, sql;

  //support for promises
  if (!cb && typeof params !== 'function')
  {
    deferred = Q.defer();
    !params ? params = null : '';
  }

  if (typeof(params) === 'function')
  {
    cb = params;
    params = null;
  }

  if(typeof query === "object")
  {
      sql = query.sql;
      if(query.params) params = query.params;
  }
  else
  {
      sql = query;
  }

  exports.debug && console.log(getElapsedTime(), "odbc.js:queryResult() => ", sql);
  self.queue.push(function (next) {
    //ODBCConnection.query() is the fastest-path querying mechanism.
    if (!self.connected)
    {
      deferred ? deferred.reject(connError) : cb(connError);
      return next();
    }

    if (params && params.length > 0)
    {
      if(Array.isArray(params))
      {
        var err = parseParams(params);
        if(err) deferred ? deferred.reject(err) : ( cb && cb(err) );
      }
      if(typeof query === 'object')
      {
        query.params = params;
        self.conn.query(query, cbQuery);
      }
      else
        self.conn.query(sql, params, cbQuery);
    }
    else
    {
      self.conn.query(sql, cbQuery);
    }

    function cbQuery (err, result, outparams)
    {
      if (err)
      {
        self.checkConnectionError(err);
        deferred ? deferred.reject(err) : ( cb && cb(err, null, outparams) );
        return next();
      }

      if (result && typeof(result) === 'object') {
          if (self.fetchMode)
          {
            result.fetchMode = self.fetchMode;
          }
      }
      else if( result )  // noResults = true is passed by application.
      {
          outparams = result; 
          result = null;
      }

      !cb ? deferred.resolve([result, outparams]) : cb(err, result, outparams);

      return next();
    } // function cbQuery
  }); //self.queue.push
  return deferred ? deferred.promise : false;
}; // Database.queryResult

Database.prototype.queryResultSync = function (query, params)
{
  var self = this, sql, outparams;
  var result; // Must get closed by caller for success case.

  if (!self.connected)
  {
    throw ({ message : "Connection not open."});
  }

  if(typeof query === "object")
  {
      sql = query.sql;
      if(query.params) params = query.params;
  }
  else
  {
      sql = query;
  }

  exports.debug && console.log(getElapsedTime(), "odbc.js:queryResultSync() => ", sql);
  try {
    if (params)
    {
      if(Array.isArray(params))
      {
          var err = parseParams(params);
          if(err) return err;
      }
      if(typeof query === 'object')
      {
          query.params = params;
          result = self.conn.querySync(query);
      }
      else
      {
          result = self.conn.querySync(sql, params);
      }
    }
    else
    {
      result = self.conn.querySync(sql); // Always returns [stmt, outparms]
    }
  } catch (e)
  {
      self.checkConnectionError(e);
      return e;
  }

  if(Array.isArray(result)) // Its a CALL stmt with OUT params.
  {
    if(result[1]) { 
        outparams = result[1]; // INOUT and OUTPUT param values for SP.
    }
    result = result[0];
  }
  if (result && typeof(result) === 'object' && self.fetchMode)
  {
    result.fetchMode = self.fetchMode;
  }
  if( outparams ) return [result, outparams];
  else return result;
}; // Database.queryResultSync

Database.prototype.querySync = function (query, params)
{
  var self = this, result, sql, outparams = null;

  if (!self.connected)
  {
    throw ({ message : "Connection not open."});
  }

  if(typeof query === "object")
  {
      sql = query.sql;
      if(query.params) params = query.params;
  }
  else
  {
      sql = query;
  }

  exports.debug && console.log(getElapsedTime(), "odbc.js:querySync() => ", sql);
  try {
    if (params)
    {
      if(Array.isArray(params))
      {
          var err = parseParams(params);
          if(err) return err;
          if(logParams) {
            console.log("%s odbc.js:querySync() params = ", getElapsedTime(), params);
          }
      }
      if(typeof query === 'object')
      {
          query.params = params;
          result = self.conn.querySync(query);
      }
      else
      {
        result = self.conn.querySync(sql, params);
      }
    }
    else
    {
      result = self.conn.querySync(query);
    }
  } catch (e)
  {
      self.checkConnectionError(e);
      return e;
  }

  if(Array.isArray(result))
  {
    if(result[1]) { 
        outparams = result[1]; // INOUT and OUT param values for SP.
    }
    result = result[0];
  }
  if(!result) return outparams;  // For noResults.

  // Processing for resultset.
  var data, resultset = [], moreResults = true, moreResultsError = null;
  var nullresult = true;

  if(outparams) {
      resultset = outparams;
      nullresult = false;
  }
  if (self.fetchMode)
  {
    result.fetchMode = self.fetchMode;
  }

  while(moreResults)
  {
      data = result.fetchAllSync();
      if(!data.length) {
          moreResults = false;
          break;
      }
      try
      {
          moreResults = result.moreResultsSync();
      }
      catch (e)
      {
          moreResultsError = e;
          moreResults = false;
          break;
      }
      if(data.length) {
          if(nullresult && !moreResults) resultset = data;
          else resultset.push(data);
          nullresult = false;
      }
  }
  result.closeSync();

  if(moreResultsError) {
      self.checkConnectionError(moreResultsError);
      return moreResultsError;
  }
  if(nullresult) return [];

  return resultset;
}; // Database.querySync

Database.prototype.executeFile = function (inputfile, delimiter, outputfile, cb) {
    var fs = require('fs-extra');
    var self = this, deferred, sql;

    if (typeof (delimiter) === 'function') {
        cb = delimiter;
        delimiter = ';'
    }

    if (typeof (outputfile) === 'function') {
        cb = outputfile;
        outputfile = undefined;
    }

    //support for promises
    if (!cb) {
      deferred = Q.defer();
    }

    if (typeof inputfile !== "string") {
        sql = inputfile.sql;
        if (inputfile.delimiter) delimiter = inputfile.delimiter;
        if (inputfile.outputfile) outputfile = inputfile.outputfile;
    }
    else {
        sql = inputfile;
    }

    if (!self.connected) {
        deferred ? deferred.reject(connError) : cb(connError);
    }
    fs.readFile(sql, function (err, sql) {
        if (err) {
            deferred ? deferred.reject(err) : cb(err);
        }
        else {
            var query = sql.toString();
            var myarray = query.split(delimiter);
            var res = "";
            if (outputfile !== undefined) {
                if (fs.existsSync(outputfile)) {
                    fs.unlinkSync(outputfile)
                }
                else {
                    fs.ensureFileSync(outputfile)
                }
            }
            for (var i = 0; i < myarray.length; i++) {
                query = (myarray[i]).trim();
                var result = [];
                if(query) {
                    result = self.querySync(query);
                    if (!Array.isArray(result)) {
                        if (outputfile === undefined) {
                            deferred ? deferred.reject(result) : cb(result);
                        }
                        else {
                            fs.appendFileSync(outputfile, result);
                        }
                    }
                    if (result.length > 0) {
                        result = JSON.stringify(result);
                        if (outputfile === undefined) {
                            res += result + delimiter;
                        }
                        else {
                            fs.appendFileSync(outputfile, result + delimiter);
                        }
                    }
                }
            }
            deferred ? deferred.resolve(res) : cb(err, res);
        }
    });
}; 

Database.prototype.executeFileSync = function (inputfile, delimiter, outputfile)
{
    var fs = require('fs-extra');
    var self = this, sql = null;
    if (!self.connected)
    {
        throw ({ message: "Connection not open." });
    }
    if (typeof inputfile !== "string")
    {
        sql = inputfile.sql;
        if (inputfile.delimiter) delimiter = inputfile.delimiter;
        if (inputfile.outputfile) outputfile = inputfile.outputfile;
    }
    else
    {
        sql = inputfile;
    }
    if (!fs.existsSync(sql))
    {
        return Error("[node-ibm_db] Input file " + sql + " does not exists");
    }
    if (outputfile !== undefined)
    {
        if (fs.existsSync(outputfile))
        {
            fs.unlinkSync(outputfile)
        }
        else
        {
            fs.ensureFileSync(outputfile)
        }
    }
    if (delimiter === undefined)
    {
        delimiter = ';'
    }
    var query = fs.readFileSync(sql, 'utf8')
    var myarray = query.split(delimiter);
    var res = "";
    for (var i = 0; i < myarray.length; i++)
    {
        var result = self.querySync(myarray[i]);
        if (!Array.isArray(result))
        {
            return result;
        }
        if (result.length > 0)
        {
            result = JSON.stringify(result)
            if (outputfile === undefined)
            {
                res += result + delimiter
            }
            else
            {
                fs.appendFileSync(outputfile, result + delimiter)
            }
        }
    }
    return res;
}; 

Database.prototype.queryStream = function queryStream(sql, params) 
{
  var self = this;
  var stream = new Readable({ objectMode: true });
  var results;
  stream._read = function() 
  {
    // after the first internal call to _read, the 'results' should be set
    // and the stream can continue fetching the results
    if (results) return self.fetchStreamingResults(results, stream);

    // in the first call to _read the stream starts to emit data once we've 
    // queried for results
    return self.queryResult(sql, params, function (err, result) 
    {
      if (err) 
      {
        return process.nextTick(function () { stream.emit('error', err); });
      }
      results = result;
      return self.fetchStreamingResults(results, stream);
    });
  };
  return stream;
};

Database.prototype.fetchStreamingResults = function(results, stream) 
{
  var self = this;
  return results.fetch(function (err, data) 
  {
    if (err) 
    {
      return process.nextTick(function () { stream.emit('error', err); });
    }
    // when no more data returns, return push null to indicate the end of stream
    if (!data) 
    { 
      results.closeSync(); // Close stmt handle. 
      stream.push(null);
    }
    else {
      stream.push(data);
    }
  });
};

Database.prototype.beginTransaction = function (cb)
{
  var self = this
      , deferred = null
      , onBeginTransaction;
  if(!cb) 
  {
    deferred = Q.defer();
  }
  onBeginTransaction = function(err) 
  {
    if(err) 
    {
      self.checkConnectionError(err);
      deferred ? deferred.reject(err) : cb(err);
    }
    else 
    {
      deferred ? deferred.resolve(true) : cb(null);
    }
  };

  self.conn.beginTransaction(onBeginTransaction);
  self.conn.inTransaction = true;

  return deferred ? deferred.promise : self;
};

Database.prototype.endTransaction = function (rollback, cb)
{
  var self = this, deferred = null;

  if(!cb) 
  {
    deferred = Q.defer();
  }
  var onEndTransaction = function(err) 
  {
    if(err) 
    {
      self.checkConnectionError(err);
      deferred ? deferred.reject(err) : cb(err);
    }
    else 
    {
      deferred ? deferred.resolve(true) : cb(null);
    }
  };
  self.conn.endTransaction(rollback, onEndTransaction);
  self.conn.inTransaction = false;

  return deferred ? deferred.promise : self;
};

Database.prototype.commitTransaction = function (cb)
{
  var self = this;
  //don't rollback
  return self.endTransaction(false, cb); 
};

Database.prototype.rollbackTransaction = function (cb)
{
  var self = this;
  return self.endTransaction(true, cb); //rollback
};

Database.prototype.beginTransactionSync = function ()
{
  var self = this;

  try {
    self.conn.beginTransactionSync();
  } catch (e) {
    self.checkConnectionError(e);
    throw new Error(e);
  }
  self.conn.inTransaction = true;

  return self;
};

Database.prototype.endTransactionSync = function (rollback)
{
  var self = this;

  self.conn.inTransaction = false;
  try {
    self.conn.endTransactionSync(rollback);
  } catch (e) {
    self.checkConnectionError(e);
    throw new Error(e);
  }

  return self;
};

Database.prototype.commitTransactionSync = function ()
{
  var self = this;
  return self.endTransactionSync(false); //don't rollback
};

Database.prototype.rollbackTransactionSync = function ()
{
  var self = this;
  return self.endTransactionSync(true); //rollback
};

Database.prototype.columns = async function(catalog, schema, table, column, callback)
{
  var self = this, deferred;
  var error = {
      error : "[node-ibm_db] Missing Arguments",
      message : "The object you passed must contain four arguments: " +
                "catalog, schema, table and column. " +
                "This is required for the columns method to work."
    };
  if (!self.queue) self.queue = [];

  callback = callback || arguments[arguments.length - 1];
  if(typeof(callback) !== 'function') {
    callback = null;
    deferred = Q.defer();
    if( arguments.length != 4 ) {
        deferred.reject(error);
        throw new Error(error.message);
    }
  }
  else if( arguments.length != 5 ) {
      callback(error, [], false);
  }
  self.queue.push(function (next) 
  {
    self.conn.columns(catalog, schema, table, column, function (err, result) 
    {
      if (err) {
        self.checkConnectionError(err);
        return deferred ? deferred.reject(err) : callback(err, [], false);
      }

      result._fetchAll(function (err, data)
      {
        result.closeSync();
        self.checkConnectionError(err);
        deferred ? deferred.resolve(data) : callback(err, data);
        return next();
      });
    });
  });
  if(deferred) return deferred.promise;
};

Database.prototype.tables = function(catalog, schema, table, type, callback)
{
  var self = this, deferred;
  var error = {
      error : "[node-ibm_db] Missing Arguments",
      message : "The object you passed must contain four arguments: " +
                "catalog, schema, table and type. " +
                "This is required for the tables method to work."
    };
  if (!self.queue) self.queue = [];

  callback = callback || arguments[arguments.length - 1];
  if(typeof(callback) !== 'function') {
    callback = null;
    deferred = Q.defer();
    if( arguments.length != 4 ) {
        deferred.reject(error);
        throw new Error(error.message);
    }
  }
  else if( arguments.length != 5 ) {
      callback(error, [], false);
  }

  self.queue.push(function (next) 
  {
    self.conn.tables(catalog, schema, table, type, function (err, result) 
    {
      if (err) {
        self.checkConnectionError(err);
        return deferred ? deferred.reject(err) : callback(err, [], false);
      }

      result._fetchAll(function (err, data)
      {
        result.closeSync();
        self.checkConnectionError(err);
        deferred ? deferred.resolve(data) : callback(err, data);
        return next();
      });
    });
  });
  if(deferred) return deferred.promise;
};

Database.prototype.describe = async function(obj, callback)
{
  var self = this, deferred;
  var error = {
      error : "[node-ibm_db] Missing Arguments",
      message : "You must pass an object as argument 0 if you want " +
                "anything productive to happen in the describe method."
    };

  if (typeof(callback) !== "function") {
    callback = null;
  }
  if(!callback) {
      deferred = Q.defer();
  }
  if (typeof(obj) !== "object") {
    deferred ? deferred.reject(error) : callback && callback(error, []);
  }

  if (!obj.database) {
    error = {
      error : "[node-ibmdb] Missing Arguments",
      message : "The object you passed did not contain a database " +
                "property. This is required for the describe method to work."
    };
    deferred ? deferred.reject(error) : callback && callback(error, []);
  }

  //set some defaults if they weren't passed
  obj.schema = obj.schema || "%";
  obj.type = obj.type || "TABLE";

  if (obj.table && obj.column)
  {
    //get the column details
    if(callback)
      self.columns(obj.database, obj.schema, obj.table, obj.column, callback);
    else
      return await self.columns(obj.database, obj.schema, obj.table, obj.column);
  }
  else if (obj.table)
  {
    //get the columns in the table
    if(callback)
      self.columns(obj.database, obj.schema, obj.table, "%", callback);
    else
      return await self.columns(obj.database, obj.schema, obj.table, "%")
                   .catch((e)=> { deferred.reject(e); });
  }
  else
  {
    //get the tables in the database
    if(callback)
      self.tables(obj.database, obj.schema, null, obj.type, callback);
    else
      return await self.tables(obj.database, obj.schema, null, obj.type);
  }
}; //Database.describe

Database.prototype.prepare = function (sql, cb)
{
  var self = this, deferred;

  if(!cb) 
  {
    deferred = Q.defer();
  }

  self.conn.createStatement(function (err, stmt) 
  {
    if(err)
    {
      self.checkConnectionError(err);
      if(cb)
      {
        return cb(err);
      } else
      {
        deferred.reject(err);
      }
    }

    stmt.queue = new SimpleQueue();
    stmt.db = self; // We need a reference to db to check for connection errors

    stmt.prepare(sql, function (err) 
    {
      if (err)
      {
        self.checkConnectionError(err);
        if(cb)
        {
          return cb(err);
        } else
        {
          deferred.reject(err)
        }
      }

      deferred ? deferred.resolve(stmt) : cb(null, stmt);
    });
  });
  return deferred ? deferred.promise : null;
};

Database.prototype.prepareSync = function (sql)
{
  var self = this;
  var stmt = {};

  try {
    stmt = self.conn.createStatementSync();
  } catch (e) {
    self.checkConnectionError(e);
    throw new Error(e);
  }

  stmt.queue = new SimpleQueue();
  stmt.db = self; // We need a reference to db to check for connection errors

  try {
    stmt.prepareSync(sql);
  } catch (e) {
    self.checkConnectionError(e);
    throw new Error(e);
  }

  return stmt;
};

Database.prototype.setIsolationLevel = function(isolationLevel) 
{
  var self = this;
  try {
    self.conn.setIsolationLevel(isolationLevel);
  } catch (e) {
    self.checkConnectionError(e);
    throw new Error(e);
  }
  return true;
};

const MAXINFO_LEN = 256; // odbc.C CLI_MAXINFO_LEN
Database.prototype.getInfoSync = function(infoType, infoLen)
{
  // infoType - mandatory
  // infoLen - optional

  var self = this;
  if (!self.connected)
  {
    throw ({ message : "Connection not open."});
  }
  if (Number.isInteger(infoType) === false) {
    return { message : "Invalid infoType requested."};
  }
  if (Number.isInteger(infoLen) === false) {
    infoLen = MAXINFO_LEN;
  }

  try {
    return self.conn.getInfoSync(infoType, infoLen);
  } catch (e) {
    self.checkConnectionError(e);
    throw new Error(e);
  }
} //getInfoSync

Database.prototype.getInfo = function (infoType, infoLen, cb)
{
  // infoType - mandatory
  // infoLen - optional
  // cb - optional. If not cb, then return promise
 
  var self = this, deferred;

  //support for promises
  if (!cb && typeof infoLen !== 'function')
  {
    deferred = Q.defer();
  }
  // For (infoType, cb)
  if (typeof(infoLen) === 'function')
  {
    cb = infoLen;
    infoLen = MAXINFO_LEN;
  }
  if (Number.isInteger(infoLen) === false) {
    infoLen = MAXINFO_LEN;
  }

  self.queue.push(function (next)
  {
    exports.debug && console.log(getElapsedTime(), "odbc.js: infoType => ", infoType);
    if (!self.connected) {
      deferred ? deferred.reject(connError) : cb(connError);
      return next();
    }

    if (Number.isInteger(infoType) === false) {
      var err = { message : "Invalid infoType used, " + infoType};
      deferred ? deferred.reject(err) : cb(err);
      return next();
    }

    self.conn.getInfo(infoType, infoLen, function (err, value)
    {
      if (err)
      {
        self.checkConnectionError(err);
        deferred ? deferred.reject(err) : cb(err);
        return next();
      }
      deferred ? deferred.resolve(value) : cb(null, value);
      return next();
    });
  });
  if(deferred) return deferred.promise;
} //getInfo

Database.prototype.getTypeInfoSync = function(dataType)
{
  // infoType - mandatory

  var self = this;
  var stmt = 0;
  var data = [];
  if (!self.connected)
  {
    throw ({ message : "Connection not open."});
  }
  if (Number.isInteger(dataType) === false) {
    throw { message : "Invalid dataType used, " + dataType};
  }

  try {
    stmt = self.conn.getTypeInfoSync(dataType);
    data = stmt.fetchAllSync();
    stmt.closeSync();
    return data;
  } catch (e) {
    self.checkConnectionError(e);
    throw new Error(e);
  }
} //getTypeInfoSync

Database.prototype.getTypeInfo = function (dataType, cb)
{
  // dataType - mandatory
  // cb - optional. If not cb, then return promise

  var self = this, deferred;

  if (typeof dataType === 'function')
  {
      cb = dataType;
  }
  //support for promises
  if (!cb && typeof dataType !== 'function')
  {
    deferred = Q.defer();
  }

  self.queue.push(function (next)
  {
    exports.debug && console.log(getElapsedTime(), "odbc.js: dataType => ", dataType);
    if (!self.connected) {
      deferred ? deferred.reject(connError) : cb(connError);
      return next();
    }

    if (Number.isInteger(dataType) === false) {
      var err = { message : "Invalid dataType." };
      if (typeof dataType === 'function') {
        err = { message : "Missing dataType." };
      }
      deferred ? deferred.reject(err) : cb(err);
      return next();
    }

    self.conn.getTypeInfo(dataType, function (err, result)
    {
      if (err) {
        self.checkConnectionError(err);
        deferred ? deferred.reject(err) : cb(err, false);
        return next();
      }

      result.fetchAll(function (err, data)
      {
        result.closeSync();
        if(err) {
            self.checkConnectionError(err);
            deferred ? deferred.reject(err) : cb(err, data);
        } else {
            deferred ? deferred.resolve(data) : cb(err, data);
        }
        return next();
      });
    });
  });
  if(deferred) return deferred.promise;
} //getTypeInfo

Database.prototype.getFunctionsSync = function(functionId)
{
  // functionId - mandatory

  var self = this;
  var fExists = 0;
  if (!self.connected)
  {
    throw ({ message : "Connection not open."});
  }
  if (Number.isInteger(functionId) === false) {
    return { message : "Invalid functionId."};
  }

  try {
    fExists = self.conn.getFunctionsSync(functionId);
  } catch (e) {
    self.checkConnectionError(e);
    throw new Error(e);
  }
  if(fExists)
  {
    if(functionId) return true;
    else
    {
        var functionMap = {};
        var i = 0;
        for ( i = 1; i < 100; i++ )
        {
            if(fExists[i])
            {
                functionMap[macro.functionids[i]] = true;
            }
            else
            {
                functionMap[i] = false;
            }
        }
        return functionMap;
    }
  }
  else return false;
} //getFunctionsSync

Database.prototype.getFunctions = function (functionId, cb)
{
  // functionId - mandatory
  // cb - optional. If not cb, then return promise

  var self = this, deferred;

  if (typeof(functionId) === 'function')
  {
    cb = functionId;
    functionId = null;
  }
  //support for promises
  if (!cb)
  {
    deferred = Q.defer();
  }

  self.queue.push(function (next)
  {
    exports.debug && console.log(getElapsedTime(), "odbc.js: functionId => ", functionId);
    if (!self.connected) {
      deferred ? deferred.reject(connError) : cb(connError);
      return next();
    }

    if (Number.isInteger(functionId) === false) {
      var err = { message : "Invalid functionId = " + functionId};
      deferred ? deferred.reject(err) : cb(err);
      return next();
    }

    self.conn.getFunctions(functionId, function (err, value)
    {
      if (err)
      {
        self.checkConnectionError(err);
        deferred ? deferred.reject(err) : cb(err);
        return next();
      }
      var result = false;
      if(value)
      {
        if(functionId) result = true;
        else
        {
            var functionMap = {};
            var i = 0;
            for ( i = 1; i < 100; i++ )
            {
              if(value[i])
              {
                functionMap[macro.functionids[i]] = true;
              }
              else
              {
                functionMap[i] = false;
              }
            }
            result = functionMap;
        }
      }
      deferred ? deferred.resolve(result) : cb(null, result);
      return next();
    });
  });
  if(deferred) return deferred.promise;
} //getFunctions

// Function to set connection level attributes
Database.prototype.setAttr = function (attr, value, cb)
{
  // attr, value - mandatory
  // cb - optional. If not cb, then return promise

  var self = this, deferred;

  //support for promises
  if (!cb)
  {
    deferred = Q.defer();
  }

  self.queue.push(function (next)
  {
    exports.debug && console.log(getElapsedTime(),
        "odbc.js:setAttr() Attribute = ", attr, ", Value = ", value);
    if (!self.connected) {
      deferred ? deferred.reject(connError) : cb(connError);
      return next();
    }

    if (Number.isInteger(attr) === false) {
      var connattr = macro.connAttributes[attr];
      if (Number.isInteger(connattr) === false) {
        var err = { message : "Invalid attr = " + attr};
        deferred ? deferred.reject(err) : cb(err);
        return next();
      } else {
        attr = connattr;
      }
    }

    self.conn.setAttr(attr, value, function (err)
    {
      if (err)
      {
        self.checkConnectionError(err);
        deferred ? deferred.reject(err) : cb(err);
        return next();
      }
      deferred ? deferred.resolve(true) : cb(null, true);
      return next();
    });
  });
  if(deferred) return deferred.promise;
} //setAttr

// Async Function to set connection level attributes
Database.prototype.setAttrSync = function (attr, value)
{
    // attr, value - mandatory
    var self = this;

    exports.debug && console.log(getElapsedTime(),
        "odbc.js:setAttrSync() Attribute = ", attr, ", Value = ", value);

    if (!self.connected) {
      return (connError);
    }
    if (Number.isInteger(attr) === false) {
      return { message : "Invalid Attribute."};
    }

  try {
      return self.conn.setAttrSync(attr, value);
  } catch (e) {
    self.checkConnectionError(e);
    return new Error(e);
  }
}; // setAttrSync

//Proxy all of the ODBCStatement functions so that they are queued
if( !odbc.ODBCStatement.prototype._execute ) { //issue #514
  odbc.ODBCStatement.prototype._execute = odbc.ODBCStatement.prototype.execute;
  odbc.ODBCStatement.prototype._executeSync = odbc.ODBCStatement.prototype.executeSync;
  odbc.ODBCStatement.prototype._executeDirect = odbc.ODBCStatement.prototype.executeDirect;
  odbc.ODBCStatement.prototype._executeDirectSync = odbc.ODBCStatement.prototype.executeDirectSync;
  odbc.ODBCStatement.prototype._executeNonQuery = odbc.ODBCStatement.prototype.executeNonQuery;
  odbc.ODBCStatement.prototype._executeNonQuerySync = odbc.ODBCStatement.prototype.executeNonQuerySync;
  odbc.ODBCStatement.prototype._prepare = odbc.ODBCStatement.prototype.prepare;
  odbc.ODBCStatement.prototype._bind = odbc.ODBCStatement.prototype.bind;
  odbc.ODBCStatement.prototype._bindSync = odbc.ODBCStatement.prototype.bindSync;
  odbc.ODBCStatement.prototype._setAttr = odbc.ODBCStatement.prototype.setAttr;
  odbc.ODBCStatement.prototype._setAttrSync = odbc.ODBCStatement.prototype.setAttrSync;
  odbc.ODBCStatement.prototype._close = odbc.ODBCStatement.prototype.close;

  //Proxy all of the ODBCResult functions so that they are queued
  odbc.ODBCResult.prototype._fetch = odbc.ODBCResult.prototype.fetch;
  odbc.ODBCResult.prototype._fetchAll = odbc.ODBCResult.prototype.fetchAll;
  odbc.ODBCResult.prototype._getData = odbc.ODBCResult.prototype.getData;
  odbc.ODBCResult.prototype._close = odbc.ODBCResult.prototype.close;
}

odbc.ODBCStatement.prototype.execute = function (params, cb)
{
  var self = this, deferred;
  self.queue = self.queue || new SimpleQueue();

  if (!cb) {
    if (typeof params === 'function') {
      cb = params;
      params = null;
    }
    else {
      deferred = Q.defer();
    }
  }

  self.queue.push(function (next) {
    //If params were passed to this function, then bind them and
    //then execute.
    if (params)
    {
      if(Array.isArray(params))
      {
        var err = parseParams(params);
        if(err)
        {
          if(deferred)
          {
            deferred.reject(err);
          } else
          {
            cb(err);
          }
        }
      }
      if(exports.debug) {
        if(logParams) {
          console.log("%s odbc.js:execute() bind params = ", getElapsedTime(), params);
        } else {
          console.log("%s odbc.js:execute()", getElapsedTime());
        }
      }
      self._bind(params, function (err) {
        if (err) {
          self.db.checkConnectionError(err);
          deferred ? deferred.reject(err) : cb(err);
          return next();
        }

        self._execute(function (err, result, outparams) {
          if (err) {
            self.db.checkConnectionError(err);
          }

          if(deferred) {
            if(err) {
              deferred.reject(err);
            } else {
              if(outparams) {
                deferred.resolve([result, outparams]);
              } else {
                deferred.resolve(result);
              }
            }
          } else {
            cb(err, result, outparams);
          }

          return next();
        });
      });
    }
    //Otherwise execute and pop the next bind call
    else
    {
      self._execute(function (err, result, outparams) {
        if (err) {
          self.db.checkConnectionError(err);
        }

        if(deferred) {
          if(err) {
            deferred.reject(err);
          } else {
              if(outparams) {
                deferred.resolve([result, outparams]);
              } else {
                deferred.resolve(result);
              }
          }
        } else {
          cb(err, result, outparams);
        }

        //NOTE: We only execute the next queued bind call after
        // we have called execute() or executeNonQuery(). This ensures
        // that we don't call a bind() a bunch of times without ever
        // actually executing that bind. Not
        self.bindQueue && self.bindQueue.next();

        return next();
      });
    }
  });
  return deferred ? deferred.promise : null;
};

odbc.ODBCStatement.prototype.executeSync = function (params)
{
  var self = this, err;

  //If params are passed to this function, first bind them and
  //then execute.
  if (params)
  {
      err = self.bindSync(params);
      if(err !== true) console.log(err);
  }
  return self._executeSync();
};

function parseParams(params)
{
    var err, prm, paramtype, ctype, sqltype, datatype, data, len;
    for (var i = 0; i < params.length; i++)
    {
        if(Object.prototype.toString.call(params[i]) == "[object Object]")
        {
            //{ParamType:"INPUT", DataType:"BLOB", Data:imgfile}
            //{"ParamType":"INPUT", CType:"BINARY", SQLType:"BLOB", Data:imgfile, Length:50}
            paramtype = 1; ctype = undefined; sqltype = undefined;
            datatype = undefined; data = undefined, len = 0;
            prm = params[i];
            if(prm.ParamType)
            {
                if(Number.isInteger(prm.ParamType))
                {
                    if(prm.ParamType > 0 && prm.ParamType < 5)
                        paramtype = prm.ParamType;
                }
                else if(prm.ParamType == "OUTPUT")
                    paramtype = 4; // SQL_PARAM_OUTPUT
                else if(prm.ParamType == "INOUT")
                    paramtype = 2; // SQL_PARAM_INPUT_OUTPUT
                else if(prm.ParamType == "FILE")
                    paramtype = 3; // SQLBindFileToParam()
            }

            if(prm.CType)
            {
                if(Number.isInteger(prm.CType))
                {
                    ctype = prm.CType;
                }
                else if(prm.CType == "CHAR")
                    ctype = 1;
                else if(prm.CType == "BINARY")
                    ctype = -2;
                else if(prm.CType == "DOUBLE" || prm.CType == "DECIMAL")
                    ctype = 8; // SQL_C_DOUBLE=8, SQL_DECIMAL=3
                else if(prm.CType == "INTEGER")
                    ctype = 4;
                else if(prm.CType == "BIGINT")
                    ctype = -25;
            }

            // Either SQLType or DataType must be entered.
            // If SQLType is used, DataType will be ignored.
            if(prm.SQLType || prm.DataType)
            {
                var type = prm.SQLType || prm.DataType;
                if(Number.isInteger(type))
                {
                    sqltype = type;
                }
                else if(type == "DOUBLE")
                {
                    sqltype = 8;                 // SQL_DOUBLE
                    if(!ctype) ctype = sqltype;
                }
                else if(type == "DECIMAL")
                {
                    sqltype = 3;                 // SQL_DECIMAL
                    if(!ctype) ctype = 8;        // sql_C_DOUBLE
                }
                else if(type == "BIGINT")
                {
                    sqltype = -5;                 // SQL_BIGINT
                    if(!ctype) ctype = -25;       // sql_C_BIGINT
                }
                else if(type == "CHAR")
                {
                    sqltype = 1;                 // SQL_CHAR
                    if(!ctype) ctype = sqltype;
                }
                else if(type == "BINARY")
                {
                    sqltype = -2;
                    if(!ctype) ctype = sqltype;
                }
                else if(type == "BLOB")
                {
                    sqltype = -98;               // SQL_BLOB
                    if(!ctype) ctype = -2;
                }
                else if(type == "CLOB")
                {
                    sqltype = -99;
                    if(!ctype) ctype = 1;
                }
                else if(type == "DBCLOB")
                {
                    sqltype = -350;
                    if(!ctype) ctype = sqltype;
                }
                else if(type == "XML")
                {
                    sqltype = -370;
                    if(!ctype) ctype = 1;
                }
                else if(type == "GRAPHIC")
                {
                    sqltype = -95;
                    if(!ctype) ctype = -99; //SQL_C_DBCHAR
                }
                else if(type == "VARGRAPHIC")
                {
                    sqltype = -96;
                    if(!ctype) ctype = -99; //SQL_C_DBCHAR
                }
                else if(type == "LONGGRAPHIC")
                {
                    sqltype = -97;
                    if(!ctype) ctype = -99; //SQL_C_DBCHAR
                }
            }
            if(prm.Length > 0 && Number.isInteger(prm.Length))
            {
                len = prm.Length;
            }

            if(prm.Data === undefined )
            {
                err = "odbc.js:parseParams =>Data is missing from " + JSON.stringify(prm);
                break;
            }
            if(Number.isInteger(prm.Data))
            {
                if(!ctype) ctype = -25; // SQL_C_SBIGINT
                if(!sqltype) sqltype = -5; // SQL_BIGINT

            }
            else if( (paramtype == 4) && (prm.Data === "") )
            {
                prm.Data = "abc";
            }

            if(!ctype) ctype = 1;
            if(!sqltype) sqltype = 1;
            if(paramtype == 3) // For BindFileToParam()
            {
                ctype = 1;
                if(!((sqltype == -98) || (sqltype == -99) ||
                     (sqltype == -350) || (sqltype == -370)))
                {
                    err = "odbc.js:parseParams => DataType is missing form " + prm;
                    break;
                }
            }

            params[i] = [paramtype, ctype, sqltype, prm.Data, len];
        }
        //console.log(i + "th param = " + params[i]);
    }
    return err;
}

if(Number.isInteger === undefined)   // node.js < v0.12.0 do not support isInteger
{
    Number.isInteger = function(x)
    {
        if((typeof x === 'number') && (x % 1 === 0)) return true;
        else return false;
    };
}

odbc.ODBCStatement.prototype.executeDirect = function (sql, cb) 
{
  var self = this, deferred;

  if(!cb) {
      deferred = Q.defer();
  }
  self.queue = self.queue || new SimpleQueue();

  self.queue.push(function (next) {
    self._executeDirect(sql, function (err, result) {
        if(err) {
            deferred ? deferred.reject(err) : cb(err, result);
        } else {
            deferred ? deferred.resolve(result) : cb(err, result);
        }

      return next();
    });
  });
  return deferred ? deferred.promise : null;
};

odbc.ODBCStatement.prototype.executeNonQuery = function (params, cb) 
{
  var self = this, deferred;

  if (!cb) {
    if (typeof params === 'function') {
      cb = params;
      params = null;
    }
    else {
      deferred = Q.defer();
    }
  }
  self.queue = self.queue || new SimpleQueue();

  self.queue.push(function (next) {
    //If params were passed to this function, then bind them and
    //then executeNonQuery.
    if (params) 
    {
      if(Array.isArray(params))
      {
        var err = parseParams(params);
        if(err) deferred ? deferred.reject(err) : cb(err);
      }
      if(exports.debug) {
        if(logParams) {
          console.log("%s odbc.js:executeNonQuery() bind params = ", getElapsedTime(), params);
        } else {
          console.log("%s odbc.js:executeNonQuery()", getElapsedTime());
        }
      }
      self._bind(params, function (err) {
        if (err) {
          if(deferred)
          {
            deferred.reject(err);
          } 
          else
          {
            cb && cb(err)
          }
          return next();
        }

        self._executeNonQuery(function (err, result) {
          if(deferred) {
            if(err) {
              deferred.reject(err);
            } else {
              deferred.resolve(result);
            }
          } else {
            cb &&  cb(err, result);
          }
          return next();
        });
      });
    }
    //Otherwise executeNonQuery and pop the next bind call
    else {
      self._executeNonQuery(function (err, result) {
        if(deferred) {
          if(err) {
            deferred.reject(err);
          } else {
            deferred.resolve(result);
          }
        } else {
          cb && cb(err, result);
        }

        //NOTE: We only execute the next queued bind call after
        // we have called execute() or executeNonQuery(). This ensures
        // that we don't call a bind() a bunch of times without ever
        // actually executing that bind.
        self.bindQueue && self.bindQueue.next();

        return next();
      });
    }
  });
  return deferred ? deferred.promise : null;
};

odbc.ODBCStatement.prototype.executeNonQuerySync = function (params)
{
  var self = this, err;

  //If params are passed to this function, first bind them and
  //then execute.
  if (params)
  {
      err = self.bindSync(params);
      if(err !== true) console.log(err);
  }
  return self._executeNonQuerySync();
};

odbc.ODBCStatement.prototype.prepare = function (sql, cb) {
  var self = this, deferred;

  if(!cb) {
      deferred = Q.defer();
  }
  self.queue = self.queue || new SimpleQueue();

  self.queue.push(function (next) {
    try {
      self._prepare(sql, function (err) {
        if(cb) {
          cb(err);
        } else {
          err ? deferred.reject(err) : deferred.resolve(true);
        }
      });
    } catch (ex) {
        cb ? cb(ex) : deferred.reject(ex);
    }
    return next();
  });
  return deferred ? deferred.promise : null;
};

// Function to bind parameters before execute or executeSync
odbc.ODBCStatement.prototype.bind = function (ary, cb) {
  var self = this, deferred;

  if(!cb) {
    if (typeof ary === 'function') {
      cb = ary;
      ary = null;
    }
    else {
      deferred = Q.defer();
    }
  }
  self.bindQueue = self.bindQueue || new SimpleQueue();

  self.bindQueue.push(function () {
    if(Array.isArray(ary))
    {
      var err = parseParams(ary);
      if(err) deferred ? deferred.reject(err) : cb(err);
    }
    if(exports.debug) {
      if(logParams) {
        console.log("%s odbc.js:bind() params = ", getElapsedTime(), ary);
      } else {
        console.log("%s odbc.js:bind()", getElapsedTime());
      }
    }
    self._bind(ary, function (err) {
      if(cb) {
        cb(err);
      } else {
        err ? deferred.reject(err) : deferred.resolve(true);
      }

      //NOTE: we do not call next() here because
      //we want to pop the next bind call only
      //after the next execute call
    });
  });
};

// Async Function to bind parameters before execute or executeSync
odbc.ODBCStatement.prototype.bindSync = function (ary) {
    var self = this;
    if(Array.isArray(ary))
    {
      var err = parseParams(ary);
      if(err) return false;
    }
    if(exports.debug) {
      if(logParams) {
        console.log("%s odbc.js::bindSync() params = ", getElapsedTime(), ary);
      } else {
        console.log("%s odbc.js:bindSync()", getElapsedTime());
      }
    }
    return self._bindSync(ary); 
};

// Function to set statement attributes before execute or executeSync
odbc.ODBCStatement.prototype.setAttr = function (attr, value, cb) {
  var self = this, deferred;

  if(!cb) {
      deferred = Q.defer();
  }
  self.setAttrQueue = self.setAttrQueue || new SimpleQueue();

  self.setAttrQueue.push(function () {
    exports.debug && console.log("%s odbc.js:setAttr() Attribute = ", getElapsedTime(), attr, ", value = ", value);
    self._setAttr(attr, value, function (err) {
      if(cb) {
        cb(err);
      } else {
        err ? deferred.reject(err) : deferred.resolve(true);
      }
    });
  });
  return deferred ? deferred.promise : null;
};

// Async Function to set statement attributes  before execute or executeSync
odbc.ODBCStatement.prototype.setAttrSync = function (attr, value) {
    var self = this;
    exports.debug && console.log("%s odbc.js:setAttrSync() Attribute = ", getElapsedTime(), attr, ", Value = ", value);
    return self._setAttrSync(attr, value); 
};

// Async funcion to close a statement object to free statement handle
// If no callback function, then return Promise.
odbc.ODBCStatement.prototype.close = function (closeOption, cb)
{
  var self = this, deferred;
  self.queue = self.queue || new SimpleQueue();

  if (!cb)
  {
      if (typeof closeOption === 'function') {
          cb = closeOption;
          closeOption = 1;
      } else {
        deferred = Q.defer();
      }
      if(!closeOption) closeOption = 1;
  }

  self.queue.push(function (next) {
    self._close(closeOption, function (err) {
      if( err ) {
          deferred ? deferred.reject(err) : cb(err);
      } else {
          deferred ? deferred.resolve(false) : cb(null);
      }

      return next();
    });
  });
  return deferred ? deferred.promise : false;
};

// Async function to fetch single row of data post execute or executeSync call.
// result.fetch(option, callback);
// result.fetch({fetchMode:4|3|0}); default value of fetchMode is 4 - OBJECT
// fetchMode:0 => will not return any data, we need to retrieve data using
// getData() or getDataSync() APIs.
// If callback function is not provided, it will return Promise.
// Below is the Promisified implementation of fetch() function.
odbc.ODBCResult.prototype.fetch = function (option, cb) {
  var self = this, deferred;
  self.queue = self.queue || new SimpleQueue();

  if (!cb)
  {
    if(typeof option === 'function')
    {
      cb = option;
      option = undefined;
    }
    else
    {
      deferred = Q.defer();
    }
  }
  if (typeof option === 'object')
  {
    if (option.fetchMode !=0 && option.fetchMode !=3 && option.fetchMode !=4)
    {
      var error = "Invalid fetchMode!";
      !cb ? deferred.reject(error) : cb(error);
    } else {
      self.fetchMode = option.fetchMode;
    }
  } else {
    option = undefined;
  }

  if (option)
  {
    self.queue.push(function (next) {
      self._fetch(option, function (err, data) {
        if( err ) {
          deferred ? deferred.reject(err) : cb(err, null);
        } else {
          deferred ? deferred.resolve(data) : cb(null, data);
        }
        return next();
      });
    });
  }
  else
  {
    self.queue.push(function (next) {
      self._fetch(function (err, data) {
        if( err ) {
          deferred ? deferred.reject(err) : cb(err, null);
        } else {
          deferred ? deferred.resolve(data) : cb(null, data);
        }
        return next();
      });
    });
  }
  return deferred ? deferred.promise : false;
};

odbc.ODBCResult.prototype.fetchAll = function (option, cb)
{
  var self = this, deferred;

  if (!cb)
  {
    if(typeof option === 'function')
    {
      cb = option;
      option = undefined;
    }
    else
    {
      deferred = Q.defer();
    }
  }
  if (typeof option === 'object')
  {
    if (option.fetchMode !=3 && option.fetchMode !=4)
    {
      var error = "Invalid fetchMode!";
      !cb ? deferred.reject(error) : cb(error);
    } else {
      self.fetchMode = option.fetchMode;
    }
  } else {
    option = undefined;
  }

  if (option)
  {
      self._fetchAll(option, function (err, data, count) {
        if( err ) {
          deferred ? deferred.reject(err) : cb(err, null, 0);
        } else {
          deferred ? deferred.resolve(data) : cb(null, data, count);
        }
      });
  }
  else
  {
      self._fetchAll(function (err, data, count) {
        if( err ) {
          deferred ? deferred.reject(err) : cb(err, null, 0);
        } else {
          deferred ? deferred.resolve(data) : cb(null, data, count);
        }
      });
  }
  return deferred ? deferred.promise : false;
};

// Async funcion to getData for a column after call of fetch with fetchMode:0
// If no callback function, then return Promise.
odbc.ODBCResult.prototype.getData = function (colNum, dataSize, cb)
{
  var self = this, deferred;

  if (!cb && typeof colNum !== 'function' && typeof dataSize !== 'function')
  {
      deferred = Q.defer();
  }
  self.queue = self.queue || new SimpleQueue();

  if (!cb && typeof dataSize === 'function')
  {
    cb = dataSize;
    dataSize = 0;
  }
  if (!colNum)
  {
      colNum = 0;
  }

  self.queue.push(function (next) {
    self._getData(colNum, dataSize, function (err, data) {
      if( err ) {
          deferred ? deferred.reject(err) : cb(err, null);
      } else {
          deferred ? deferred.resolve(data) : cb(null, data);
      }

      return next();
    });
  });
  return deferred ? deferred.promise : false;
};

// Async funcion to close a result object to free statement handle
// If no callback function, then return Promise.
odbc.ODBCResult.prototype.close = function (closeOption, cb)
{
  var self = this, deferred;
  self.queue = self.queue || new SimpleQueue();

  if (!cb)
  {
      if (typeof closeOption === 'function') {
          cb = closeOption;
          closeOption = 1;
      } else {
        deferred = Q.defer();
      }
      if(!closeOption) closeOption = 1;
  }

  self.queue.push(function (next) {
    self._close(closeOption, function (err) {
      if( err ) {
          deferred ? deferred.reject(err) : cb(err);
      } else {
          deferred ? deferred.resolve(false) : cb(null);
      }

      return next();
    });
  });
  return deferred ? deferred.promise : false;
};


/***** CONNECTION POOLING RELATED APIs *******/
/*===========================================*/

module.exports.Pool = Pool;

Pool.count = 0;

function Pool (_options) {
  var self = this;
  self.options = {};
  self.maxPoolSize = 0;
  if(_options) 
  {
    if(_options.idleTimeout && !isNaN(_options.idleTimeout))
      self.options.idleTimeout = _options.idleTimeout;
    if(_options.autoCleanIdle)
      self.options.autoCleanIdle = _options.autoCleanIdle;
    if(_options.maxPoolSize)
      self.maxPoolSize = _options.maxPoolSize;
    if(_options.connectTimeout)
      self.options.connectTimeout = _options.connectTimeout;
    if(_options.systemNaming)
      self.options.systemNaming=_options.systemNaming;
  }
  self.index = Pool.count++;
  self.availablePool = {};
  self.usedPool = {};
  self.poolSize = 0;
  self.queue = self.queue || new SimpleQueue();
  self.queue.executing = true; // Do not call next, untill next is called by db.close();
  if(!ENV) ENV = new odbc.ODBC();
  self.odbc = ENV;
  self.options.connectTimeout = self.options.connectTimeout || 60;
}

// Get a new connection from pool asynchronously
Pool.prototype.open = function (connStr, callback)
{
  var self = this
    , db, deferred
    ;

  if (!callback)
  {
    deferred = Q.defer();
  }
  //check to see if we already have a connection for this connection string
  if (self.availablePool[connStr] && self.availablePool[connStr].length)
  {
    // Remove connection if marked as not connected.
    do {
      db = self.availablePool[connStr].shift();
      if (db && !db.connected) {
        db = null;
        if(self.poolSize) self.poolSize--;
      }
      else { break; }
    } while (!db);
  }
  if (db) {
    db.lastUsed=null;
    self.usedPool[connStr] = self.usedPool[connStr] || [];
    self.usedPool[connStr].push(db);
    deferred ? deferred.resolve(db) : callback(null, db);
  }
  else if((self.maxPoolSize > 0) && (self.poolSize >= self.maxPoolSize))
  {
    // Move it to queue untill an existing connection is not closed.
    // Call it from db.close() version of pool.open and pool.init.
    function queueCallback(next) {
      if (self.availablePool[connStr] && self.availablePool[connStr].length)
      {
        db = self.availablePool[connStr].shift();
        db.lastUsed=null;
        self.usedPool[connStr] = self.usedPool[connStr] || [];
        self.usedPool[connStr].push(db);
        deferred ? deferred.resolve(db) : callback(null, db);
      }
      else {
        self.queue.push(queueCallback);
      }
    }
    self.queue.push(queueCallback);
  }
  else
  {
    if(typeof(self.options.odbc) === 'undefined')
        self.options.odbc = self.odbc;
    self.options.pool = self;
    self.options.connStr = connStr;
    db = new Database(self.options);
    self.poolSize++;

    db.open(connStr, function (error) {
      exports.debug && console.log("%s odbc.js : pool[%s] : pool.db.open new connection.", getElapsedTime(), self.index);
      if(error)
      {
        self.poolSize--;
        deferred ? deferred.reject(error) : callback(error, db);
      }
      else
      {
        self.usedPool[connStr] = self.usedPool[connStr] || [];
        db.created = Date.now();
        self.usedPool[connStr].push(db);
        deferred ? deferred.resolve(db) : callback(error, db);
      }
    }); //db.open
  }
  return deferred ? deferred.promise : null;
};

// Get a new connection from pool synchronously
Pool.prototype.openSync = function (connStr)
{
  var self = this
    , db
    ;

  //check to see if we already have a connection for this connection string
  if (self.availablePool[connStr] && self.availablePool[connStr].length)
  {
    // Remove connection if marked as not connected.
    do {
      db = self.availablePool[connStr].shift();
      if (db && !db.connected) {
        db = null;
        if(self.poolSize) self.poolSize--;
      }
      else { break; }
    } while (!db);
  }
  if (db) {
    db.lastUsed=null;
    self.usedPool[connStr] = self.usedPool[connStr] || [];
    self.usedPool[connStr].push(db);
    return db;
  }
  else if((self.maxPoolSize > 0) && (self.poolSize >= self.maxPoolSize))
  {
    // Move it to queue untill an existing connection is not closed.
    // Call it from db.close() version of pool.open and pool.init.
    function queueCallback(next) {
      if (self.availablePool[connStr] && self.availablePool[connStr].length)
      {
        db = self.availablePool[connStr].shift();
        db.lastUsed=null;
        self.usedPool[connStr] = self.usedPool[connStr] || [];
        self.usedPool[connStr].push(db);
        return db;
      }
      else {
        self.queue.push(queueCallback);
      }
    }
    self.queue.push(queueCallback);
  }
  else
  {
    if(typeof(self.options.odbc) === 'undefined')
        self.options.odbc = self.odbc;
    self.options.pool = self;
    self.options.connStr = connStr;
    db = new Database(self.options);

    exports.debug && console.log("%s odbc.js : pool[%s] : pool.db.openSync new connection.", getElapsedTime(), self.index);
    db.openSync(connStr);
    self.poolSize++;
    self.usedPool[connStr] = self.usedPool[connStr] || [];
    db.created = Date.now();
    self.usedPool[connStr].push(db);
    return db;
  }
}; //pool.openSync

// closeSync function for pooled connection.
Database.prototype.poolCloseSync = function ()
{
    var db = this;
    var self = this.pool;
    var connStr = this.connStr;
    if( !self ) { return db.closeSync(); }

    self.availablePool[connStr] = self.availablePool[connStr] || [];
    // If conn.closeSync is called more than once for this connection,
    // nothing to do. Just return.
    if( self.availablePool[connStr] &&
        self.availablePool[connStr].indexOf(db) >= 0 )
    {
        return;
    }
    db.lastUsed = Date.now();

    // If this connection has some active transaction, rollback the
    // transaction to free up the held resorces before moving back to
    // the pool. So that, next request can get afresh connection from pool.
    if(db.conn && db.conn.inTransaction)
    {
      try {
        db.rollbackTransactionSync();
      } catch(error) {
          // Just ignore the error, since we're closing the connection.
      }
    }

    //remove this db from the usedPool
    if(self.usedPool[connStr]) {
      self.usedPool[connStr].splice(self.usedPool[connStr].indexOf(db), 1);
      self.poolSize--;
    }

    //move this connection back to the connection pool at the end.
    if(db.conn)
    {
      self.availablePool[connStr].push(db);
      self.poolSize++;
    }

    if(db.conn && self.options.autoCleanIdle) self.cleanUp(connStr);
    return self.queue.next();
};  // db.poolCloseSync function

// conn.close function for pooled connection.
Database.prototype.poolClose = function (cb)
{
    var db = this;
    var self = this.pool;
    var connStr = this.connStr;
    if( !self ) { return db.close(cb); }
    var deferred;
    if(!cb) {
        deferred = Q.defer();
    }

    self.availablePool[connStr] = self.availablePool[connStr] || [];
    // If conn.close is called more than once for this connection,
    // nothing to do. Just call the callback function and return.
    if( self.availablePool[connStr] &&
        self.availablePool[connStr].indexOf(db) >= 0 )
    {
        deferred ? deferred.resolve(true) : cb(null);
        return;
    }
    db.lastUsed = Date.now();

    // If this connection has some active transaction, rollback the
    // transaction to free up the held resorces before moving back to
    // the pool. So that, next request can get afresh connection from pool.
    if(db.conn && db.conn.inTransaction)
    {
        db.rollbackTransaction(function(err){});
    }

    //remove this db from the usedPool
    if(self.usedPool[connStr]) {
      self.usedPool[connStr].splice(self.usedPool[connStr].indexOf(db), 1);
      self.poolSize--;
    }

    //move this connection back to the connection pool at the end.
    if(db.conn)
    {
      self.availablePool[connStr].push(db);
      self.poolSize++;
    }
    deferred ? deferred.resolve(true) : cb(null);

    if(db.conn && self.options.autoCleanIdle) self.cleanUp(connStr);
    //if(exports.debug) {
        //process.stdout.write(getElapsedTime());
        //console.log(util.inspect(self, {depth: null}));
    //}
    return self.queue.next();
};  // poolClose function

Pool.prototype.init = function(count, connStr)
{
  var self = this;
  var ret = false;
  exports.debug && console.log(getElapsedTime(), "Max pool size at start of init = " + self.maxPoolSize);

  //check to see if we already have a connection for this connection string
  if (self.availablePool[connStr] && self.availablePool[connStr].length)
  {
    console.log("Pool is already initialized and it has "+
           self.availablePool[connStr].length + " available connections.\n");
    return;
  }
  else if(count < 1) {
    let err = {};
    err["message"] = "Error: Count should be greater than 0. Pool not initialized.";
    exports.debug && console.log(getElapsedTime(), err.message);
    return err;
  }
  else
  {
    if((self.maxPoolSize > 0) && (count > self.maxPoolSize))
    {
        console.log(getElapsedTime(), " ** Can not open connection more than max pool size.\n");
        count = self.maxPoolSize;
    }

    if(typeof(self.options.odbc) === 'undefined')
        self.options.odbc = self.odbc;
    self.options.pool = self;
    self.options.connStr = connStr;
    for(var i = 0; i < count; i++)
    {
        var db = new Database(self.options);
        self.poolSize++;
        try{
            ret = db.openSync(connStr);
        } catch (ret) {
            self.poolSize--;
            exports.debug && console.log("%s odbc.js: %d connection(s) initialized.\n", getElapsedTime(), self.poolSize);
            return ret;
        }
        if(ret !== true) break; 
        exports.debug && console.log("%s odbc.js : pool[%s] : pool.init %d", getElapsedTime(), self.index, i);

        self.availablePool[connStr] = self.availablePool[connStr] || [];
        db.created = Date.now();

        db.availablePool = self.availablePool[connStr] || [];
        self.availablePool[connStr].push(db);
    }
    self.usedPool[connStr] = self.usedPool[connStr] || [];
    exports.debug && console.log(getElapsedTime(), "Max pool size = " + self.maxPoolSize);
    exports.debug && console.log(getElapsedTime(), "Pool = ", self);
    return ret;
  }
};// Pool.init()

// Initialize pool asynchronously
Pool.prototype.initAsync = function(count, connStr, cb)
{
  var self = this, deferred;
  var ret = false;
  exports.debug && console.log(getElapsedTime(), "Max pool size at start of init = " + self.maxPoolSize);

  if (typeof(connStr) === "object")
  {
    let obj = connStr;
    connStr = "";

    Object.keys(obj).forEach(function (key) {
      connStr += key + "=" + obj[key] + ";";
    });
  }

  if (!cb)
  {
    deferred = Q.defer();
  }
  //check to see if we already have a connection for this connection string
  if (self.availablePool[connStr] && self.availablePool[connStr].length)
  {
    console.log("Pool is already initialized and it has "+
           self.availablePool[connStr].length + " available connections.\n");
    deferred ? deferred.resolve(true) : cb(null);
  }
  else if(count < 1) {
    let err = {};
    err["message"] = "Error: Count should be greater than 0. Pool not initialized.";
    exports.debug && console.log(getElapsedTime(), err.message);
    deferred ? deferred.reject(err) : cb(err);
  }
  else
  {
    if((self.maxPoolSize > 0) && (count > self.maxPoolSize))
    {
        console.log(getElapsedTime(), " ** Can not open connection more than max pool size.\n");
        count = self.maxPoolSize;
    }

    if(typeof(self.options.odbc) === 'undefined')
        self.options.odbc = self.odbc;
    self.options.pool = self;
    self.options.connStr = connStr;
    self.availablePool[connStr] = self.availablePool[connStr] || [];
    self.usedPool[connStr] = self.usedPool[connStr] || [];

    for(var i = 0; i < count; i++)
    {
      (function (connNo) {
        var db = new Database(self.options);
        self.poolSize++;
        db.open(connStr, function(error) {
          if(error) {
            self.poolSize--;
          } else {
            exports.debug && console.log("%s odbc.js: %d connections initialized.\n",
                                         getElapsedTime(), self.poolSize);
            exports.debug && console.log("%s odbc.js : pool[%s] : pool.init %d",
                                         getElapsedTime(), self.index, connNo);

            db.created = Date.now();
            db.availablePool = self.availablePool[connStr];
            self.availablePool[connStr].push(db);
          }
          if(connNo == count) {
            exports.debug && console.log(getElapsedTime(), "Pool = ", self);
            if(error) {
              deferred ? deferred.reject(error) : cb(error);
            } else {
              deferred ? deferred.resolve(true) : cb(null);
            }
          }
        });
      })(i+1);
    }
  }
  return deferred ? deferred.promise : null;
};// Pool.initAsync()

// No of active connections in pool should not grow more than maxPoolSize.
// Run test/test-max-pool-size.js to test this functionality.
Pool.prototype.setMaxPoolSize = function(size)
{
    var self = this;
    self.maxPoolSize = size;
    exports.debug && console.log(getElapsedTime(), "Max pool size is set to " + self.maxPoolSize);
    return true;
};

Pool.prototype.setConnectTimeout = function(timeout)
{
    // Don't use this API. Will be removed in next major release.
    // It is just a place holder as of now to avoid breaking of 
    // existing applications that uses it.

    // Now there is no timeout for connection request from pool.
    // Once conn.close() get called for existing pooled connection,
    // the waiting connection request will get that connection from
    // pool as connection pooling uses now FIFO queue for new
    // connections beyond maxPoolSize.
    return true;
};

// Close idle connections
Pool.prototype.cleanUp = function(connStr) {
  var self = this;
  if(self.availablePool[connStr].length < 2) return;

  self.availablePool[connStr] = self.availablePool[connStr].filter(function(conn) 
  {
    if(conn.lastUsed && 
       (Date.now()-conn.lastUsed > (self.options.idleTimeout || 1800 * 1000)) && 
       conn.realClose ) 
    {
       conn.realClose(function() 
       {
         if(self.poolSize) self.poolSize--;
         exports.debug && console.log("odbc.js : pool[%s] : Pool.cleanUp() : " +
                 "pool.realClose() : Connection duration : %s", self.index, 
                 (Date.now() - conn.created)/1000);
       });
       return false;
    }
    else 
    {
      return true;
    }
  });
}; //Pool.cleanUp()

Pool.prototype.close = function (callback)
{
  var self = this
    , required = 0
    , received = 0
    , connections
    , key
    , x
    , deferred
    ;

    if(!callback) {
        deferred = Q.defer();
    }
  exports.debug && console.log("%s odbc.js : pool[%s] : pool.close()", getElapsedTime(), self.index);
  //we set a timeout because a previous db.close() may
  //have caused db.open() behind the scenes to prepare
  //a new connection
  setTimeout(function () {
    //merge the available pool and the usedPool
    var pools = {};

    for (key in self.availablePool)
    {
      pools[key] = (pools[key] || []).concat(self.availablePool[key]);
    }

    for (key in self.usedPool)
    {
      pools[key] = (pools[key] || []).concat(self.usedPool[key]);
    }

    exports.debug && console.log("%s odbc.js : pool[%s] : pool.close() - setTimeout() callback", getElapsedTime(), self.index);
    //console.dir(pools);

    if (Object.keys(pools).length === 0)
    {
      if (callback) return callback();
      else return deferred.resolve(true);
    }

    for (key in pools)
    {
      connections = pools[key];
      var connLen = connections.length;
      required += connLen;

      if(exports.debug) { 
        console.log("%s odbc.js : pool[%s] : pool.close() - processing pools - connections: %s",
                    getElapsedTime(), self.index, connections.length);
      }

      for (x = 0 ; x < connLen; x ++)
      {
        (function (x) {
          //call the realClose method to avoid
          //automatically re-opening the connection
          if(exports.debug) { 
            console.log("%s odbc.js : pool[%s] : pool.close() - calling realClose() for connection #%s", 
                        getElapsedTime(), self.index, x + 1);
          }

          if(connections[x].realClose) {
           connections[x].realClose(function () {
            if(exports.debug) { 
              console.log("%s odbc.js : pool[%s] : pool.close() - realClose() callback for connection #%s", 
                          getElapsedTime(), self.index, x + 1);
            }
            module.exports.close(connections[x]);
            received += 1;
            if(self.poolSize) self.poolSize--;

            if (received === required)
            {
              deferred ? deferred.resolve(true) : callback();

              //prevent mem leaks
              self = null;
              connections = null;
              required = null;
              received = null;
              key = null;

              return;
            }
           }); // connections[x].realClose i.e. conn.close().
          }
          else {
              exports.debug && console.log("%s realClose is not a member of connection %s", getElapsedTime(), x + 1);
          }
        })(x);
      } //for loop.
    } //for (key in pools)
  }, 2000);  //setTimeout
  return deferred ? deferred.promise : null;
};  //Pool.close()

Pool.prototype.closeSync = function ()
{
    var self = this
      , connections
      , key
      , x, error
      ;

    exports.debug && console.log("%s odbc.js : pool[%s] : pool.closeSync()", getElapsedTime(), self.index);

    //merge the available pool and the usedPool
    var pools = {};

    for (key in self.availablePool)
    {
      pools[key] = (pools[key] || []).concat(self.availablePool[key]);
    }
    for (key in self.usedPool)
    {
      pools[key] = (pools[key] || []).concat(self.usedPool[key]);
    }

    if (Object.keys(pools).length === 0)
    {
      return null;
    }

    for (key in pools)
    {
      connections = pools[key];
      var connLen = connections.length;

      if(exports.debug) {
        console.log("%s odbc.js : pool[%s] : pool.closeSync() - processing pools - connections: %s",
                    getElapsedTime(), self.index, connLen);
      }

      for (x = 0 ; x < connLen; x ++)
      {
          if(exports.debug) {
            console.log("%s odbc.js : pool[%s] : pool.closeSync() - calling realClose() for connection #%s",
                        getElapsedTime(), self.index, x + 1);
          }

          if(connections[x].realCloseSync) {
            error = connections[x].realCloseSync();
            module.exports.close(connections[x]);
            if(self.poolSize) self.poolSize--;
          }
          else {
              exports.debug && console.log("%s realCloseSync is not a member of connection %s", getElapsedTime(), x + 1);
          }
      }
    } //for (key in pools)
    self = null;
    return error;
};  //Pool.closeSync()


// Add attributes from climacros.js
var attr;
for (attr in macro.attributes) {
    module.exports[attr] = macro.attributes[attr];
}


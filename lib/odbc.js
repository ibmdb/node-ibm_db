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
if(os.platform() == 'win32')
{
  process.env.PATH = process.env.PATH + ';' +
                     path.resolve(__dirname, '../installer/clidriver/bin') + ';' +
                     path.resolve(__dirname, '../installer/clidriver/lib');
  process.env.LIB = process.env.LIB + ';' +
                     path.resolve(__dirname, '../installer/clidriver/bin') + ';' +
                     path.resolve(__dirname, '../installer/clidriver/lib');
}

var odbc = require("bindings")("odbc_bindings")
  , SimpleQueue = require("./simple-queue")
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
    }
    else {
        exports.debug = false;
        console.log("node-ibm_db logs disabled.");
    }
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
    var key;
    for(key in db)
    {
      delete db[key];
    }
    delete db;
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

        if (cb)
        {
          cb(err);
        } else
        {
          deferred.resolve(err);
        }
        return next();
      });
    }
    else
    {
      self.connected = false;
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

  return result
}; // closeSync

Database.prototype.query = function (query, params, cb)
{
  var self = this, deferred, sql, resultset = [], multipleResultSet = false;
  var sqlca = {state:"0", sqlcode: 0};

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
        if(initialErr && initialErr.state) {
          sqlca.state = initialErr.state;
        }
        if(initialErr && initialErr.sqlcode) {
          sqlca.sqlcode = initialErr.sqlcode;
        }
        // For pooled connection, if we get SQL30081N, then close
        // the connection now only and then proceed.
        if(self.realClose){
          if((initialErr && initialErr['message'] &&
              initialErr['message'].search("SQL30081N") != -1))
          {
            // Close all connections in availablePool as all are invalid now
            for (var i=0, len = self.availablePool.length; i < len; i++) {
              self.availablePool[i].closeSync();
            }
            self.closeSync();
          }
        }
        deferred ? deferred.reject(initialErr, resultset, sqlca) :
                   cb(initialErr, resultset, sqlca);
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

        result.fetchAll(function (err, data, colcount) {
          var moreResults = false, moreResultsError = null;

          // If there is any error, return it now only.
          if( err || initialErr )
          {
            // For pooled connection, if we get SQL30081N, then close
            // the connection now only and then proceed.
            if(self.realClose){
              if((err && err['message'] &&
                      err['message'].search("SQL30081N") != -1) ||
                 (initialErr && initialErr['message'] &&
                  initialErr['message'].search("SQL30081N") != -1))
              {
                // Close all connections in availablePool as all are invalid now
                for (var i=0, len = self.availablePool.length; i < len; i++) {
                  self.availablePool[i].closeSync();
                }
                self.closeSync();
              }
            }
            if(multipleResultSet) resultset.push(data);
            else resultset = data;
            err = initialErr || err;
            sqlca.state = err.state || sqlca.state;
            sqlca.sqlcode = err.sqlcode || sqlca.sqlcode;
            console.log(err, sqlca);
            deferred ? deferred.reject(err) : cb(err, resultset, sqlca);
            if (result) { result.closeSync(); }
            initialErr = null;
            err = null;
            return next();
          }

          // Get the result data
          try
          {
            if(colcount)  // Check for more result set.
              moreResults = result.moreResultsSync();
          }
          catch (e)
          {
            moreResultsError = e;
            moreResults = false;
            sqlca.state = e.state || sqlca.state;
            sqlca.sqlcode = e.sqlcode || sqlca.state;
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
            !cb ? deferred.resolve(moreResultsError || resultset, sqlca) : cb(moreResultsError, resultset, sqlca);
          }
          
          moreResultsError = null;
          return next();
        });
      } // function fetchMore
    } //function cbQuery

    if (!self.connected)
    {
      var err = {message : "Connection not open.",
                 sqlstate: "08001",
                 sqlcode : -30081};
      sqlca = {sqlstate: "08001", sqlcode : -30081};
      deferred ? deferred.reject(err) : cb(err, [], sqlca);
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
    if (params && params.length > 0)
    {
      if(Array.isArray(params))
      {
        var err = parseParams(params);
        if(err) deferred ? deferred.reject(err) : cb(err);
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
      var err = {message : "Connection not open.",
                 sqlstate: "08001",
                 sqlcode : -30081};
      deferred ? deferred.reject(err) : cb(err);
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

      !cb ? deferred.resolve(result, outparams) : cb(err, result, outparams);

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
  { return e; }

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
  { return e; }

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

  if(moreResultsError) return moreResultsError;
  if(nullresult) return [];

  return resultset;
}; // Database.querySync

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
    onBeginTransaction = function(err) 
    {
      if(err) 
      {
        deferred.reject(err);
      }
      else 
      {
        deferred.resolve(true);
      }
    };
  }

  self.conn.beginTransaction(deferred ? onBeginTransaction : cb);
  self.conn.inTransaction = true;

  return deferred ? deferred.promise : self;
};

Database.prototype.endTransaction = function (rollback, cb)
{
  var self = this;

  self.conn.endTransaction(rollback, cb);
  self.conn.inTransaction = false;

  return self;
};

Database.prototype.commitTransaction = function (cb)
{
  var self = this, deferred = null, onEndTransaction;
  if(!cb) 
  {
    deferred = Q.defer();
    onEndTransaction = function(err) 
    {
      if(err) 
      {
        deferred.reject(err);
      }
      else 
      {
        deferred.resolve(true);
      }
    };
  }

  //don't rollback
  self.conn.endTransaction(false, deferred ? onEndTransaction : cb); 
  self.conn.inTransaction = false;

  return deferred ? deferred.promise : self;
};

Database.prototype.rollbackTransaction = function (cb)
{
  var self = this, deferred = null, onEndTransaction;
  if(!cb) {
    deferred = Q.defer();
    onEndTransaction = function(err) {
      if(err) {
        deferred.reject(err);
      }
      else {
        deferred.resolve(true);
      }
    };
  }

  self.conn.endTransaction(true, deferred ? onEndTransaction : cb); //rollback
  self.conn.inTransaction = false;

  return deferred ? deferred.promise : self;
};

Database.prototype.beginTransactionSync = function ()
{
  var self = this;

  self.conn.beginTransactionSync();
  self.conn.inTransaction = true;

  return self;
};

Database.prototype.endTransactionSync = function (rollback)
{
  var self = this;

  self.conn.endTransactionSync(rollback);
  self.conn.inTransaction = false;

  return self;
};

Database.prototype.commitTransactionSync = function ()
{
  var self = this;

  self.conn.endTransactionSync(false); //don't rollback
  self.conn.inTransaction = false;

  return self;
};

Database.prototype.rollbackTransactionSync = function ()
{
  var self = this;

  self.conn.endTransactionSync(true); //rollback
  self.conn.inTransaction = false;

  return self;
};

Database.prototype.columns = function(catalog, schema, table, column, callback) 
{
  var self = this;
  if (!self.queue) self.queue = [];

  callback = callback || arguments[arguments.length - 1];

  self.queue.push(function (next) 
  {
    self.conn.columns(catalog, schema, table, column, function (err, result) 
    {
      if (err) return callback(err, [], false);

      result.fetchAll(function (err, data) 
      {
        result.closeSync();
        callback && callback(err, data);
        return next();
      });
    });
  });
};

Database.prototype.tables = function(catalog, schema, table, type, callback)
{
  var self = this;
  if (!self.queue) self.queue = [];

  callback = callback || arguments[arguments.length - 1];

  self.queue.push(function (next) 
  {
    self.conn.tables(catalog, schema, table, type, function (err, result) 
    {
      if (err) return callback(err, [], false);

      result.fetchAll(function (err, data) 
      {
        result.closeSync();
        callback && callback(err, data);
        return next();
      });
    });
  });
};

Database.prototype.describe = function(obj, callback)
{
  var self = this;

  if (typeof(callback) !== "function")
  {
    throw({
      error : "[node-odbc] Missing Arguments",
      message : "You must specify a callback function in order " +
                "for the describe method to work."
    });

    return false;
  }

  if (typeof(obj) !== "object")
  {
    callback({
      error : "[node-odbc] Missing Arguments",
      message : "You must pass an object as argument 0 if you want " +
                "anything productive to happen in the describe method."
    }, []);

    return false;
  }

  if (!obj.database)
  {
    callback({
      error : "[node-odbc] Missing Arguments",
      message : "The object you passed did not contain a database " +
                "property. This is required for the describe method to work."
    }, []);

    return false;
  }

  //set some defaults if they weren't passed
  obj.schema = obj.schema || "%";
  obj.type = obj.type || "TABLE";

  if (obj.table && obj.column)
  {
    //get the column details
    self.columns(obj.database, obj.schema, obj.table, obj.column, callback);
  }
  else if (obj.table)
  {
    //get the columns in the table
    self.columns(obj.database, obj.schema, obj.table, "%", callback);
  }
  else
  {
    //get the tables in the database
    self.tables(obj.database, obj.schema, null, obj.type, callback);
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
      if(cb)
      {
        return cb(err);
      } else
      {
        deferred.reject(err);
      }
    }

    stmt.queue = new SimpleQueue();

    stmt.prepare(sql, function (err) 
    {
      if (err)
      {
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

  var stmt = self.conn.createStatementSync();

  stmt.queue = new SimpleQueue();

  stmt.prepareSync(sql);

  return stmt;
};

Database.prototype.setIsolationLevel = function(isolationLevel) 
{
  var self = this;
  return (self.conn.setIsolationLevel(isolationLevel));
};

//Proxy all of the ODBCStatement functions so that they are queued
odbc.ODBCStatement.prototype._execute = odbc.ODBCStatement.prototype.execute;
odbc.ODBCStatement.prototype._executeSync = odbc.ODBCStatement.prototype.executeSync;
odbc.ODBCStatement.prototype._executeDirect = odbc.ODBCStatement.prototype.executeDirect;
odbc.ODBCStatement.prototype._executeDirectSync = odbc.ODBCStatement.prototype.executeDirectSync;
odbc.ODBCStatement.prototype._executeNonQuery = odbc.ODBCStatement.prototype.executeNonQuery;
odbc.ODBCStatement.prototype._executeNonQuerySync = odbc.ODBCStatement.prototype.executeNonQuerySync;
odbc.ODBCStatement.prototype._prepare = odbc.ODBCStatement.prototype.prepare;
odbc.ODBCStatement.prototype._bind = odbc.ODBCStatement.prototype.bind;
odbc.ODBCStatement.prototype._bindSync = odbc.ODBCStatement.prototype.bindSync;

odbc.ODBCStatement.prototype.execute = function (params, cb)
{
  var self = this, deferred;
  // promises logic
  if (!cb && typeof params !== 'function')
  {
    deferred = Q.defer();
  }

  self.queue = self.queue || new SimpleQueue();

  if (!cb && typeof params === 'function')
  {
    cb = params;
    params = null;
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
          if(!deferred)
          {
            cb(err);
          } else
          {
            deferred.reject(err);
          }
        }
      }
      self._bind(params, function (err) {
        if (err) {
          if(!deferred)
          {
            cb(err);
          } else
          {
            deferred.reject(err);
          }
          return next();
        }

        self._execute(function (err, result, outparams) {
          if(!deferred)
          {
            cb(err, result, outparams);
          } else
          {
            if(err)
            {
              deferred.reject(err);
            } else
            {
              deferred.resolve(result, outparams);
            }
          }

          return next();
        });
      });
    }
    //Otherwise execute and pop the next bind call
    else
    {
      self._execute(function (err, result, outparams) {
        if(!deferred)
        {
          cb(err, result, outparams);
        } else
        {
          if(err)
          {
            deferred.reject(err);
          } else
          {
            deferred.resolve(result, outparams);
          }
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
                else if(prm.CType == "INTEGER")
                    ctype = 4;
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
  var self = this;

  self.queue = self.queue || new SimpleQueue();

  self.queue.push(function (next) {
    self._executeDirect(sql, function (err, result) {
      cb && cb(err, result);

      return next();
    });
  });
};

odbc.ODBCStatement.prototype.executeNonQuery = function (params, cb) 
{
  var self = this, deferred;

  if (!cb && typeof params !== 'function')
  {
      deferred = Q.defer();
  }
  self.queue = self.queue || new SimpleQueue();

  if (!cb) 
  {
    cb = params;
    params = null;
  }

  self.queue.push(function (next) {
    //If params were passed to this function, then bind them and
    //then executeNonQuery.
    if (params) 
    {
      if(Array.isArray(params))
      {
        var err = parseParams(params);
        if(err) 
        {
          if(!deferred)
          {
            cb && cb(err);
          }
          else
          {
            deferred.reject(err);
          }
        }
      }
      self._bind(params, function (err) {
        if (err) {
          if(!deferred) 
          {
            cb && cb(err)
          } 
          else
          {
            deferred.reject(err);
          }
          return next();
        }

        self._executeNonQuery(function (err, result) {
          if(!deferred) 
          {
            cb &&  cb(err, result);
          } 
          else
          {
            if(err)
            {
              deferred.reject(err);
            } 
            else
            {
              deferred.resolve(result);
            }
          }
          return next();
        });
      });
    }
    //Otherwise executeNonQuery and pop the next bind call
    else {
      self._executeNonQuery(function (err, result) {
        if(!deferred) 
        {
          cb && cb(err, result);
        } 
        else
        {
          if(err)
          {
            deferred.reject(err);
          } 
          else
          {
            deferred.resolve(result);
          }
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

odbc.ODBCStatement.prototype.prepare = function (sql, cb) {
  var self = this;

  self.queue = self.queue || new SimpleQueue();

  self.queue.push(function (next) {
    self._prepare(sql, function (err) {
      cb && cb(err);

      return next();
    });
  });
};

// Function to bind parameters before execute or executeSync
odbc.ODBCStatement.prototype.bind = function (ary, cb) {
  var self = this;

  self.bindQueue = self.bindQueue || new SimpleQueue();

  self.bindQueue.push(function () {
    if(Array.isArray(ary))
    {
      var err = parseParams(ary);
      if(err && cb) cb(err);
    }
    self._bind(ary, function (err) {
      cb && cb(err);

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
    return self._bindSync(ary); 
};


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

Pool.prototype.open = function (connStr, callback)
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
        delete db;
        db = null;
      }
      else { break; }
    } while (!db);
  }
  if (db) {
    db.lastUsed=null;
    self.usedPool[connStr] = self.usedPool[connStr] || [];
    self.usedPool[connStr].push(db);
    callback && callback(null, db);
  }
  else if((self.maxPoolSize > 0) && (self.poolSize >= self.maxPoolSize))
  {
    // Move it to queue untill an existing connection is not closed.
    // Call it from db.close() version of pool.open and pool.init.
    self.queue.push(function (next) {
      if (self.availablePool[connStr] && self.availablePool[connStr].length)
      {
        db = self.availablePool[connStr].shift();
        db.lastUsed=null;
        self.usedPool[connStr] = self.usedPool[connStr] || [];
        self.usedPool[connStr].push(db);
        callback && callback(null, db);
      }
    });
  }
  else
  {
    if(typeof(self.options.odbc) === undefined)
        self.options.odbc = self.odbc;
    db = new Database(self.options);
    self.poolSize++;

    db.realClose = db.close;
    db.close = function (cb)
    {
      var db = this;
      self.availablePool[connStr] = self.availablePool[connStr] || [];
      // If conn.close is called more than once for this connection, 
      // nothing to do. Just call the callback function and return.
      if( self.availablePool[connStr] && 
          self.availablePool[connStr].indexOf(db) >= 0 )
      { 
          cb && cb(null);
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
      self.usedPool[connStr].splice(self.usedPool[connStr].indexOf(db), 1);
      self.poolSize--;

      //move this connection back to the connection pool at the end.
      if(db.conn)
      {
        self.availablePool[connStr].push(db);
        self.poolSize++;
      }
      cb && cb(null);

      if(db.conn && self.options.autoCleanIdle) self.cleanUp(connStr);
      if(exports.debug) { 
        process.stdout.write(getElapsedTime());
        console.log(util.inspect(self, {depth: null}));
      }
      return self.queue.next();
    };  // db.close function

    db.open(connStr, function (error) {
      exports.debug && console.log("%s odbc.js : pool[%s] : pool.db.open new connection.", getElapsedTime(), self.index);
      if(error)
      {
        self.poolSize--;
      }
      else
      {
        self.usedPool[connStr] = self.usedPool[connStr] || [];
        db.created = Date.now();
        self.usedPool[connStr].push(db);
      }
      db.availablePool = self.availablePool[connStr];
      callback && callback(error, db);
    }); //db.open
  }
};

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
  else
  {
    if((self.maxPoolSize > 0) && (count > self.maxPoolSize))
    {
        console.log(getElapsedTime(), " ** Can not open connection more than max pool size.\n");
        count = self.maxPoolSize;
    }

    if(typeof(self.options.odbc) === undefined)
        self.options.odbc = self.odbc;
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

        db.realClose = db.close;
        db.close = function (cb)
        {
          var db = this;
          self.availablePool[connStr] = self.availablePool[connStr] || [];
          // If conn.close is called more than once for this connection, 
          // nothing to do. Just call the callback function and return.
          if( self.availablePool[connStr] && 
              self.availablePool[connStr].indexOf(db) >= 0 )
          { 
              cb && cb(null);
              return;
          }
          db.lastUsed = Date.now();
          if(db.conn && db.conn.inTransaction)
          {
              db.rollbackTransaction(function(err){});
          }
          self.usedPool[connStr].splice(self.usedPool[connStr].indexOf(db), 1);
          self.poolSize--;
          if(db.conn)
          {
            self.availablePool[connStr].push(db);
            self.poolSize++;
          }
          if(cb) cb(null);
          if(db.conn && self.options.autoCleanIdle) self.cleanUp(connStr);
          return self.queue.next();
        };  // db.close function
        self.availablePool[connStr].push(db);
        db.availablePool = self.availablePool[connStr];
    }
    self.usedPool[connStr] = self.usedPool[connStr] || [];
    exports.debug && console.log(getElapsedTime(), "Max pool size = " + self.maxPoolSize);
    exports.debug && console.log(getElapsedTime(), "Pool = ", self);
    return ret;
  }
};// Pool.init()

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
    ;

  exports.debug && console.log("%s odbc.js : pool[%s] : pool.close()", getElapsedTime(), self.index);
  //we set a timeout because a previous db.close() may
  //have caused the a behind the scenes db.open() to prepare
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
      else return null;
    }

    for (key in pools)
    {
      connections = pools[key];
      required += connections.length;

      if(exports.debug) { 
        console.log("%s odbc.js : pool[%s] : pool.close() - processing pools %s - connections: %s", 
                    getElapsedTime(), self.index, key, connections.length);
      }

      for (x = 0 ; x < connections.length; x ++)
      {
        (function (x) {
          //call the realClose method to avoid
          //automatically re-opening the connection
          if(exports.debug) { 
            console.log("%s odbc.js : pool[%s] : pool.close() - calling realClose() for connection #%s", 
                        getElapsedTime(), self.index, x);
          }

          if(connections[x].realClose) {
           connections[x].realClose(function () {
            if(exports.debug) { 
              console.log("%s odbc.js : pool[%s] : pool.close() - realClose() callback for connection #%s", 
                          getElapsedTime(), self.index, x);
            }
            module.exports.close(connections[x]);
            received += 1;
            if(self.poolSize) self.poolSize--;

            if (received === required)
            {
              if(callback) callback();

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
              exports.debug && console.log("%s realClose is not a member of connection %s", getElapsedTime(), x);
          }
        })(x);
      } //for loop.
    } //for (key in pools)
  }, 2000);  //setTimeout
};  //Pool.close()

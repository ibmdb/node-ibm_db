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
                       path.resolve(__dirname, '../installer/clidriver/bin');
}

var odbc = require("bindings")("odbc_bindings")
  , SimpleQueue = require("./simple-queue")
  , util = require("util")
  , Q = require('q');


// Call of odbc.ODBC() loads odbc library and allocate environment handle.
// All calls of new Database() should use this same odbc unless passed as
// options.odbc. ENV will keep value of this odbc after first call of Database.
var ENV;

module.exports = function (options)
{
  return new Database(options);
}

module.exports.Database = Database;
module.exports.ODBC = odbc.ODBC;
module.exports.ODBCConnection = odbc.ODBCConnection;
module.exports.ODBCStatement = odbc.ODBCStatement;
module.exports.ODBCResult = odbc.ODBCResult;
module.exports.loadODBCLibrary = odbc.loadODBCLibrary;

exports.debug = false;
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
  var db;

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
    cb(err, db);
  });
}  // ibmdb.open

module.exports.openSync = function (connStr, options)
{
  var db = new Database(options);
  db.openSync(connStr);
  return db;
} // ibmdb.openSync

module.exports.close = function(db)
{
  if(db && typeof(db) == "object")
  {
    for(key in db)
    {
      delete db[key];
    }
    delete db;
    db = undefined;
  }
} // ibmdb.close

function Database(options)
{
  var self = this;

  options = options || {};

  if (odbc.loadODBCLibrary)
  {
    if (!options.library && !module.exports.library)
    {
      throw new Error("You must specify a library when complied with dynodbc, "
        + "otherwise this jams will segfault.");
    }

    if (!odbc.loadODBCLibrary(options.library || module.exports.library))
    {
      throw new Error("Could not load library. You may need to specify full "
        + "path.");
    }
  }

  self.odbc = (options.odbc) ? options.odbc : ((ENV) ? ENV : new odbc.ODBC());
  if(!ENV) ENV = self.odbc;
  self.queue = new SimpleQueue();
  self.fetchMode = options.fetchMode || null;
  self.connected = false;
  self.connectTimeout = options.connectTimeout || null;
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

  if (typeof(connStr) == "object")
  {
    var obj = connStr;
    connStr = "";

    Object.keys(obj).forEach(function (key) {
      connStr += key + "=" + obj[key] + ";";
    });
  }

  if (!cb)
  {
    deferred = Q.defer()
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

    self.conn.open(connStr, function (err, result)
    {
      if(cb)
      {
        if (err) return cb(err);

        self.connected = true;

        return cb(err, result);
      } else
      {
        if(err) deferred.reject(err);

        self.connected = true
        deferred.resolve(result)
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

  if (typeof(connStr) == "object")
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
} // Database.openSync

Database.prototype.close = function (cb)
{
  var self = this, deferred;
  if(!cb) {
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

  var resutl;
  if(self.conn) result = self.conn.closeSync();

  self.connected = false;
  delete self.conn;

  return result
} // closeSync

Database.prototype.query = function (sql, params, cb)
{
  var self = this, deferred;


  //support for promises
  if (!cb && typeof params !== 'function')
  {
    deferred = Q.defer();
    !params ? params = null : '';
  }


  if (typeof(params) == 'function')
  {
    cb = params;
    params = null;
  }

  if (!self.connected)
  {
    deferred ? deferred.reject({ message : "Connection not open."}) : cb({ message : "Connection not open."}, [], false);
    return deferred ?  deferred.promise : false;
  }

  self.queue.push(function (next) {
    function cbQuery (initialErr, result)
    {
      if (typeof(result) === 'object') {
        fetchMore();
      } else {
        cb(initialErr, [], false);
        return next();
      }

      function fetchMore()
      {
        if (self.fetchMode)
        {
          result.fetchMode = self.fetchMode;
        }

        result.fetchAll(function (err, data) {
          var moreResults, moreResultsError = null;

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
                self.closeSync();
              }
            }
            deferred ? deferred.reject(initialErr || err) : cb(initialErr || err, data, moreResults);
            result.closeSync();
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
          }

          //close the result before calling back
          //if there are not more result sets
          if (!moreResults)
          {
            //deferred ? deferred.reject(moreResultsError) : "";
            result.closeSync();
          }

          // send exception error and/or data to callback function.
          !cb ? deferred.resolve(data) : cb(moreResultsError, data, moreResults);
          moreResultsError = null;

          if (moreResults)
          {
            return fetchMore();
          }
          else
          {
            return next();
          }
        });
      }
    } //function cbQuery

    exports.debug && console.log("Executing %s", sql);
    if (params)
    {
      if(Array.isArray(params))
      {
        var err = parseParams(params);
        if(err) deferred ? deferred.reject(err) : cb(err);
      }
      if(sql.search(/^call /i))
        self.conn.query(sql, params, cbQuery);
      else // Its a CALL statement.
      {
        self.conn.query({"sql":sql, "params":params, "noResults":true}, cb);
      }
    }
    else
    {
      self.conn.query(sql, cbQuery);
    }
  }); //self.queue.push
  return deferred ? deferred.promise : false;
}; // Database.query

Database.prototype.queryResult = function (sql, params, cb)
{
  var self = this;

  if (typeof(params) == 'function')
  {
    cb = params;
    params = null;
  }

  if (!self.connected)
  {
    return cb({ message : "Connection not open."}, null);
  }

  self.queue.push(function (next) {
    //ODBCConnection.query() is the fastest-path querying mechanism.
    if (params)
    {
      if(Array.isArray(params))
      {
        var err = parseParams(params);
        if(err) cb(err);
      }
      self.conn.query(sql, params, cbQuery);
    }
    else
    {
      self.conn.query(sql, cbQuery);
    }

    function cbQuery (err, result)
    {
      if (err)
      {
        cb(err, null);
        return next();
      }

      if (self.fetchMode)
      {
        result.fetchMode = self.fetchMode;
      }

      cb(err, result);

      return next();
    } // function cbQuery
  }); //self.queue.push
}; // Database.queryResult

Database.prototype.queryResultSync = function (sql, params)
{
  var self = this, result;

  if (!self.connected)
  {
    throw ({ message : "Connection not open."});
  }

  if (params)
  {
    if(Array.isArray(params))
    {
        var err = parseParams(params);
        if(err) cb(err);
    }
    if(sql.search(/^call /i))
      result = self.conn.querySync(sql, params);
    else // Its a CALL statement.
    {
      result = self.conn.querySync({"sql":sql, "params":params, "noResults":true});
      return result;
    }
  }
  else
  {
    result = self.conn.querySync(sql);
  }

  if (self.fetchMode)
  {
    result.fetchMode = self.fetchMode;
  }

  return result;
}; // Database.queryResultSync

Database.prototype.querySync = function (sql, params)
{
  var self = this, result;

  if (!self.connected)
  {
    throw ({ message : "Connection not open."});
  }

  if (params)
  {
    if(Array.isArray(params))
    {
        var err = parseParams(params);
        if(err) return err;
    }
    if(sql.search(/^call /i))
        result = self.conn.querySync(sql, params);
    else // Its a CALL statement.
    {
      result = self.conn.querySync({"sql":sql, "params":params, "noResults":true});
      return result;
    }
  }
  else
  {
    result = self.conn.querySync(sql);
  }

  if (self.fetchMode)
  {
    result.fetchMode = self.fetchMode;
  }

  var data = result.fetchAllSync();

  result.closeSync();

  return data;
}; // Database.querySync

Database.prototype.beginTransaction = function (cb)
{
  var self = this, deferred = null, onBeginTransaction;
  if(!cb) {
    deferred = Q.defer();
    onBeginTransaction = function(err) {
      if(err) {
        deferred.reject(err);
      }
      else {
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
  var self = this, deferred = null, onBeginTransaction;
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

  self.conn.endTransaction(false, deferred ? onEndTransaction : cb); //don't rollback
  self.conn.inTransaction = false;

  return deferred ? deferred.promise : self;
};

Database.prototype.rollbackTransaction = function (cb)
{
  var self = this, deferred = null, onBeginTransaction;
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

Database.prototype.columns = function(catalog, schema, table, column, callback) {
  var self = this;
  if (!self.queue) self.queue = [];

  callback = callback || arguments[arguments.length - 1];

  self.queue.push(function (next) {
    self.conn.columns(catalog, schema, table, column, function (err, result) {
      if (err) return callback(err, [], false);

      result.fetchAll(function (err, data) {
        result.closeSync();

        callback(err, data);

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

  self.queue.push(function (next) {
    self.conn.tables(catalog, schema, table, type, function (err, result) {
      if (err) return callback(err, [], false);

      result.fetchAll(function (err, data) {
        result.closeSync();

        callback(err, data);

        return next();
      });
    });
  });
};

Database.prototype.describe = function(obj, callback)
{
  var self = this;

  if (typeof(callback) != "function")
  {
    throw({
      error : "[node-odbc] Missing Arguments",
      message : "You must specify a callback function in order for the describe method to work."
    });

    return false;
  }

  if (typeof(obj) != "object")
  {
    callback({
      error : "[node-odbc] Missing Arguments",
      message : "You must pass an object as argument 0 if you want anything productive to happen in the describe method."
    }, []);

    return false;
  }

  if (!obj.database)
  {
    callback({
      error : "[node-odbc] Missing Arguments",
      message : "The object you passed did not contain a database property. This is required for the describe method to work."
    }, []);

    return false;
  }

  //set some defaults if they weren't passed
  obj.schema = obj.schema || "%";
  obj.type = obj.type || "table";

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
    self.tables(obj.database, obj.schema, null, obj.type || "table", callback);
  }
}; //Database.describe

Database.prototype.prepare = function (sql, cb)
{
  var self = this, deferred;

  if(!cb) {
    deferred = Q.defer();
  }

  self.conn.createStatement(function (err, stmt) {
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

    stmt.prepare(sql, function (err) {
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
}

Database.prototype.prepareSync = function (sql, cb)
{
  var self = this;

  var stmt = self.conn.createStatementSync();

  stmt.queue = new SimpleQueue();

  stmt.prepareSync(sql);

  return stmt;
}

Database.prototype.setIsolationLevel = function(isolationLevel) {
  var self = this;
  return (self.conn.setIsolationLevel(isolationLevel));
}

//Proxy all of the asynchronous functions so that they are queued
odbc.ODBCStatement.prototype._execute = odbc.ODBCStatement.prototype.execute;
odbc.ODBCStatement.prototype._executeDirect = odbc.ODBCStatement.prototype.executeDirect;
odbc.ODBCStatement.prototype._executeNonQuery = odbc.ODBCStatement.prototype.executeNonQuery;
odbc.ODBCStatement.prototype._prepare = odbc.ODBCStatement.prototype.prepare;
odbc.ODBCStatement.prototype._bind = odbc.ODBCStatement.prototype.bind;

odbc.ODBCStatement.prototype.execute = function (params, cb)
{
  var self = this, deferred;
  // promises logic
  if (!cb && typeof params !== 'function')
  {
    deferred = Q.defer();
    // if(!params)
    // {
    //   params = null;
    // }

  }

  self.queue = self.queue || new SimpleQueue();

  if (!cb)
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

        self._execute(function (err, result) {
          if(!deferred)
          {
            cb(err, result);
          } else
          {
            if(err)
            {
              deferred.reject(err);
            } else
            {
              deferred.resolve(result);
            }
          }

          return next();
        });
      });
    }
    //Otherwise execute and pop the next bind call
    else
    {
      self._execute(function (err, result) {
        if(!deferred)
        {
          cb(err, result);
        } else
        {
          if(err)
          {
            deferred.reject(err);
          } else
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

function parseParams(params)
{
    var err, prm, paramtype, ctype, sqltype, datatype, data, len;
    for (var i = 0; i < params.length; i++)
    {
        if(Object.prototype.toString.call(params[i]) == "[object Object]")
        {
            //{ParamType:"INPUT", DataType:"BLOB", Data:imgfile}
            //{"ParamType":"INPUT", CType:"BINARY", SQLType:"BLOB",Data:imgfile}
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

            if(Number.isInteger(prm.Data))
            {
                if(!ctype) ctype = -25; // SQL_C_SBIGINT
                if(!sqltype) sqltype = -5; // SQL_BIGINT

            }
            if(!ctype) ctype = 1;
            if(!sqltype) sqltype = 1;
            if(paramtype == 3) // For BindFileToParam()
            {
                ctype = 1;
                if(!((sqltype == -98) || (sqltype == -99) ||
                     (sqltype == -350) || (sqltype == -370)))
                {
                    err = "Data Type is missing form " + prm;
                    break;
                }
            }

            if(prm.Data || (paramtype == 4))
            {
                params[i] = [paramtype, ctype, sqltype, prm.Data, len];
            }
            else
            {
                err = "Data is missing from " + JSON.stringify(prm);
                break;
            }
        }
        //console.log(i + "th param = " + params[i]);
    }
    return err;
}

if(Number.isInteger == undefined)   // node.js < v0.12.0 do not support isInteger
{
    Number.isInteger = function(x)
    {
        if((typeof x === 'number') && (x % 1 === 0)) return true;
        else false;
    }
}

odbc.ODBCStatement.prototype.executeDirect = function (sql, cb) {
  var self = this;

  self.queue = self.queue || new SimpleQueue();

  self.queue.push(function (next) {
    self._executeDirect(sql, function (err, result) {
      cb(err, result);

      return next();
    });
  });
};

odbc.ODBCStatement.prototype.executeNonQuery = function (params, cb) {
  var self = this;

  self.queue = self.queue || new SimpleQueue();

  if (!cb) {
    cb = params;
    params = null;
  }

  self.queue.push(function (next) {
    //If params were passed to this function, then bind them and
    //then executeNonQuery.
    if (params) {
      if(Array.isArray(params))
      {
        var err = parseParams(params);
        if(err) cb(err);
      }
      self._bind(params, function (err) {
        if (err) {
          return cb(err);
        }

        self._executeNonQuery(function (err, result) {
          cb(err, result);

          return next();
        });
      });
    }
    //Otherwise executeNonQuery and pop the next bind call
    else {
      self._executeNonQuery(function (err, result) {
        cb(err, result);

        //NOTE: We only execute the next queued bind call after
        // we have called execute() or executeNonQuery(). This ensures
        // that we don't call a bind() a bunch of times without ever
        // actually executing that bind. Not
        self.bindQueue && self.bindQueue.next();

        return next();
      });
    }
  });
};

odbc.ODBCStatement.prototype.prepare = function (sql, cb) {
  var self = this;

  self.queue = self.queue || new SimpleQueue();

  self.queue.push(function (next) {
    self._prepare(sql, function (err) {
      cb(err);

      return next();
    });
  });
};

odbc.ODBCStatement.prototype.bind = function (ary, cb) {
  var self = this;

  self.bindQueue = self.bindQueue || new SimpleQueue();

  self.bindQueue.push(function () {
    if(Array.isArray(ary))
    {
      var err = parseParams(ary);
      if(err) cb(err);
    }
    self._bind(ary, function (err) {
      cb(err);

      //NOTE: we do not call next() here because
      //we want to pop the next bind call only
      //after the next execute call
    });
  });
};


module.exports.Pool = Pool;

Pool.count = 0;

function Pool (_options) {
  var self = this;
  self.options = {};
  self.maxPoolSize = 0;
  if(_options) {
    if(_options.idleTimeout && !isNaN(_options.idleTimeout))
      self.options.idleTimeout=_options.idleTimeout;
    if(_options.autoCleanIdle)
      self.options.autoCleanIdle=_options.autoCleanIdle;
    if(_options.maxPoolSize)
      self.maxPoolSize = _options.maxPoolSize;
    if(_options.connectTimeout)
      self.options.connectTimeout = _options.connectTimeout;
  }
  self.index = Pool.count++;
  self.availablePool = {};
  self.usedPool = {};
  self.poolSize = 0;
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
    db = self.availablePool[connStr].shift()
    db.lastUsed=null;
    self.usedPool[connStr].push(db)
    callback(null, db);
  }
  else if((self.maxPoolSize > 0) && (self.poolSize >= self.maxPoolSize))
  {
    var timeout = self.options.connectTimeout;
    var error = {"message":"Connection Timeout Occurred, Pool is full."};
    var interval =  setInterval(function () {
      if (self.availablePool[connStr] && self.availablePool[connStr].length)
      {
        db = self.availablePool[connStr].shift()
        db.lastUsed=null;
        self.usedPool[connStr].push(db)
        clearInterval(interval);
        callback(null, db);
      }
      if(timeout == 0) 
      {
        clearInterval(interval);
        callback(error, null);
      }
      else timeout--;
    }, 1000);  //setInterval
  }
  else
  {
    db = new Database({ odbc : self.odbc });
    self.poolSize++;
    db.open(connStr, function (error) {
      exports.debug && console.log(
          "odbc.js : pool[%s] : pool.db.open new connection.", self.index);
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
      callback(error, db);
    }); //db.open

    db.realClose = db.close;
    db.close = function (cb)
    {
      db.lastUsed = Date.now();
      //call back early, we can do the rest of this stuff after the client
      //thinks that the connection is closed.
      cb(null);

      // If this connection has some active transaction, rollback the
      // transaction to free up the held resorces before moving back to
      // the pool. So that, next request can get afresh connection from pool.
      if(db.conn && db.conn.inTransaction)
      {
          db.rollbackTransaction(function(err){});
      }

      //remove this db from the usedPool
      self.usedPool[connStr].splice(self.usedPool[connStr].indexOf(db), 1);

      //move this connection back to the connection pool
      if(db.conn)
      {
        self.availablePool[connStr] = self.availablePool[connStr] || [];
        self.availablePool[connStr].unshift(db);

        //start cleanUp if enabled
        if(self.options.autoCleanIdle)self.cleanUp(connStr);
      }
      exports.debug && console.dir(self);
    };  // db.close function

  }
};

Pool.prototype.init = function(count, connStr)
{
  var self = this;
  var db;
  var ret = false;

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
        exports.debug && console.log("Can not open connection more than max pool size.\n");
        count = self.maxPoolSize;
    }

    for(var i = 0; i < count; i++)
    {
        db = new Database({ odbc : self.odbc });
        self.poolSize++;
        try{
            ret = db.openSync(connStr);
        } catch (ret) {
            self.poolSize--;
            exports.debug && console.log("odbc.js: %d connection(s) initialized.\n",
                                         self.poolSize);
            return ret;
        }
        exports.debug && console.log(
            "odbc.js : pool[%s] : pool.init %d", self.index, i);

        self.availablePool[connStr] = self.availablePool[connStr] || [];
        db.created = Date.now();
        self.availablePool[connStr].push(db);

        db.realClose = db.close;
        db.close = function (cb)
        {
          db.lastUsed = Date.now();
          cb(null);
          if(db.conn && db.conn.inTransaction)
          {
              db.rollbackTransaction(function(err){});
          }
          self.usedPool[connStr].splice(self.usedPool[connStr].indexOf(db), 1);
          if(db.conn)
          {
            self.availablePool[connStr] = self.availablePool[connStr] || [];
            self.availablePool[connStr].unshift(db);
            if(self.options.autoCleanIdle)self.cleanUp(connStr);
          }
          exports.debug && console.dir(self);
        };  // db.close function
    }
    self.usedPool[connStr] = self.usedPool[connStr] || [];
    exports.debug && console.log("Max pool size = " + self.maxPoolSize);
    return ret;
  }
}// Pool.init()

Pool.prototype.setMaxPoolSize = function(size)
{
    var self = this;
    self.maxPoolSize = size;
    return true;
}

Pool.prototype.setConnectTimeout = function(timeout)
{
    var self = this;
    self.options.connectTimeout = timeout;
    return true;
}

// Close idle connections
Pool.prototype.cleanUp = function(connStr,callback) {
  var self = this;
  if(self.availablePool[connStr].length==1) return;
  self.availablePool[connStr]=self.availablePool[connStr].filter(function(conn) 
  {
    if(conn.lastUsed && (Date.now()-conn.lastUsed > (self.options.idleTimeout || 1800 * 1000)) && conn.realClose ) {
       conn.realClose(function() {
         if(self.poolSize) self.poolSize--;
         exports.debug && console.log("odbc.js : pool[%s] : Pool.cleanUp() : pool.realClose() : Connection duration : %s", self.index, (Date.now() - conn.created)/1000);
       })
       return false;
    }
    else return true;
  })
} //Pool.cleanUp()

Pool.prototype.close = function (callback)
{
  var self = this
    , required = 0
    , received = 0
    , connections
    , key
    , x
    ;

  exports.debug && console.log("odbc.js : pool[%s] : pool.close()", self.index);
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

    exports.debug && console.log(
      "odbc.js : pool[%s] : pool.close() - setTimeout() callback", self.index);
    //exports.debug && console.dir(pools);

    if (Object.keys(pools).length == 0)
    {
      return callback();
    }

    for (key in pools)
    {
      connections = pools[key];
      required += connections.length;

      exports.debug && console.log("odbc.js : pool[%s] : pool.close() - processing pools %s - connections: %s", self.index, key, connections.length);

      for (x = 0 ; x < connections.length; x ++)
      {
        (function (x) {
          //call the realClose method to avoid
          //automatically re-opening the connection
          exports.debug && console.log("odbc.js : pool[%s] : pool.close() - calling realClose() for connection #%s", self.index, x);

          if(connections[x].realClose) {
           connections[x].realClose(function () {
            exports.debug && console.log("odbc.js : pool[%s] : pool.close() - realClose() callback for connection #%s", self.index, x);
            module.exports.close(connections[x]);
            received += 1;
            if(self.poolSize) self.poolSize--;

            if (received === required)
            {
              callback();

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
              exports.debug && console.log("realClose is not a member of connection %s", x);
          }
        })(x);
      } //for loop.
    } //for (key in pools)
  }, 2000);  //setTimeout
};  //Pool.close()

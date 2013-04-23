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

var odbc = require("bindings")("odbc_bindings")
  , SimpleQueue = require("./simple-queue")
  , util = require("util")
  ;

module.exports = function (options) {
  return new Database(options);
}

module.exports.debug = false;

module.exports.Database = Database;
module.exports.ODBC = odbc.ODBC;
module.exports.ODBCConnection = odbc.ODBCConnection;
module.exports.ODBCStatement = odbc.ODBCStatement;
module.exports.ODBCResult = odbc.ODBCResult;

function Database(options) {
  var self = this;
  
  options = options || {};
  
  self.odbc = (options.odbc) ? options.odbc : new odbc.ODBC();
  self.queue = new SimpleQueue();
  self.fetchMode = options.fetchMode || null;
  self.connected = false;
}

//Expose constants
Object.keys(odbc.ODBC).forEach(function (key) {
  if (typeof odbc.ODBC[key] !== "function") {
    //On the database prototype
    Database.prototype[key] = odbc.ODBC[key];
    
    //On the exports
    module.exports[key] = odbc.ODBC[key];
  }
});

Database.prototype.open = function (connectionString, cb) {
  var self = this;
  
  self.odbc.createConnection(function (err, conn) {
    if (err) return cb(err);
    
    self.conn = conn;
    
    self.conn.open(connectionString, function (err, result) {
      if (err) return cb(err);
                   
      self.connected = true;
      
      return cb(err, result);
    });
  });
};

Database.prototype.openSync = function (connectionString) {
  var self =  this;
  
  self.conn = self.odbc.createConnectionSync();
  
  return self.conn.openSync(connectionString);
}

Database.prototype.close = function (cb) {
  var self = this;
  
  self.queue.push(function (next) {
    self.conn.close(function (err) {
      self.connected = false;
      
      cb(err);
      return next();
    });
  });
};

Database.prototype.query = function (sql, params, cb) {
  var self = this;
  
  if (typeof(params) == 'function') {
    cb = params;
    params = null;
  }
  
   if (!self.connected) {
    return cb({ message : "Connection not open."}, [], false);
  }
  
  self.queue.push(function (next) {
    //ODBCConnection.query() is the fastest-path querying mechanism.
    var cbActual = function (err, result) {
      if (err) {
        cb(err, [], false);
        
        return next();
      }
      
      fetchMore();
      
      function fetchMore() {
        //TODO: pass fetchMode if it's not null
        result.fetchAll(function (err, data) {
          var moreResults = result.moreResultsSync();
          
          cb(err, data, moreResults);
          
          if (moreResults) {
            return fetchMore();
          }
          else {
            result.closeSync();
            
            return next();
          }
        });
      }
    }
    
    if (params) {
      self.conn.query(sql, params, cbActual);
    }
    else {
      self.conn.query(sql, cbActual);
    }
  });
};

Database.prototype.queryResult = function (sql, params, cb) {
  var self = this;
  
  if (typeof(params) == 'function') {
    cb = params;
    params = [];
  }
  
  if (!self.connected) {
    return cb({ message : "Connection not open."}, [], false);
  }
  
  self.queue.push(function (next) {
    //ODBCConnection.query() is the fastest-path querying mechanism.
    self.conn.query(sql, params, function (err, result) {
      if (err) {
        cb(err, null);
        
        return next();
      }
      
      cb(err, result);
      
      return next();
    });
  });
};

Database.prototype.querySync = function (sql, params) {
  var self = this, result;
  
  if (!self.connected) {
    throw ({ message : "Connection not open."});
  }
  
  if (params) {
    result = self.conn.querySync(sql, params);
  }
  else {
    result = self.conn.querySync(sql);
  }
  
  var data = result.fetchAllSync();
  
  result.closeSync();
  
  return data;
};

Database.prototype.beginTransactionSync = function () {
  var self = this;
  
  self.conn.beginTransactionSync();
  
  return self;
};

Database.prototype.endTransactionSync = function (rollback) {
  var self = this;
  
  self.conn.endTransactionSync(rollback);
  
  return self;
};

Database.prototype.commitTransactionSync = function () {
  var self = this;
  
  self.conn.endTransactionSync(false); //don't rollback
  
  return self;
}

Database.prototype.rollbackTransactionSync = function () {
  var self = this;
  
  self.conn.endTransactionSync(true); //rollback
  
  return self;  
}

Database.prototype.columns = function(catalog, schema, table, column, callback) {
  var self = this;
  if (!self.queue) self.queue = [];
  
  callback = callback || arguments[arguments.length - 1];
  
  self.queue.push(function (next) {
    self.conn.columns(catalog, schema, table, column, function (err, result) {
      if (err) return callback(err, [], false);

      result.fetchAll(function (err, data) {
        callback(err, data);
        
        return next();
      });
    });
  });
};

Database.prototype.tables = function(catalog, schema, table, type, callback) {
  var self = this;
  if (!self.queue) self.queue = [];
  
  callback = callback || arguments[arguments.length - 1];
  
  self.queue.push(function (next) {
    self.conn.tables(catalog, schema, table, type, function (err, result) {
      if (err) return callback(err, [], false);

      result.fetchAll(function (err, data) {
        callback(err, data);
        
        return next();
      });
    });
  });
};

Database.prototype.describe = function(obj, callback) {
  var self = this;
  
  if (typeof(callback) != "function") {
    throw({
      error : "[node-odbc] Missing Arguments",
      message : "You must specify a callback function in order for the describe method to work."
    });
    
    return false;
  }
  
  if (typeof(obj) != "object") {
    callback({
      error : "[node-odbc] Missing Arguments",
      message : "You must pass an object as argument 0 if you want anything productive to happen in the describe method."
    }, []);
    
    return false;
  }
  
  if (!obj.database) {
    callback({
      error : "[node-odbc] Missing Arguments",
      message : "The object you passed did not contain a database property. This is required for the describe method to work."
    }, []);
    
    return false;
  }
  
  //set some defaults if they weren't passed
  obj.schema = obj.schema || "%";
  obj.type = obj.type || "table";
  
  if (obj.table && obj.column) {
    //get the column details
    self.columns(obj.database, obj.schema, obj.table, obj.column, callback);
  }
  else if (obj.table) {
    //get the columns in the table
    self.columns(obj.database, obj.schema, obj.table, "%", callback);
  }
  else {
    //get the tables in the database
    self.tables(obj.database, obj.schema, null, obj.type || "table", callback);
  }
};

Database.prototype.prepare = function (sql, cb) {
  var self = this;
  
  self.conn.createStatement(function (err, stmt) {
    if (err) return cb(err);
    
    stmt.queue = new SimpleQueue();
    
    stmt.prepare(sql, function (err) {
      if (err) return cb(err);
      
      return cb(null, stmt);
    });
  });
}

Database.prototype.prepareSync = function (sql, cb) {
  var self = this;
  
  var stmt = self.conn.createStatementSync();
  
  stmt.queue = new SimpleQueue();
    
  stmt.prepareSync(sql);
    
  return stmt;
}

//Proxy all of the asynchronous functions so that they are queued
odbc.ODBCStatement.prototype._execute = odbc.ODBCStatement.prototype.execute;
odbc.ODBCStatement.prototype._executeDirect = odbc.ODBCStatement.prototype.executeDirect;
odbc.ODBCStatement.prototype._prepare = odbc.ODBCStatement.prototype.prepare;
odbc.ODBCStatement.prototype._bind = odbc.ODBCStatement.prototype.bind;

odbc.ODBCStatement.prototype.execute = function (cb) {
  var self = this;
  
  self.queue.push(function (next) {
    self._execute(function (err, result) {
      cb(err, result);
      
      return next();
    });
  });
};

odbc.ODBCStatement.prototype.executeDirect = function (sql, cb) {
  var self = this;
  
  self.queue.push(function (next) {
    self._executeDirect(sql, function (err, result) {
      cb(err, result);
      
      return next();
    });
  });
};

odbc.ODBCStatement.prototype.prepare = function (sql, cb) {
  var self = this;
  
  self.queue.push(function (next) {
    self._prepare(sql, function (err) {
      cb(err);
      
      return next();
    });
  });
};

odbc.ODBCStatement.prototype.bind = function (ary, cb) {
  var self = this;
  
  self.queue.push(function (next) {
    self._bind(ary, function (err) {
      cb(err);
      
      return next();
    });
  });
};



module.exports.Pool = Pool;

Pool.count = 0;

function Pool () {
  var self = this;
  self.index = Pool.count++;
  self.availablePool = {};
  self.usedPool = {};
  self.odbc = new odbc.ODBC();
}

Pool.prototype.open = function (connectionString, callback) {
  var self = this
    , db
    ;

  //check to see if we already have a connection for this connection string
  if (self.availablePool[connectionString] && self.availablePool[connectionString].length) {
    db = self.availablePool[connectionString].shift()
    self.usedPool[connectionString].push(db)

    callback(null, db);
  }
  else {
    db = new Database({ odbc : self.odbc });
    db.realClose = db.close;
    
    db.close = function (cb) {
      //call back early, we can do the rest of this stuff after the client thinks
      //that the connection is closed.
      cb(null);
      
      
      //close the connection for real
      //this will kill any temp tables or anything that might be a security issue.
      db.realClose(function () {
         //remove this db from the usedPool
         self.usedPool[connectionString].splice(self.usedPool[connectionString].indexOf(db), 1);

        //re-open the connection using the connection string
        db.open(connectionString, function (error) {
          if (error) {
            console.error(error);
            return;
          }
          
          //add this clean connection to the connection pool
          self.availablePool[connectionString] = self.availablePool[connectionString] || [];
          self.availablePool[connectionString].push(db);
          exports.debug && console.dir(self);
        });
      });
    };
    
    db.open(connectionString, function (error) {
      exports.debug && console.log("odbc.js : pool[%s] : pool.db.open callback()", self.index);

      self.usedPool[connectionString] = self.usedPool[connectionString] || [];
      self.usedPool[connectionString].push(db);

      callback(error, db);
    });
  }
};

Pool.prototype.close = function (callback) {
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

    for (key in self.availablePool) {
      pools[key] = (pools[key] || []).concat(self.availablePool[key]);
    }

    for (key in self.usedPool) {
      pools[key] = (pools[key] || []).concat(self.usedPool[key]);
    }

    exports.debug && console.log("odbc.js : pool[%s] : pool.close() - setTimeout() callback", self.index);
    exports.debug && console.dir(pools);

    if (Object.keys(pools).length == 0) {
      return callback();
    }

    for (key in pools) {
      connections = pools[key];
      required += connections.length;

      exports.debug && console.log("odbc.js : pool[%s] : pool.close() - processing pools %s - connections: %s", self.index, key, connections.length);

      for (x = 0 ; x < connections.length; x ++) {
        (function (x) {
          //call the realClose method to avoid
          //automatically re-opening the connection
          exports.debug && console.log("odbc.js : pool[%s] : pool.close() - calling realClose() for connection #%s", self.index, x);

          connections[x].realClose(function () {
            exports.debug && console.log("odbc.js : pool[%s] : pool.close() - realClose() callback for connection #%s", self.index, x);
            received += 1;

            if (received === required) {
              callback();

              //prevent mem leaks
              self = null;
              connections = null;
              required = null;
              received = null;
              key = null;

              return;
            }
          });
        })(x);
      }
    }
  }, 2000);
};

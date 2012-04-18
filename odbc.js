/*
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

var odbc;

try {
  odbc = require("./odbc_bindings");
} catch (e) {
  try {
    odbc = require("./build/default/odbc_bindings");
  } catch (e) {
    odbc = require("./build/Release/odbc_bindings");
  }
}

var Database = exports.Database = function () {
  var self = this;
  var db = new odbc.Database();
  db.executing = false;
  db.connected = false;
  db.queue = [];
  
  db.__proto__ = Database.prototype;
  
  return db;
};

Database.prototype = {
  __proto__: odbc.Database.prototype,
  constructor: Database,
};

Database.prototype.processQueue = function () {
  var self = this;
  
  if (!self.queue) self.queue = [];
  
  if (self.connected && !self.executing && self.queue.length) {
    var currentQuery = self.queue[0];
    self.executing = true;
    
    currentQuery.method.apply(currentQuery.context, currentQuery.args); //TODO: we need to make sure we aren't sending any extra arguments to the cpp method
  }
};

Database.prototype.query = function(sql, params, callback) {
  var self = this, args = [];

  if (callback == null) {
    callback = params; // no parameters supplied
    params = null;
  }
  
  if (!self.connected) {
    return callback( { message : "Connection not open." }, [], false );
  }
  
  if (!self.queue) self.queue = [];
  
  args.push(sql);

  if (params) {
    args.push(params);  
  }

  args.push(function (error, rows, morefollowing) {
    //check to see if this is the last result set returned
    if (!morefollowing) {
      self.queue.shift();
      self.executing = false;
    }

    if (callback) callback.apply(self, arguments);

    self.processQueue();
  });
 
  self.queue.push({
    context : self,
    method : self.dispatchQuery,
    args : args
  });
  
  self.processQueue();
};

Database.prototype.open = function(connectionString, callback) {
  var self = this;
  
  if (self.connected) {
    return callback( { message : "Connection already open." }, [], false);
  }
  
  self.dispatchOpen(connectionString, function (err) {
    self.connected = true;
    self.processQueue();
    
    return callback(err);
  });
};

/**
 * 
 * We must queue the close. If we don't then we may close during the middle of a query which 
 * could cause a segfault or other madness
 * 
 **/

Database.prototype.close = function(callback) {
  var self = this;
  
  if (!self.queue) self.queue = [];
  
  self.queue.push({
    context : self,
    method : self.dispatchClose,
    args : [function (err) {
      self.queue = [];
      self.connected = false;
      self.executing = false;

      if (err && !callback) throw err;
      else if (callback) callback(err)
    }]
  });
  
  self.processQueue();
};

Database.prototype.tables = function(catalog, schema, table, type, callback) {
  var self = this;
  if (!self.queue) self.queue = [];
  
  self.queue.push({
    context : self,
    method : self.dispatchTables,
    catalog : (arguments.length > 1) ? catalog : "",
    schema : (arguments.length > 2) ? schema : "",
    table : (arguments.length > 3) ? table : "",
    type : (arguments.length > 4) ? type : "",
    callback : (arguments.length == 5) ? callback : arguments[arguments.length - 1],
    args : arguments
  });
  
  self.processQueue();
};

Database.prototype.columns = function(catalog, schema, table, column, callback) {
  var self = this;
  if (!self.queue) self.queue = [];
  
  self.queue.push({
    context : self,
    method : self.dispatchColumns,
    catalog : (arguments.length > 1) ? catalog : "",
    schema : (arguments.length > 2) ? schema : "",
    table : (arguments.length > 3) ? table : "",
    column : (arguments.length > 4) ? column : "",
    callback : (arguments.length == 5) ? callback : arguments[arguments.length - 1],
    args : arguments
  });
  
  self.processQueue();
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


var Pool = exports.Pool = function () {
  var self = this;
  
  self.connectionPool = {};
}

Pool.prototype.open = function (connectionString, callback) {
  var self = this;
  
  //check to see if we already have a connection for this connection string
  if (self.connectionPool[connectionString] && self.connectionPool[connectionString].length) {
    callback(null, self.connectionPool[connectionString].shift());
  }
  else {
    var db = new Database();
    db.realClose = db.close;
    
    db.close = function (cb) {
      //call back early, we can do the rest of this stuff after the client thinks
      //that the connection is closed.
      cb(null);
      
      
      //close the connection for real
      //this will kill any temp tables or anything that might be a security issue.
      db.realClose( function () {
        
        //re-open the connection using the connection string
        db.open(connectionString, function (error) {
          
          //add this clean connection to the connection pool
          self.connectionPool[connectionString] = self.connectionPool[connectionString] || [];
          self.connectionPool[connectionString].push(db);
          
        });
      });
    };
    
    db.open(connectionString, function (error) {
      callback(error, db);
    });
  }
};

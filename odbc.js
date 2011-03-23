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

var sys = require("sys");
var odbc = require("./odbc_bindings");

var Database = exports.Database = function () {
    var self = this;
    var db = new odbc.Database();

    db.__proto__ = Database.prototype;

    db.addListener("ready", function () {
        db.maybeDispatchQuery();
    });

    db.addListener("result", function () {
        //process.assert(db.currentQuery).ok();
        require('assert').ok(db.currentQuery);
        
        var callback = db.currentQuery[1];
        var args = Array.prototype.slice.call(db.currentQuery[2]);
        args.shift();
        args.shift();

        if(arguments[0])
        {
            arguments[0].query = db.currentQuery[0];
        }
        
        //check to see if this is the last result set returned
        if (!arguments[2]) {
            db.currentQuery = null;
        }
        
        if (callback) {
            var newArgs = Array.prototype.slice.call(arguments);

            for(var i = 0; i < args.length; i++) {
                newArgs.push(args[i]);
            }

            callback.apply(db, newArgs);
        }
        db.maybeDispatchQuery();
    });

    return db;
};

Database.prototype = {
    __proto__: odbc.Database.prototype,
    constructor: Database,
};

Database.prototype.maybeDispatchQuery = function () {
    if (!this._queries) return;
    if (!this.currentQuery && this._queries.length > 0) {
        this.currentQuery = this._queries.shift();
        this.dispatchQuery(this.currentQuery[0]);
    }
};

Database.prototype.query = function(sql, callback) {
    this._queries = this._queries || [];
    this._queries.push([sql, callback, arguments]);
    this.maybeDispatchQuery();
};

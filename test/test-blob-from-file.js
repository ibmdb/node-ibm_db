// Test insertion of BLOB and CLOB data using FILE NAME.
// No need to read the file into memory buffer.
// Author: bimaljha@in.ibm.com

var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , fs = require('fs')
    , cn = common.connectionString;

ibmdb.open(cn, function (err,conn) {
  if (err) {
    console.log(err);
    process.exit(-1);
  }
  try {
      conn.querySync("drop table mytab");
  } catch (e) {};
  try {
    conn.querySync("create table mytab (empId int, photo BLOB(1M), trace CLOB(1M), document BLOB(1M))");
  } catch (e) {};
  
  var img1 = __dirname + '/data/phool.jpg'; //fs.readFileSync('phool.jpg','binary');
  var text = __dirname + '/data/trc.fmt'; //fs.readFileSync('trc.fmt','ascii');
  var doc  = __dirname + '/data/sampledoc.docx'; 

  var len1  = fs.statSync(img1)["size"];
  var len2  = fs.statSync(text)["size"];
  var len3  = fs.statSync(doc)["size"];

  console.log( "img1.length = " + len1);
  console.log( "text.length = " + len2);
  console.log( "doc.length = " +  len3);

  conn.prepare("insert into mytab(empId, photo, trace, document) VALUES (?, ?, ?, ?)",
  function (err, stmt) {
    if (err) {
      console.log(err);
      return conn.closeSync();
    }
    //var photo = [3, -2, -98, img1];  // We can use such array too.
    //var photo = {ParamType:"FILE", CType:"BINARY", "SQLType:"BLOB", Data:img1};
    // Except, numbers and string; all other datatypes like LOBS, GRAPHIC, File, etc
    // must be passed as JSON Object or Array.
    var photo = {ParamType:"FILE", DataType: "BLOB", "Data":img1};
    var tracefile = {ParamType: 3, "DataType": "CLOB", Data: text};
    var document = {ParamType:"FILE", DataType: "BLOB", "Data":doc};

    stmt.execute([18, photo, tracefile, document],
    function (err, result) {
      if( err ) console.log(err);
      else result.closeSync();

      conn.prepare("select * from mytab", function (err, stmt) {
        if(err) {
          console.log(err);
          return conn.closeSync();
        }

        stmt.execute([], function(err, result) {
          if(err) console.log(err);
          else {
            data = result.fetchAllSync();
            fs.writeFileSync('phool2.jpg', data[0].PHOTO, 'binary');
            fs.writeFileSync('trc2.fmt', data[0].TRACE, 'ascii');
            fs.writeFileSync('doc2.docx', data[0].DOCUMENT, 'binary');

            try {
              conn.querySync("drop table mytab");
            } catch (e) {};
  
            result.closeSync();
            var size1 = fs.statSync("phool2.jpg")["size"];
            var size2 = fs.statSync("trc2.fmt")["size"];
            var size3 = fs.statSync("doc2.docx")["size"];

            console.log("Lengths after select = " + size1 + ", " + size2 + ", " + size3);
            assert(len1, size1);
            assert(len2, size2);
            assert(len3, size3);

            fs.unlinkSync("phool2.jpg");
            fs.unlinkSync("doc2.docx");
            fs.unlink("trc2.fmt", function () { console.log('done'); });
          }
        });
      });
    });
  });
});

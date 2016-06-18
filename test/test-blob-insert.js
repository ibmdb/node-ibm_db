// Test insertion of BLOB and CLOB data using memory buffer.
// First read the file into buffer and then pass buffer to ibm_db.
// Author : bimaljha@in.ibm.com

var common = require("./common")
    , ibmdb = require("../")
    , assert = require("assert")
    , fs = require('fs')
    , cn = common.connectionString;

ibmdb.open(cn, function (err,conn) 
{
  if (err) 
  {
    return console.log(err);
  }
  try {
      conn.querySync("drop table mytab");
  } catch (e) {};
  try {
    conn.querySync("create table mytab (empId int, photo BLOB(30K), trace CLOB(20K))");
    } catch (e) {};
  
  var img1= fs.readFileSync('data/smile.jpg','binary');
  var text= fs.readFileSync('data/trace.txt','ascii');

  var len1 = img1.length;
  var len2 = text.length;
  console.log( "img1.length = " + len1);
  console.log( "text.length = " + len2);

  conn.prepare("insert into mytab(empId, photo, trace) VALUES (?, ?, ?)", 
      function (err, stmt) 
   {
    if (err) 
    {
      console.log(err);
      return conn.closeSync();
    }
    //var photo = [1, -2, -98, img1];  // We can use such array too.
    //var photo = {ParamType:"INPUT", CType:"BINARY", "SQLType:"BLOB", Data:img1};
    /*
     Default ParamType is 1(SQL_PARAM_INPUT)
     Default CType is 1 (SQL_C_CHAR)
     Default SQLType is 1. You must use SQLType or DataType
     if value is passed as JSON Object or Array.
     Except, numbers and string; all other datatypes like LOBS, GRAPHIC, File, etc
     must be passed as JSON Object or Array.
     */
    var photo = {ParamType:1, DataType: "BLOB", "Data":img1};
    var tracefile = {DataType: "CLOB", Data: text};

	stmt.execute([18, photo, tracefile], function (err, result) 
	{
      if( err ) console.log(err);  
      else result.closeSync();
	  
	  conn.prepare("select * from mytab", function (err, stmt)
      {
        if(err) 
        {
          console.log(err);
          return conn.closeSync();
        }

        stmt.execute([], function(err, result) {
          if(err) console.log(err);
          else 
          {
            data = result.fetchAllSync();
            fs.writeFileSync('smile2.jpg', data[0].PHOTO, 'hex');
            fs.writeFileSync('trace2.txt', data[0].TRACE, 'ascii');
            try {
                conn.querySync("drop table mytab");
            } catch (e) {};
            result.closeSync();
  
            var size1 = fs.statSync("smile2.jpg")["size"];
            var size2 = fs.statSync("trace2.txt")["size"];
            console.log("Lengths after select = " + size1+ ", " + size2);
            assert(len1, size1);
            assert(len2, size2);

            fs.unlinkSync("smile2.jpg");
            fs.unlink("trace2.txt", function () { console.log('done'); });
          }
        });
      });
	});
  });
});



// Test insertion of BLOB and CLOB data using FILE NAME.
// No need to read the file into memory buffer.
// Author: bimaljha@in.ibm.com

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
  
  var img1= 'data/smile.jpg'; //fs.readFileSync('smile.jpg','binary');
  var text= 'data/trace.txt'; //fs.readFileSync('trace.txt','ascii');

  var len1  = fs.statSync(img1)["size"];
  var len2  = fs.statSync(text)["size"];
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
    //var photo = [3, -2, -98, img1];  // We can use such array too.
    //var photo = {ParamType:"FILE", CType:"BINARY", "SQLType:"BLOB", Data:img1};
    // Except, numbers and string; all other datatypes like LOBS, GRAPHIC, File, etc
    // must be passed as JSON Object or Array.
    var photo = {ParamType:"FILE", DataType: "BLOB", "Data":img1};
    var tracefile = {ParamType: 3, "DataType": "CLOB", Data: text};

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



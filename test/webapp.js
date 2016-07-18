var ibmdb = require("../")
    , common = require("./common")
    , app = require('express')()
    , cn = common.connectionString
    , conn = ""
    , requestNo = 0
    , img1 = "data/pushpa.jpg"
    , img2 = "data/panda.jpg"
    ;

ibmdb.open(cn, function(err, connection) {
    if(err) {
        console.log(err);
        exit(1);
    }
    else conn = connection;

    try{
        conn.querySync("drop table imgtab");
    } catch (e) {};
    try{
        conn.querySync("create table imgtab (id int, filename varchar(50), image BLOB(200K))");
    } catch (e) {};
    
    conn.prepare("insert into imgtab (id, filename, image) values(?, ? , ?)", function(err, stmt){
        if(err) return console.log(err);
        stmt.execute([1, 'img1.jpg', {ParamType:"FILE", DataType:"BLOB", Data:img1}], 
            function(err, result){
              if(err) return console.log(err);
              console.log("image 1 inserted.");
            });
        stmt.execute([2, 'img2.jpg', {ParamType:"FILE", DataType:"BLOB", Data:img2}], 
            function(err, result){
              if(err) return console.log(err);
              else {result.closeSync();
              console.log("image 2 inserted.");}
            });
    });
});

app.listen(3000);

app.get('/', function(req, res) {
    res.sendFile('webapp.html', {root: __dirname });
});

app.get('/:id', function(req, res) {
  var imgid = req.params.id;
  var no = ++requestNo;
  console.log("Received id = " + imgid + " for request no "+no);
  if(imgid != parseInt(imgid)) return console.log("id is not int.");
  if(!conn) res.send("App is trying to establish connection with database server, please retry after some time.");
  else
  conn.query("SELECT id, filename, image FROM imgtab WHERE id=?", [imgid], function(err, rows) {
    if(err){
      return console.log(err);
    } else {
      res.set('Content-Type', 'image/jpeg');
      res.send(new Buffer(rows[0].IMAGE, 'binary'));
      res.end();
      console.log("File "+ rows[0].FILENAME + " sent for request no = "+no+", id = "+ imgid);
    }
  });
})


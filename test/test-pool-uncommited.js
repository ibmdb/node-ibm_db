var common = require("./common")
	, ibmdb = require("../")
	, pool = new ibmdb.Pool()
	, cn = common.connectionString


var request =  function (err, conn) {
  if (err) {
    console.log(err);
    process.exit(-1);
  }
  try {
      conn.querySync("drop table mytab4");
  } catch(e) {};
  conn.beginTransaction(function (err) {
    if (err) {
      //could not begin a transaction for some reason. 
      console.log(err);
      return conn.closeSync();
    }
    conn.querySync("create table mytab4 (c1 int, c2 varchar(20))");
    conn.querySync("insert into mytab4 values( 3, 'bimal')");
    console.log(conn.querySync("select * from mytab4"));
    conn.close(function(){});
  });
}

pool.open(cn, request);
pool.open(cn, request);
setTimeout(function() {
    pool.open(cn, request);
    pool.open(cn, request);
}, 8000);

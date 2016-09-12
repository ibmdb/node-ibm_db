var common = require("./common")
	, odbc = require("../")
	, pool = new odbc.Pool()
	, cn = common.connectionString


pool.open(cn, function (err, conn) {
  if (err) {
    return console.log(err);
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
    conn.rollbackTransaction(function (err) {
      if (err) {
        //error during commit 
        console.log(err);
        return conn.closeSync();
      }
    });
  });
});



import * as ibmdb from '../index';

function testFetchMode() {
  // Valid
  ibmdb.open('db', { fetchMode: 0 });
  ibmdb.open('db', { fetchMode: 3 });
  ibmdb.open('db', { fetchMode: 4 });
  ibmdb.open('db', { fetchMode: ibmdb.FETCH_ARRAY });
  ibmdb.open('db', { fetchMode: ibmdb.FETCH_OBJECT });
  ibmdb.open('db', { fetchMode: ibmdb.FETCH_NODATA });

  // Invalid
  ibmdb.open('db', { fetchMode: 18 });
}

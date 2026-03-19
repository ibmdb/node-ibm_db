import * as ibmdb from '../index';
import { Readable } from 'node:stream';

// At runtime, use the real ibm_db module (TypeScript stubs have no values).
// The 'ibmdb' import is for compile-time type checking.
// eslint-disable-next-line @typescript-eslint/no-var-requires
const db2 = require('../../lib/odbc') as typeof ibmdb;

// =============================================================
// TypeScript type-validation tests for ibm_db
// These verify that the type definitions accept correct types
// and reject incorrect types at compile time.
// =============================================================

let passed = 0;
let failed = 0;

function check(name: string, fn: () => boolean) {
  try {
    if (fn()) {
      console.log(`  PASS: ${name}`);
      passed++;
    } else {
      console.log(`  FAIL: ${name}`);
      failed++;
    }
  } catch (e: any) {
    console.log(`  FAIL: ${name} - ${e.message}`);
    failed++;
  }
}

// --- Test 1: FetchMode constants ---
console.log('\n--- Test 1: FetchMode constants ---');
check('FETCH_ARRAY is defined', () => db2.FETCH_ARRAY !== undefined);
check('FETCH_OBJECT is defined', () => db2.FETCH_OBJECT !== undefined);
check('FETCH_NODATA is defined', () => db2.FETCH_NODATA !== undefined);

// --- Test 2: SQL_FILE_* constants ---
console.log('\n--- Test 2: SQL_FILE_* constants ---');
check('SQL_FILE_READ is number', () => typeof db2.SQL_FILE_READ === 'number');
check('SQL_FILE_CREATE is number', () => typeof db2.SQL_FILE_CREATE === 'number');
check('SQL_FILE_OVERWRITE is number', () => typeof db2.SQL_FILE_OVERWRITE === 'number');
check('SQL_FILE_APPEND is number', () => typeof db2.SQL_FILE_APPEND === 'number');
check('SQL_FILE_READ == 2', () => db2.SQL_FILE_READ === 2);
check('SQL_FILE_CREATE == 8', () => db2.SQL_FILE_CREATE === 8);
check('SQL_FILE_OVERWRITE == 16', () => db2.SQL_FILE_OVERWRITE === 16);
check('SQL_FILE_APPEND == 32', () => db2.SQL_FILE_APPEND === 32);

// --- Test 3: Statement attribute constants ---
console.log('\n--- Test 3: Attribute constants ---');
check('SQL_ATTR_QUERY_TIMEOUT is number', () => typeof db2.SQL_ATTR_QUERY_TIMEOUT === 'number');
check('SQL_ATTR_PARAMSET_SIZE is number', () => typeof db2.SQL_ATTR_PARAMSET_SIZE === 'number');
check('SQL_ATTR_ROW_ARRAY_SIZE is number', () => typeof db2.SQL_ATTR_ROW_ARRAY_SIZE === 'number');

// --- Test 4: Type info constants ---
console.log('\n--- Test 4: Type info constants ---');
check('BLOB is number', () => typeof db2.BLOB === 'number');
check('CLOB is number', () => typeof db2.CLOB === 'number');
check('INTEGER is number', () => typeof db2.INTEGER === 'number');
check('VARCHAR is number', () => typeof db2.VARCHAR === 'number');

// --- Test 5: Function ID constants ---
console.log('\n--- Test 5: Function ID constants ---');
check('SQLBINDFILETOCOL exists', () => typeof db2.SQLBINDFILETOCOL === 'number');
check('SQLBINDFILETOPARAM exists', () => typeof db2.SQLBINDFILETOPARAM === 'number');
check('SQLPUTDATA exists', () => typeof db2.SQLPUTDATA === 'number');
check('SQLPARAMDATA exists', () => typeof db2.SQLPARAMDATA === 'number');

// --- Test 6: SQLParamObject type structure ---
console.log('\n--- Test 6: SQLParamObject type checks ---');
check('SQLParamObject with Buffer Data compiles', () => {
  const p: ibmdb.SQLParamObject = {
    ParamType: 'INPUT', Data: Buffer.from('test'), Length: 4
  };
  return p.ParamType === 'INPUT';
});
check('SQLParamObject with Buffer[] Data compiles', () => {
  const chunks: Buffer[] = [Buffer.from('chunk1'), Buffer.from('chunk2')];
  const p: ibmdb.SQLParamObject = {
    ParamType: 'INPUT', Data: chunks, CType: 'BINARY', SQLType: 'BLOB'
  };
  return Array.isArray(p.Data) && p.Data.length === 2;
});
check('SQLParamObject with Readable Data compiles', () => {
  const stream = new Readable({ read() {} });
  const p: ibmdb.SQLParamObject = {
    ParamType: 'INPUT', Data: stream, CType: 'BINARY', SQLType: 'BLOB'
  };
  return p.Data instanceof Readable;
});
check('SQLParamObject Length is optional', () => {
  const p: ibmdb.SQLParamObject = {
    ParamType: 'INPUT', Data: Buffer.from('test')
  };
  return p.Length === undefined;
});

// --- Test 7: FetchOptions type ---
console.log('\n--- Test 7: FetchOptions type checks ---');
check('FetchOptions with fetchMode compiles', () => {
  const opts: ibmdb.FetchOptions = { fetchMode: db2.FETCH_ARRAY };
  return opts.fetchMode !== undefined;
});

// --- Test 8: FileOption type ---
console.log('\n--- Test 8: FileOption type checks ---');
check('FileOption accepts 8 (SQL_FILE_CREATE)', () => {
  const opt: ibmdb.FileOption = 8;
  return opt === 8;
});
check('FileOption accepts 16 (SQL_FILE_OVERWRITE)', () => {
  const opt: ibmdb.FileOption = 16;
  return opt === 16;
});
check('FileOption accepts 32 (SQL_FILE_APPEND)', () => {
  const opt: ibmdb.FileOption = 32;
  return opt === 32;
});

// --- Test 9: open/openSync type (no actual connection) ---
console.log('\n--- Test 9: Module API types ---');
check('db2.open is a function', () => typeof db2.open === 'function');
check('db2.openSync is a function', () => typeof db2.openSync === 'function');
check('db2.debug is a function', () => typeof db2.debug === 'function');
check('db2.getElapsedTime is a function', () => typeof db2.getElapsedTime === 'function');

// Invalid FetchMode - compile-time check only (suppressed with @ts-expect-error)
// @ts-expect-error: 18 is not a valid FetchMode
const _invalidOpts: ibmdb.Options = { fetchMode: 18 };

// --- Summary ---
console.log(`\n=== Results: ${passed} passed, ${failed} failed ===`);
if (failed > 0) process.exit(1);

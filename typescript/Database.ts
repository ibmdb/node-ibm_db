import { ConnStr } from './ConnStr';
import { DescribeObject } from './DescribeObject';
import { ODBC } from './ODBC';
import { ODBCConnection } from './ODBCConnection';
import { ODBCResult, SQLResults } from './ODBCResult';
import { ODBCStatement } from './ODBCStatement';
import { Options } from './Options';
import { Pool } from './Pool';
import { FetchMode } from './attributes';
import { DB2Error } from './DB2Error';
import { Readable } from 'node:stream';

export interface SQLCA {
  sqlstate: string;
  sqlcode: number;
}

export interface SQLParamObject {
  ParamType:
    | 'INPUT'
    | 'OUTPUT'
    | 'INOUT'
    | 'FILE'
    | 'ARRAY'
    | 1 // Same as 'INPUT'
    | 2 // Same as 'INOUT'
    | 3 // Same as 'FILE'
    | 4; // Same as 'OUTPUT'
  CType?:
    | 'CHAR'
    | 'BINARY'
    | 'DOUBLE'
    | 'DECIMAL'
    | 'INTEGER'
    | 'BIGINT'
    | 1 // Same as 'CHAR'
    | -2 // Same as 'BINARY'
    | 8 // Same as 'DOUBLE' and 'DECIMAL'
    | 4 // Same as 'INTEGER'
    | -25; // Same as 'BIGINT'
  SQLType?:
    | 'DOUBLE'
    | 'DECIMAL'
    | 'BIGINT'
    | 'CHAR'
    | 'BINARY'
    | 'BLOB'
    | 'CLOB'
    | 'DBCLOB'
    | 'XML'
    | 'GRAPHIC'
    | 'VARGRAPHIC'
    | 'LONGGRAPHIC'
    | 8 // Same as 'DOUBLE'
    | 3 // Same as 'DECIMAL'
    | -5 // Same as 'BIGINT'
    | 1 // Same as 'CHAR'
    | -2 // Same as 'BINARY'
    | -98 // Same as 'BLOB'
    | -99 // Same as 'CLOB'
    | -350 // Same as 'DBCLOB'
    | -370 // Same as 'XML'
    | -95 // Same as 'GRAPHIC'
    | -96 // Same as 'VARGRAPHIC'
    | -97; // Same as 'LONGGRAPHIC'
  DataType?: number | string;
  Data: number | '' | Buffer;
  Length: number;
}

export type SQLParam = string | number | null | SQLParamObject;

export interface SQLQuery {
  sql: string;
  params?: SQLParam[];
  noResults?: boolean;
  ArraySize?: number;
}

export interface SQLExecuteFileParam {
  sql: string;
  delimiter: string;
  outputfile: string;
}

export type CloseOption = number & { __TYPE__: 'CloseOption' };

export class Database {
  odbc: ODBC;
  fetchMode: FetchMode | null;
  connected: boolean;
  connectTimeout: number | null;
  systemNaming?: boolean;
  codeSet: string | null;
  mode: string | null;
  pool: Pool | null;
  connStr: string | null;
  conn?: ODBCConnection;

  SQL_CLOSE: CloseOption;
  SQL_DROP: CloseOption;
  SQL_UNBIND: CloseOption;
  SQL_RESET_PARAMS: CloseOption;
  SQL_DESTROY: CloseOption;
  FETCH_ARRAY: FetchMode;
  FETCH_OBJECT: FetchMode;

  constructor(options?: Options);
  constructor() {}

  open(connStr: string | ConnStr): Promise<void>;
  open(connStr: string | ConnStr, cb: (err?: DB2Error) => void): null;
  open(): any {}

  openSync(connStr: string | ConnStr): true;
  openSync(): any {}

  close(cb: (err: null | Error) => void): false;
  close(): Promise<true>;
  close(): any {}

  closeSync(): true | undefined;
  closeSync(): any {}

  checkConnectionError(error: Error): void;
  checkConnectionError() {}

  query(
    query: string,
    params: SQLParam[],
    cb: (err: Error, outputParam: SQLResults, sqlca: SQLCA) => void
  ): false;
  query(
    query: string | SQLQuery,
    cb: (err: Error, outputParam: SQLResults, sqlca: SQLCA) => void
  ): false;
  query(query: string | SQLQuery, params?: SQLParam[]): Promise<SQLResults>;
  query(): any {}

  queryResult(
    query: string | SQLQuery,
    params: SQLParam[],
    cb: (err: Error, res?: null | ODBCResult, outputParam?: SQLResults) => void
  ): false;
  queryResult(
    query: string | SQLQuery,
    cb: (err: Error, res?: null | ODBCResult, outputParam?: SQLResults) => void
  ): false;
  queryResult(
    query: string | SQLQuery,
    params?: SQLParam[]
  ): Promise<[result: null | ODBCResult, outparams: SQLResults]>;
  queryResult(): any {}

  queryResultSync(
    query: string | SQLQuery,
    params?: SQLParam[]
  ): ODBCResult | [ODBCResult | null, SQLResults];
  queryResultSync(): any {}

  querySync(
    query: string | SQLQuery,
    params?: SQLParam[]
  ): SQLResults | undefined | null;
  querySync(): any {}

  executeFile(
    inputfile: string,
    delimiter: string,
    outputfile: string | undefined,
    cb: (err: SQLResults, res: string) => void
  ): void;
  executeFile(
    inputfile: string,
    delimiter: string,
    cb: (err: SQLResults, res: string) => void
  ): void;
  executeFile(
    inputfile: string | SQLExecuteFileParam,
    cb: (err: SQLResults, res: string) => void
  ): void;
  // BUGBUG: This function has a bug in it where it doesn't return the promise
  executeFile(
    inputfile: string | SQLExecuteFileParam,
    delimiter?: string,
    outputfile?: string
  ): Promise<string>;
  executeFile(): any {}

  executeFileSync(
    inputfile: string | SQLExecuteFileParam,
    delimiter?: string,
    outputfile?: string | undefined
  ): string;
  executeFileSync(): any {}

  // Emits a stream of RecordTupleOrArray
  queryStream(sql: string | SQLQuery, params?: SQLParam[]): Readable;
  queryStream(): any {}

  // Emits a stream of RecordTupleOrArray for each result
  fetchStreamingResults(results: ODBCResult, stream: Readable): void;
  fetchStreamingResults(): any {}

  beginTransaction(cb: (err: DB2Error | null) => void): Database;
  beginTransaction(): Promise<true>;
  beginTransaction(): any {}

  endTransaction(
    rollback: boolean,
    cb: (err: DB2Error | null) => void
  ): Database;
  endTransaction(): Promise<true>;
  endTransaction(): any {}

  commitTransaction(cb: (err: DB2Error | null) => void): Database;
  commitTransaction(): Promise<void>;
  commitTransaction(): any {}

  rollbackTransaction(cb: (err: DB2Error | null) => void): Database;
  rollbackTransaction(): Promise<void>;
  rollbackTransaction(): any {}

  beginTransactionSync(): Database;
  beginTransactionSync(): any {}

  endTransactionSync(rollback: boolean): Database;
  endTransactionSync(): any {}

  commitTransactionSync(): Database;
  commitTransactionSync(): any {}

  rollbackTransactionSync(): Database;
  rollbackTransactionSync(): any {}

  columns(
    catalog: string | null,
    schema: string | null,
    table: string | null,
    column: string | null,
    cb: (
      err: null | DB2Error,
      res: SQLResults | null,
      errInColumnsCall?: false
    ) => void
  ): void;
  columns(
    catalog: string | null,
    schema: string | null,
    table: string | null,
    column: string | null
  ): Promise<SQLResults | null>;
  columns(): any {}

  tables(
    catalog: string | null,
    schema: string | null,
    table: string | null,
    type: string | null,
    cb: (
      err: null | DB2Error,
      res: SQLResults | null,
      errInColumnsCall?: false
    ) => void
  ): void;
  tables(
    catalog: string | null,
    schema: string | null,
    table: string | null,
    type: string | null
  ): Promise<SQLResults | null>;
  tables(): any {}

  describe(
    obj: DescribeObject,
    cb: (
      err: null | DB2Error,
      res: SQLResults | null,
      errInColumnsCall?: false
    ) => void
  ): void;
  describe(obj: DescribeObject): Promise<SQLResults | null>;
  describe(): any {}

  prepare(
    sql: string,
    cb: (err: DB2Error | null, stmt: ODBCStatement) => void
  ): void;
  prepare(sql: string): Promise<ODBCStatement>;
  prepare(): any {}

  prepareSync(sql: string): ODBCStatement;
  prepareSync(): any {}

  setIsolationLevel(isolationLevel: 1 | 2 | 4 | 8 | 32): true;
  setIsolationLevel(): any {}

  getInfoSync(
    infoType: number,
    infoLen: number
  ): false | null | string | number;
  getInfoSync(): any {}

  getInfo(
    infoType: number,
    infoLen: number,
    cb: (err: DB2Error | null, info: null | string | number) => void
  ): void;
  getInfo(
    infoType: number,
    cb: (err: DB2Error | null, info: null | string | number) => void
  ): void;
  getInfo(infoType: number, infoLen?: number): Promise<null | string | number>;
  getInfo(): any {}

  getTypeInfoSync(dataType: number): SQLResults;
  getTypeInfoSync(): any {}

  getTypeInfo(
    dataType: number,
    cb: (err: DB2Error | null, info: false | SQLResults | null) => void
  ): void;
  getTypeInfo(dataType: number): Promise<SQLResults>;
  getTypeInfo(): any {}

  getFunctionsSync(functionId: number): boolean;
  getFunctionsSync(functionId: 0): false | Record<string, boolean>;
  getFunctionsSync(): any {}

  getFunctions(
    functionId: number,
    cb: (err: DB2Error | null, res: boolean) => void
  ): void;
  getFunctions(
    functionId: 0,
    cb: (err: DB2Error | null, res: Record<string, boolean>) => void
  ): void;
  getFunctions(): any {}

  setAttr(
    attr: number | string,
    value: number | null | string,
    cb: (err: DB2Error | null, res?: true) => void
  ): void;
  setAttr(attr: number | string, value: number | null | string): Promise<true>;
  setAttr(): any {}

  setAttrSync(attr: number, value: number | null | string): boolean;
  setAttrSync(): any {}
}

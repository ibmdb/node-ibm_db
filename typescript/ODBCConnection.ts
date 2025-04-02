import { SQLParam, SQLQuery } from './Database';
import { DB2Error } from './DB2Error';
import { ODBCResult, SQLResults } from './ODBCResult';
import { ODBCStatement } from './ODBCStatement';

export class ODBCConnection {
  loginTimeout: number;
  connectTimeout: number;
  connected: boolean;
  systemNaming: boolean;

  private constructor() {
    // internal only
  }

  open(connStr: string, cb: (err?: DB2Error) => void): ODBCConnection;
  open(): any {}

  openSync(connStr: string): true;
  openSync(): any {}

  close(cb: (err?: Error) => void): undefined;
  close() {}

  closeSync(): true;
  closeSync(): any {}

  createDbSync(
    dbName: string,
    codeSet: string | null,
    mode: string | null
  ): boolean;
  createDbSync(): any {}

  dropDbSync(dbName: string): boolean;
  dropDbSync(): any {}

  createStatement(
    cb: (err: DB2Error, odbcStatement: ODBCStatement) => void
  ): void;
  createStatement(): any {}

  createStatementSync(): ODBCStatement;
  createStatementSync(): any {}

  query(
    query: string,
    params: SQLParam[],
    cb: (
      err: null | DB2Error,
      res: null | ODBCResult,
      outputParam: SQLResults
    ) => void
  ): void;
  query(
    query: string | SQLQuery,
    cb: (
      err: null | DB2Error,
      res: null | ODBCResult,
      outputParam: SQLResults
    ) => void
  ): void;
  query(): void {}

  querySync(
    query: string,
    params?: SQLParam[]
  ): ODBCResult | [ODBCResult | null, SQLResults | undefined];
  querySync(): any {}

  tables(
    catalog: string | null,
    schema: string | null,
    table: string | null,
    type: string | null,
    cb: (
      err: null | DB2Error,
      res: null | ODBCResult,
      outputParam: SQLResults
    ) => void
  ): void;
  tables() {}

  columns(
    catalog: string | null,
    schema: string | null,
    table: string | null,
    column: string | null,
    cb: (
      err: null | DB2Error,
      res: null | ODBCResult,
      outputParam: SQLResults
    ) => void
  ): void;
  columns() {}

  beginTransactionSync(): boolean;
  beginTransactionSync(): any {}

  beginTransaction(cb: (err?: DB2Error) => void): void;
  beginTransaction(): any {}

  endTransactionSync(rollback: boolean): boolean;
  endTransactionSync(): any {}

  endTransaction(rollback: boolean, cb: (err?: DB2Error) => void): void;
  endTransaction(): any {}

  setIsolationLevel(isolationLevel?: number): boolean;
  setIsolationLevel(): any {}

  getInfoSync(
    infotype: number,
    infolen: number
  ): false | null | string | number;
  getInfoSync(): any {}

  getInfo(
    infotype: number,
    infolen: number,
    cb: (err: DB2Error | null, info: null | string | number) => void
  ): void;
  getInfo(): any {}

  getTypeInfoSync(datatype: number): ODBCResult;
  getTypeInfoSync(): any {}

  getTypeInfo(
    datatype: number,
    cb: (err: DB2Error | null, res: ODBCResult | null) => void
  ): void;
  getTypeInfo(): any {}

  getFunctionsSync(functionId: number): false | number;
  getFunctionsSync(functionId: 0): false | number[];
  getFunctionsSync(): any {}

  getFunctions(
    functionId: number,
    cb: (err: DB2Error | null, value: number | null) => void
  ): void;
  getFunctions(
    functionId: 0,
    cb: (err: DB2Error | null, value: number[] | null) => void
  ): void;
  getFunctions() {}

  setAttrSync(attr: number, value: number | null | string): boolean;
  setAttrSync(): any {}

  setAttr(
    attr: number,
    value: number | null | string,
    cb: (err: DB2Error | null, res?: true) => void
  ): void;
  setAttr() {}
}

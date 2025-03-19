import { FetchMode } from './attributes';
import { CloseOption } from './Database';
import { DB2Error } from './DB2Error';
import { ColumnValue, RecordArray, RecordTuple } from './ODBC';

export type RecordTupleOrArray = RecordArray | RecordTuple;

export type SQLResults = Array<RecordTupleOrArray>;

export interface FetchOptions {
  fetchMode: FetchMode;
}

export interface Column {
  index: number;
  SQL_DESC_NAME: string;
  SQL_DESC_TYPE_NAME: string;
  SQL_DESC_CONSIZE_TYPE: number;
  SQL_DESC_DISPLAY_SIZE: number;
  SQL_DESC_PRECISION: number;
  SQL_DESC_SCALE: number;
  SQL_DESC_LENGTH: number;
}

export class ODBCResult {
  private constructor() {
    // Internal only
  }

  fetchMode: number; // Defaults to FETCH_OBJECT

  fetch(
    options: FetchOptions,
    cb: (err: null | DB2Error, data: RecordTupleOrArray | null) => void
  ): void;
  fetch(
    cb: (err: null | DB2Error, data: RecordTupleOrArray | null) => void
  ): void;
  fetch(options?: FetchOptions): Promise<RecordTupleOrArray | null>;
  fetch(): any {}

  fetchSync(options?: FetchOptions): null | RecordTupleOrArray;
  fetchSync(): any {}

  fetchAll(
    options: FetchOptions,
    cb: (
      err: DB2Error | null,
      data: SQLResults | null,
      colCount: number
    ) => void
  ): void;
  fetchAll(
    cb: (
      err: DB2Error | null,
      data: SQLResults | null,
      colCount: number
    ) => void
  ): void;
  fetchAll(options?: FetchOptions): Promise<SQLResults>;
  fetchAll(): any {}

  fetchAllSync(options?: FetchOptions): SQLResults;
  fetchAllSync(): any {}

  getData(
    colNum: number,
    dataSize: number,
    cb: (err: null | DB2Error, row: ColumnValue | null) => void
  ): void;
  getData(
    colNum: number,
    cb: (err: null | DB2Error, row: ColumnValue | null) => void
  ): void;
  getData(colNum?: number, dataSize?: number): Promise<ColumnValue>;
  getData(): any {}

  getDataSync(colNum: number, dataSize: number): ColumnValue | null;
  getDataSync(): any {}

  close(closeOption: CloseOption, cb: (err: DB2Error | null) => void): void;
  close(cb: (err: DB2Error | null) => void): void;
  close(closeOption?: CloseOption): Promise<false>;
  close(): any {}

  closeSync(closeOption?: CloseOption): true;
  closeSync(): any {}

  moreResultsSync(): boolean;
  moreResultsSync(): any {}

  getColumnNamesSync(): string[];
  getColumnNamesSync(): any {}

  getColumnMetadataSync(): Array<Column>;
  getColumnMetadataSync(): any {}

  getSQLErrorSync(): DB2Error;
  getSQLErrorSync(): any {}

  getAffectedRowsSync(): number;
  getAffectedRowsSync(): any {}
}

import { SQLParam } from './Database';
import { DB2Error } from './DB2Error';
import { ODBCResult } from './ODBCResult';

export class ODBCStatement {
  private constructor() {
    // Internal only
  }

  prepareSync(sql: string): boolean;
  prepareSync(): any {}

  closeSync(closeOption: number): true;
  closeSync(): any {}

  execute(
    params: SQLParam[],
    cb: (
      err: DB2Error,
      result?: ODBCResult,
      outparams?: Array<null | number | boolean | string> | null
    ) => void
  ): void;
  execute(
    cb: (
      err: DB2Error,
      result?: ODBCResult,
      outparams?: Array<null | number | boolean | string> | null
    ) => void
  ): void;
  execute(
    params?: SQLParam[]
  ): Promise<
    [ODBCResult, Array<null | number | boolean | string>] | ODBCResult
  >;
  execute(): any {}

  executeSync(
    params?: SQLParam[]
  ): [ODBCResult, Array<null | number | boolean | string>] | ODBCResult | null;
  executeSync(): any {}

  executeDirect(
    sql: string,
    cb: (err: DB2Error | null, result?: ODBCResult) => void
  ): null;
  executeDirect(sql: string): Promise<ODBCResult>;
  executeDirect(): any {}

  executeNonQuery(
    params: SQLParam[],
    cb: (err: DB2Error | null, res?: number) => void
  ): null;
  executeNonQuery(cb: (err: DB2Error | null, res?: number) => void): null;
  executeNonQuery(params?: SQLParam[]): Promise<number>;
  executeNonQuery(): any {}

  executeNonQuerySync(params: SQLParam[]): null | number;
  executeNonQuerySync(): any {}

  prepare(sql: string, cb: (err: DB2Error | null) => void): null;
  prepare(sql: string): Promise<true>;
  prepare(): any {}

  bind(params: SQLParam[], cb: (err: DB2Error | null) => void): void;
  bind(cb: (err: DB2Error | null) => void): void;
  bind(params?: SQLParam[]): Promise<true>;
  bind(): any {}

  bindSync(params: SQLParam[]): boolean;
  bindSync(): any {}

  setAttr(
    attr: number,
    value: number | null | string,
    cb: (err: DB2Error | null) => void
  ): null;
  setAttr(attr: number, value: number | null | string): Promise<true>;
  setAttr(): any {}

  setAttrSync(attr: number, value: number | null | string): boolean;
  setAttrSync(): any {}

  close(closeOption: number, cb: (err: DB2Error | null) => void): void;
  close(cb: (err: DB2Error | null) => void): void;
  close(closeOption?: number): Promise<false>;
  close(): any {}

  primaryKeys(
    catalog: string | null,
    schema: string | null,
    table: string,
    cb: (err: DB2Error | null, rows?: object[]) => void
  ): void;
  primaryKeys(
    catalog: string | null,
    schema: string | null,
    table: string
  ): Promise<object[]>;
  primaryKeys(): any {}

  primaryKeysSync(
    catalog: string | null,
    schema: string | null,
    table: string
  ): object[];
  primaryKeysSync(): any {}

  foreignKeys(
    pkCatalog: string | null,
    pkSchema: string | null,
    pkTable: string | null,
    fkCatalog: string | null,
    fkSchema: string | null,
    fkTable: string | null,
    cb: (err: DB2Error | null, rows?: object[]) => void
  ): void;
  foreignKeys(
    pkCatalog: string | null,
    pkSchema: string | null,
    pkTable: string | null,
    fkCatalog: string | null,
    fkSchema: string | null,
    fkTable: string | null
  ): Promise<object[]>;
  foreignKeys(): any {}

  foreignKeysSync(
    pkCatalog: string | null,
    pkSchema: string | null,
    pkTable: string | null,
    fkCatalog: string | null,
    fkSchema: string | null,
    fkTable: string | null
  ): object[];
  foreignKeysSync(): any {}

  procedures(
    catalog: string | null,
    schema: string | null,
    procedure: string | null,
    cb: (err: DB2Error | null, rows?: object[]) => void
  ): void;
  procedures(
    catalog: string | null,
    schema: string | null,
    procedure: string | null
  ): Promise<object[]>;
  procedures(): any {}

  proceduresSync(
    catalog: string | null,
    schema: string | null,
    procedure: string | null
  ): object[];
  proceduresSync(): any {}

  procedureColumns(
    catalog: string | null,
    schema: string | null,
    procedure: string | null,
    column: string | null,
    cb: (err: DB2Error | null, rows?: object[]) => void
  ): void;
  procedureColumns(
    catalog: string | null,
    schema: string | null,
    procedure: string | null,
    column: string | null
  ): Promise<object[]>;
  procedureColumns(): any {}

  procedureColumnsSync(
    catalog: string | null,
    schema: string | null,
    procedure: string | null,
    column: string | null
  ): object[];
  procedureColumnsSync(): any {}

  /**
   * Result object returned by paramData/paramDataSync
   */
  // ParamDataResult interface is defined below

  /**
   * Get next parameter for which a data value is needed.
   * Used with putData for sending large data in chunks.
   */
  paramData(
    cb: (err: DB2Error | null, result?: ParamDataResult) => void
  ): void;
  paramData(): Promise<ParamDataResult>;
  paramData(): any {}

  /**
   * Synchronously get next parameter for which a data value is needed.
   */
  paramDataSync(): ParamDataResult;
  paramDataSync(): any {}

  /**
   * Send data value for a parameter.
   * Used with paramData for sending large data in chunks.
   */
  putData(
    data: Buffer | string | null,
    length: number,
    cb: (err: DB2Error | null, result?: boolean) => void
  ): void;
  putData(
    data: Buffer | string | null,
    cb: (err: DB2Error | null, result?: boolean) => void
  ): void;
  putData(data: Buffer | string | null, length?: number): Promise<boolean>;
  putData(): any {}

  /**
   * Synchronously send data value for a parameter.
   */
  putDataSync(data: Buffer | string | null, length?: number): boolean;
  putDataSync(): any {}

  /**
   * Execute the prepared statement for streaming.
   * Does NOT auto-complete the data-at-execution loop.
   * Returns { needsData: boolean } to indicate if paramData/putData calls are required.
   */
  executeForStreaming(
    cb: (err: DB2Error | null, result?: ExecuteStreamingResult) => void
  ): void;
  executeForStreaming(): Promise<ExecuteStreamingResult>;
  executeForStreaming(): any {}

  /**
   * Synchronous version of executeForStreaming.
   */
  executeForStreamingSync(): ExecuteStreamingResult;
  executeForStreamingSync(): any {}

  /**
   * High-level streaming helper.
   * Executes the statement and streams data from a Readable stream
   * to a data-at-execution parameter without buffering.
   */
  executeWithStream(
    streamParamIndex: number,
    stream: NodeJS.ReadableStream,
    cb: (err: DB2Error | null, result?: StreamResult) => void
  ): void;
  executeWithStream(
    stream: NodeJS.ReadableStream,
    cb: (err: DB2Error | null, result?: StreamResult) => void
  ): void;
  executeWithStream(
    streamParamIndex: number,
    stream: NodeJS.ReadableStream
  ): Promise<StreamResult>;
  executeWithStream(stream: NodeJS.ReadableStream): Promise<StreamResult>;
  executeWithStream(): any {}

  /**
   * Cancel this executing SQL statement.
   * Calls SQLCancel ODBC API.
   * @param cb - Optional callback. If not provided, returns a Promise.
   */
  cancel(cb: (err: DB2Error | null, result?: boolean) => void): void;
  cancel(): Promise<boolean>;
  cancel(): any {}

  /**
   * Synchronous version of cancel.
   * @returns true on success
   */
  cancelSync(): boolean;
  cancelSync(): any {}
}

/**
 * Result object returned by executeForStreaming/executeForStreamingSync
 */
export interface ExecuteStreamingResult {
  /** True if data-at-execution parameters need data via paramData/putData */
  needsData: boolean;
}

/**
 * Result object returned by executeWithStream
 */
export interface StreamResult {
  /** True if streaming was performed */
  streamed: boolean;
  /** Index of the parameter that received streamed data */
  paramIndex?: number;
}

/**
 * Result object returned by paramData/paramDataSync
 */
export interface ParamDataResult {
  /** True if SQL_NEED_DATA was returned, meaning more data is needed */
  needsData: boolean;
  /** The parameter index that needs data (only set when needsData is true) */
  paramIndex: number | null;
}

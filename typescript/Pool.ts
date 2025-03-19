import { ConnStr } from './ConnStr';
import { Database } from './Database';
import { DB2Error } from './DB2Error';
import { ODBC } from './ODBC';
import { PoolOptions } from './PoolOptions';

export class Pool {
  options: PoolOptions;
  maxPoolSize: number;
  index: number;
  availablePool: object;
  usedPool: object;
  poolSize: number;
  odbc: ODBC;

  constructor(options?: PoolOptions);
  constructor() {}

  open(connStr: string, cb: (err: DB2Error | null, db: Database) => void): null;
  open(connStr: string): Promise<Database>;
  open(): any {}

  openSync(connStr: string): Database;
  openSync(): any {}

  poolCloseSync(): void;
  poolCloseSync(): any {}

  poolClose(cb: (err: null) => void): void;
  poolClose(): Promise<true>;
  poolClose(): any {}

  init(count: number, connStr: string): DB2Error | true;
  init(): any {}

  initAsync(
    count: number,
    connStr: string | ConnStr,
    cb: (err: Error | null) => void
  ): DB2Error | true;
  initAsync(count: number, connStr: string | ConnStr): Promise<true>;
  initAsync(): any {}

  setMaxPoolSize(size: number): true;
  setMaxPoolSize(): any {}

  // DEPRECATED
  setConnectTimeout(timeout: number): true;
  setConnectTimeout(): any {}

  cleanup(connStr: string): boolean | undefined;
  cleanup(): any {}

  close(cb: () => void): void;
  close(): Promise<true>;
  close(): any {}

  closeSync(): null | true;
  closeSync(): any {}
}

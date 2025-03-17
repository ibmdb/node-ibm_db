import { ConnStr } from './ConnStr';
import { Database } from './Database';
import { Options } from './Options';

export * from './attributes';
export * from './ConnStr';
export * from './Database';
export * from './DescribeObject';
export * from './ODBC';
export * from './ODBCConnection';
export * from './ODBCResult';
export * from './ODBCStatement';
export * from './Options';
export * from './Pool';
export * from './PoolOptions';
export * from './DB2Error';

export default function (options?: Options): Database;
export default function (): any {}

export function getElapsedTime(): string;
export function getElapsedTime(): any {}

export function debug(x: boolean | 2): void;
export function debug(): any {}

export function open(
  connStr: string | ConnStr,
  options: Options | null,
  cb?: (err: Error, db: Database) => void
): void;
export function open(
  connStr: string | ConnStr,
  cb?: (err: Error, db: Database) => void
): void;
export function open(
  connStr: string | ConnStr,
  options?: Options | null
): Promise<Database>;
export function open(): any {}

export function openSync(
  connStr: string | ConnStr,
  options?: Options
): Database;
export function openSync(): any {}

export function createDbSync(
  dbName: string,
  connStr: string | ConnStr,
  options: Options
): boolean;
export function createDbSync(): any {}

export function dropDbSync(
  dbName: string,
  connStr: string | ConnStr,
  options: Options
): boolean;
export function dropDbSync(): any {}

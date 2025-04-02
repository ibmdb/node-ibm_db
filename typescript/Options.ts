import { ODBC } from './ODBC';
import { FetchMode } from './attributes';
import { Pool } from './Pool';

export interface Options {
  odbc?: ODBC;
  fetchMode?: 0 | 3 | 4 | FetchMode | null;
  connected?: boolean;
  connectTimeout?: number | null;
  systemNaming?: boolean;
  codeSet?: string | null;
  mode?: string | null;
  pool?: Pool | null;
  connStr?: string | null;
}

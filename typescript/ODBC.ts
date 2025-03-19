import { DB2Error } from './DB2Error';
import { ODBCConnection } from './ODBCConnection';

export type ColumnValue = number | null | Date | boolean | Buffer | string;

export type RecordArray = Array<ColumnValue>;
export type RecordTuple = Record<string, ColumnValue>;

export type ArrayParam =
  | [
      number /*paramtype*/,
      number /*c_type*/,
      number /*type*/,
      (
        | null
        | number
        | boolean
        | Array<null | number | boolean | Buffer | string>
        | Buffer
        | string
      )
    ]
  | [
      number /*paramtype*/,
      number /*c_type*/,
      number /*type*/,
      (
        | null
        | number
        | boolean
        | Array<null | number | boolean | Buffer | string>
        | Buffer
        | string
      ),
      number /*buffer length*/
    ];

export class ODBC {
  static SQL_CLOSE: number;
  static SQL_DROP: number;
  static SQL_UNBIND: number;
  static SQL_RESET_PARAMS: number;
  static SQL_DESTROY: number;
  static FETCH_ARRAY: number;
  static FETCH_OBJECT: number;

  constructor() {}

  createConnection(
    cb: (err: DB2Error | null, conn?: ODBCConnection) => void
  ): void;
  createConnection() {}

  createConnectionSync(): ODBCConnection;
  createConnectionSync(): any {}
}

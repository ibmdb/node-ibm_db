export interface DB2Error extends Error {
  sqlcode: number;
  sqlstate: string;
  resultset?: Array<null | number | Buffer | string>;
}

# Typescript types

## Building the types

The types are written as standard Typescript code (.ts files).  After making any changes, be sure to
run `npm run build:types` to build the types and populate the dist/ folder before publishing
the npm module.

## New APIs

### ODBCResult

- **fetchN(count, options?, cb?)** — Fetch up to `count` rows at a time (block fetch). Returns an array of rows. Supports callback and Promise styles.
- **fetchNSync(count, options?)** — Synchronous version of `fetchN`.
- **bindFileToCol(colNum, filePath, fileOption?, cb?)** — Bind a LOB column to an output file for direct file I/O during fetch. Supports callback and Promise styles.
- **bindFileToColSync(colNum, filePath, fileOption?)** — Synchronous version of `bindFileToCol`.

### Chunked LOB Insert (SQLPutData)

The `Data` field of `SQLParamObject` now accepts:
- **`Buffer[]`** — Array of Buffers for chunked LOB insert (sync and async). Each Buffer is sent as a separate chunk via SQLPutData.
- **`Readable`** — A Readable stream for chunked LOB insert (async only). Stream data is collected into chunks before sending.

### File Option Constants (attributes.ts)

- `SQL_FILE_READ` (2) — Used internally for input file parameters.
- `SQL_FILE_CREATE` (8) — Create new file; fail if it already exists.
- `SQL_FILE_OVERWRITE` (16) — Create or overwrite the file (default for `bindFileToCol`).
- `SQL_FILE_APPEND` (32) — Append LOB data to an existing file.
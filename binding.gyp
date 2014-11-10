{
  'targets' : [
    {
      'target_name' : 'odbc_bindings',
      'sources' : [
        'src/odbc.cpp',
        'src/odbc_connection.cpp',
        'src/odbc_statement.cpp',
        'src/odbc_result.cpp',
        'src/dynodbc.cpp'
      ],
      'defines' : [
        'UNICODE',
        'ODBC64'
      ],
      'conditions' : [
        [ 'OS == "linux" and target_arch =="ia32"', {
          'libraries' : [
            '-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib32 ',
            '-ldb2'
          ],
          'include_dirs': [
            '$(IBM_DB_HOME)/include'
          ],
          'cflags' : [
            "-g "
          ],
        }],

        [ 'OS == "linux" and target_arch =="x64" ', {
          'libraries' : [
            '-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib64 ',
            '-ldb2o'
          ],
          'include_dirs': [
            '$(IBM_DB_HOME)/include'
          ],
          'cflags' : [
            "-g "
          ],
        }],
		
		[ 'OS=="win" and target_arch =="x64"', {
          'sources' : [
            'src/strptime.c',
            'src/odbc.cpp'
          ],
        'libraries': [
               '$(IBM_DB_HOME)/lib/db2cli.lib',
               '$(IBM_DB_HOME)/lib/db2api.lib',
        ],
		'include_dirs': [
            '$(IBM_DB_HOME)/include',
          ],
        }]
      ]
    }
  ]
}

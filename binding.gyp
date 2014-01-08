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
        'UNICODE'
      ],
      'conditions' : [
        [ 'OS == "linux"', {
          'libraries' : [ 
            '-ldb2' 
          ],
          'cflags' : [
            '-g'
          ],
	  'ldflags': [ "-Xlinker -rpath -Xlinker '$$ORIGIN/clidriver'" ]
        }]
      ]
    }
  ]
}

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
        [ 'OS == "linux" and target_arch =="ia32" and IBM_DB_HOME == ""', {
          'libraries' : [
            '-L<!(pwd)/installer/clidriver/lib -L<!(pwd)/installer/clidriver/lib/lib32 '
			'-ldb2'
          ],
          'include_dirs': [
            '<!(pwd)/installer/clidriver/include'
          ],
          'cflags' : [
            "-g "
          ],
        }],

		[ 'OS == "linux" and target_arch =="ia32" and IBM_DB_HOME != "" ', {
          'libraries' : [
            '-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib/lib32 '
			'-ldb2'
          ],
          'include_dirs': [
            '$(IBM_DB_HOME)/include'
          ],
          'cflags' : [
            "-g "
          ],
        }],
        
        [ 'OS == "linux" and target_arch =="x64" and IBM_DB_HOME == "" ', {
          'libraries' : [
            '-L<!(pwd)/installer/clidriver/lib -L<!(pwd)/installer/clidriver/lib/lib64', 
	     '-ldb2'
          ],
          'include_dirs': [
            '<!(pwd)/installer/clidriver/include'
           ],
          'cflags' : [
            "-g "
          ],
        }],
       
        [ 'OS == "linux" and target_arch =="x64" and IBM_DB_HOME != "" ', {
          'libraries' : [
            '-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib/lib64', 
	     '-ldb2'
          ],
          'include_dirs': [
            '$(IBM_DB_HOME)/include'
           ],
          'cflags' : [
            "-g "
          ],
        }],
		[ 'OS=="win" and target_arch =="ia32"', {
          'sources' : [
            'src/strptime.c',
            'src/odbc.cpp'
          ],
        'libraries': [
               '$(IBM_DB_HOME)/lib/db2cli.lib',
			   '$(IBM_DB_HOME)/db2api.lib',
			   '$(IBM_DB_HOME)/lib/Win32/db2cli.lib',
			   '$(IBM_DB_HOME)/Win32/db2api.lib',
        ],
		'include_dirs': [
            '$(IBM_DB_HOME)/include'
          ],
        }],
		
		[ 'OS=="win" and target_arch =="x64"', {
          'sources' : [
            'src/strptime.c',
            'src/odbc.cpp'
          ],
        'libraries': [
			   '<!(pwd)/installer/clidriver/lib/db2cli.lib',
			   '<!(pwd)/installer/clidriver/db2api.lib'
        ],
		'include_dirs': [
			'<!(pwd)/installer/clidriver/include'
          ],
        }]
      ]
    }
  ]
}
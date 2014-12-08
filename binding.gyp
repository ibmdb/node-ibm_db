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
	"variables": {
		# Set the linker location, no extra linking needed, just link backwards one directory
		"ORIGIN_LIB_PATH%": "$$ORIGIN/../../installer/clidriver/lib",
		"DS_DRIVER_INCLUDE_PATH%": "installer/clidriver/include",
		"DS_DRIVER_LIB_PATH%": "../installer/clidriver/lib",
	},
	'conditions' : [
        [ 'OS == "linux" and target_arch =="ia32" and IBM_DB_HOME == "" ', {
			'ldflags' : [
            	"-Wl,-rpath,'<(ORIGIN_LIB_PATH)' "
			],	
			'libraries' : [
				'-L<(DS_DRIVER_LIB_PATH)', 
				'-ldb2'
			],
			'include_dirs': [
				'<(DS_DRIVER_INCLUDE_PATH)'
			],
			'cflags' : [
				"-g "
			],
        }],
		
		[ 'OS == "linux" and target_arch =="ia32" and IBM_DB_HOME != "" ', {
          'libraries' : [
            '-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib/lib32 '
			'-ldb2',
			"-Wl,-rpath $(IBM_DB_HOME)/lib/lib32;$(IBM_DB_HOME)/lib"
          ],
          'include_dirs': [
            '$(IBM_DB_HOME)/include'
          ],
          'cflags' : [
            "-g "
          ],
        }],
        
        [ 'OS == "linux" and target_arch =="x64" and IBM_DB_HOME == "" ', {
		  'ldflags' : [
            	"-Wl,-rpath,'<(ORIGIN_LIB_PATH)' "
          ],	
          'libraries' : [
            	'-L<(DS_DRIVER_LIB_PATH)', 
				'-ldb2'
          ],	
          'include_dirs': [
            '<(DS_DRIVER_INCLUDE_PATH)'
           ],
          'cflags' : [
            "-g "
          ],
        }],       
        [ 'OS == "linux" and target_arch =="x64" and IBM_DB_HOME != "" ', {
          'libraries' : [
            '-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib/lib64', 
			'-ldb2',
			"-Wl,-rpath=$(IBM_DB_HOME)/lib"
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
               '$(IBM_DB_HOME)/lib/Win32/db2cli.lib',
			   '$(IBM_DB_HOME)/lib/Win32/db2api.lib',
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
			   '$(IBM_DB_HOME)/lib/db2cli.lib',
               '$(IBM_DB_HOME)/lib/db2api.lib'
        ],
		'include_dirs': [
            '$(IBM_DB_HOME)/include',
          ],
        }]
      ]
    }
  ]
}
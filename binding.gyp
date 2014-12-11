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
	},
	'conditions' : [
        [ 'OS == "linux" and target_arch =="ia32" ', {
		  'conditions' : [
			[ 'IS_DOWNLOADED == "true" ', {
				'ldflags' : [
					"-Wl,-R,'<(ORIGIN_LIB_PATH)' "
				],
			}]
		  ],	
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
		  'conditions' : [
			[ 'IS_DOWNLOADED == "true" ', {
				'ldflags' : [
					"-Wl,-R,'<(ORIGIN_LIB_PATH)' "
				],
			}]
		  ],		
          'libraries' : [
            	'-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib64 ', 
				'-ldb2'
          ],	
          'include_dirs': [
				'$(IBM_DB_HOME)/include'
           ],
          'cflags' : [
            "-g "
          ],
        }],
		
        [ 'OS == "mac" and target_arch =="x64" ', {
          'libraries' : [
            '-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib64 ',
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

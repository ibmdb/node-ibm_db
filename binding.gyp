{
  'targets' : [
    {
      'target_name' : 'odbc_bindings',
      'sources' : [
        'src/odbc.cpp',
        'src/odbc_connection.cpp',
        'src/odbc_statement.cpp',
        'src/odbc_result.cpp',
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")"
      ],
      'conditions' : [
        [ 'OS != "zos"',
          { 'defines' : [ 'UNICODE'], }
        ]
      ],
      "variables": {
        # Set the linker location, no extra linking needed, just link backwards one directory
        "ORIGIN_LIB_PATH%": "$$ORIGIN/../../installer/clidriver/lib",
      },
      'conditions' : [
        [ '(OS == "linux" and (target_arch =="ia32" or target_arch == "s390" or target_arch == "ppc32")) or (OS == "aix" and target_arch == "ppc")',
          { 'conditions' : [
              [ 'IS_DOWNLOADED == "true" ',
                { 'ldflags' : [ "-Wl,-R,'<(ORIGIN_LIB_PATH)' " ] }
              ]
            ],  
            'libraries' : [ '-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib32 ', '-ldb2' ],
            'include_dirs': ['$(IBM_DB_HOME)/include'],
            'cflags' : ['-g'],
          }],  

        [ '(OS == "linux" or OS == "aix") and (target_arch =="x64"  or target_arch == "s390x" or target_arch == "ppc64")',
          { 'conditions' : [
              [ 'IS_DOWNLOADED == "true" ',
                { 'ldflags' : ["-Wl,-R,'<(ORIGIN_LIB_PATH)' " ], }
              ]
            ],    
            'libraries' : ['-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib64 ','-ldb2' ],
            'include_dirs': ['$(IBM_DB_HOME)/include'],
            'cflags' : ['-g'],
          }],
        [ 'OS == "zos" ',
          { 'libraries' : ['dsnao64c.x'],
            'include_dirs': ['build/include'],
            'cflags' : ['-g']
          }],
        [ 'OS == "mac" and target_arch =="x64" ',
          { 'xcode_settings': {'GCC_ENABLE_CPP_EXCEPTIONS': 'YES' },
            'libraries' : ['-L$(IBM_DB_HOME)/lib ', '-ldb2'],
            'include_dirs': ['$(IBM_DB_HOME)/include'],
            'cflags' : ['-g']
          }],

        [ 'OS=="win" and target_arch =="ia32"',
          { 'sources' : ['src/strptime.c', 'src/odbc.cpp'],
            'libraries': [
               '$(IBM_DB_HOME)/lib/Win32/db2cli.lib',
               '$(IBM_DB_HOME)/lib/Win32/db2api.lib'],
            'include_dirs': ['$(IBM_DB_HOME)/include']
          }],

        [ 'OS=="win" and target_arch =="x64"',
          { 'sources' : ['src/strptime.c', 'src/odbc.cpp'],
            'libraries': [
              '$(IBM_DB_HOME)/lib/db2cli64.lib',
              '$(IBM_DB_HOME)/lib/db2app64.lib'],
            'include_dirs': ['$(IBM_DB_HOME)/include'],
          }],

        [ 'OS != "linux" and OS!="win" and OS!="darwin" and target_arch =="ia32" ',
          { 'conditions' : [
              [ 'IS_DOWNLOADED == "true" ',
                {'ldflags' : ["-Wl,-R,'<(ORIGIN_LIB_PATH)' "]}]
            ],
            'libraries' : ['-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib32 ', '-ldb2'],
            'include_dirs': ['$(IBM_DB_HOME)/include'],
            'cflags' : ['-g']
          }], 

        [ 'OS != "linux" and OS != "win" and OS != "mac" and target_arch == "x64" ',
          { 'conditions' : [
              ['IS_DOWNLOADED == "true" ', {'ldflags' : ["-Wl,-R,'<(ORIGIN_LIB_PATH)' "]}]
            ],    
            'libraries' : ['-L$(IBM_DB_HOME)/lib -L$(IBM_DB_HOME)/lib64 ', '-ldb2'],
            'include_dirs': ['$(IBM_DB_HOME)/include'],
            'cflags' : ['-g']
          }]
      ]
    }
  ]
}

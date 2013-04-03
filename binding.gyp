{
  'targets' : [
    {
      'target_name' : 'odbc_bindings',
      'sources' : [ 
        'src/odbc.cpp',
        'src/odbc_connection.cpp',
        'src/odbc_statement.cpp',
        'src/odbc_result.cpp'
      ],
      'defines' : [
        
      ],
      'conditions' : [
        [ 'OS == "linux"', {
          'libraries' : [ 
            '-lodbc' 
          ]
        }],
        [ 'OS == "mac"', {
          'libraries' : [ 
            '-lodbc' 
          ]
        }],
        [ 'OS=="win"', {
          'sources' : [
            'src/strptime.c',
            'src/odbc.cpp'
          ],
          'libraries' : [ 
            '-lodbccp32.lib' 
          ]
        }]
      ]
    }
  ]
}

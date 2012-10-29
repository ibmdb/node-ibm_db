{
	'targets' : [
		{
			'target_name' : 'odbc_bindings',
			'sources' : [ 
				'src/odbc.cpp',
                'src/odbc_result.cpp'
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

{
	'targets' : [
		{
			'target_name' : 'odbc_bindings',
			'sources' : [ 
				'src/odbc.cpp',
				'src/odbc_result.cpp'
			],
			'defines' : [
			],
			'conditions' : [
				[ 'OS == "linux"', {
					'libraries' : [ 
						'-lodbc' 
					],
					'cflags' : [
						'-g'
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

{
	'targets' : [
		{
			'target_name' : 'odbc_bindings',
			'sources' : [ 
				'src/Database.cpp'
			],
			'conditions' : [
				[ 'OS == "linux"', {
					'libraries' : [ 
						'-lodbc' 
					]
				}],
				[ 'OS=="win"', {
					'sources' : [
						'src/strptime.c',
						'src/Database.cpp'
					],
					'libraries' : [ 
						'-lodbccp32.lib' 
					]
				}]
			]
		}
	]
}

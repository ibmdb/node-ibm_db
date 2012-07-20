{
	'targets' : [
		{
			'target_name' : 'odbc_bindings',
			'sources' : [ 
				'src/Database.cpp' 
			],
			'libraries' : [ 
				'-lodbc' 
			],
			'conditions' : [
				[ 'OS == "linux"', {
					
				}],
				[ 'OS=="win"', {
					
				}]
			]
		}
	]
}

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
			'include_dirs' : [
				'/usr/local/lib', 
				'/opt/local/lib',
				'/usr/include'
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

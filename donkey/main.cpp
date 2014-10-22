#include "donkey.hpp"
#include <cstdio>

#include "modules/io/io_module.hpp"
#include "modules/containers/containers_module.hpp"


int main(int argc, char* argv[]){
	if(argc < 2 || argc > 3){
		printf("usage %s <module> [directory (current is default)]\n", argv[0]);
		return 1;
	}

	const char* root = (argc == 3 ? argv[2] : ".");

	donkey::compiler c(root);
	
	c.add_module_loader("io", &donkey::load_io_module);
	c.add_module_loader("containers", &donkey::load_containers_module);
	
	if(!c.compile_module(argv[1])){
		printf("cannot load module %s/%s.dky\n", root, argv[1]);
		return 2;
	}
	
	return 0;
}


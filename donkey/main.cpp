#include "donkey.hpp"
#include <cstdio>

#include "modules/io/io_module.hpp"
#include "modules/containers/containers_module.hpp"
#include "modules/gui/gui_module.hpp"
#include "modules/functional/functional_module.hpp"


int main(int argc, char* argv[]){
	setlocale(LC_ALL, "");

	if(argc < 2 || argc > 3){
		printf("usage %s <module> [directory (current is default)]\n", argv[0]);
		return 1;
	}

	const char* root = (argc == 3 ? argv[2] : ".");

	donkey::compiler c(root);
	
	c.add_module_loader("io", &donkey::load_io_module);
	c.add_module_loader("containers", &donkey::load_containers_module);
	//c.add_module_loader("gui", &donkey::load_gui_module);
	c.add_module_loader("functional", &donkey::load_functional_module);
	
	if(!c.load_module(argv[1])){
		printf("cannot load module %s/%s.dky\n", root, argv[1]);
		return 2;
	}
	
	return 0;
}


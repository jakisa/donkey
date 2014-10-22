#include "donkey.hpp"
#include <cstdio>
#include <cstring>

#include "modules/io/io_module.hpp"
#include "modules/containers/containers_module.hpp"


int main(){
	
	donkey::compiler c("../donkey/dky/");
	
	c.add_module_loader("io", &donkey::load_io_module);
	c.add_module_loader("containers", &donkey::load_containers_module);
	
	c.compile_module("test");
	
	return 0;
}


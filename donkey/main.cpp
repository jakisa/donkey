#include "donkey.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

#include <iostream>

#include "modules/io/io_module.hpp"

int main(){
	

	donkey::compiler c("../donkey/dky/");
	
	c.add_module_loader("io", &donkey::load_io_module);
	
	c.compile_module("test");
	
	return 0;
}


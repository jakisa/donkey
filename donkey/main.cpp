#include "donkey.hpp"
#include <cstdio>
#include <cstring>
#include <vector>


int main(){
	donkey::compiler c("../donkey/");
	
	c.compile_module("test");
	
	c.execute_module("test");
	
	return 0;
}


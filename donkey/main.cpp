#include "donkey.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

#include "runtime_context.hpp"
#include "errors.hpp"
#include "helpers.hpp"


#include <iostream>


int main(){
	int x = sizeof(donkey::variable);


	donkey::compiler c("../donkey/");
	
	c.compile_module("test");
	
	c.execute_module("test");
	return 0;
}


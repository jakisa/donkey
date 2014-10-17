#include "donkey.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

#include <iostream>


int main(){
	

	donkey::compiler c("../donkey/dky/");
	
	c.compile_module("test");
	
	return 0;
}


#include "donkey.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

//#include "stack.hpp"



//#include <iostream>

int main(){
	/*
	donkey::stack s;
	
	s.add_size(1000);
	
	for(int i = 0; i < 1000; ++i){
		s[i] = donkey::variable(i);
	}
	
	auto v = s[255];
	
	for(size_t i = 0; i < 1000; ++i){
		if(s[i].as_number() != i){
			std::cout << "ukurcu\n" << std::endl;
		}
	}*/

	donkey::compiler c("../donkey/");
	
	c.compile_module("test");
	
	c.execute_module("test");
	
	return 0;
}


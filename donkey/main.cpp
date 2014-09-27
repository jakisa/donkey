#include "donkey.hpp"

#include <cstdio>
#include <vector>

#include "expression_builder.hpp"

#include <cstring>

using namespace donkey;

int main(){
	//compiler comp("../donkey/");

	//comp.compile_module("test");

	//testRPN("a+b");
	
	char buff[1024];
	do{
		fgets(buff, 1024, stdin);
		try{
			//testRPN(buff);
		}catch(donkey::exception& e){
			printf("EXCEPTION: %s\n", e.what());
		}
	}while(strcmp(buff, "\n"));

	return 0;
}

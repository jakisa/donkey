#include "donkey.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

#include "runtime_context.hpp"
#include "errors.hpp"
#include "helpers.hpp"

#include "native_function.hpp"


#include <iostream>

int main(){
	donkey::compiler c("../donkey/");
	
	c.compile_module("test");
	
	c.execute_module("test");
	
	//donkey::detail::param_converter<std::function<int()> >::from_native(&test);
	
	//donkey::create_native_function(S());
	
	return 0;
}


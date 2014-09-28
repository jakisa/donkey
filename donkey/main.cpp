#include "donkey.hpp"

#include <cstdio>
#include <vector>

#include "expression_builder.hpp"

#include <cstring>
#include <cmath>

#include <functional>

using namespace donkey;

class test_lookup: public identifier_lookup{
public:
	virtual identifier_ptr get_identifier(std::string name) const override{
		if(name == "a"){
			return identifier_ptr(new global_variable_identifier(0));
		}
		if(name == "b"){
			return identifier_ptr(new global_variable_identifier(1));
		}
		if(name == "c"){
			return identifier_ptr(new global_variable_identifier(2));
		}
		if(name == "d"){
			return identifier_ptr(new global_variable_identifier(3));
		}
		if(name == "e"){
			return identifier_ptr(new global_variable_identifier(4));
		}
		if(name == "sin"){
			return identifier_ptr(new function_identifier(0));
		}
		return identifier_ptr();
	}
	
	virtual bool is_allowed(std::string) const override{
		return true;
	}
};


class test_code: public code_container{
private:
	static variable_ptr sin(runtime_context& ctx, size_t prms){
		//return new variable_ptr(;
		if(prms == 0){
			return variable_ptr();
		}
		double x = std::static_pointer_cast<number_variable>(ctx.stack[ctx.stack.size()-prms])->value();
		return variable_ptr(new number_variable(::sin(x)));
	}
public:
	virtual variable_ptr call_function_by_address(code_address address, runtime_context& ctx, size_t prms) const{
		if(address == 0){
			return sin(ctx, prms);
		}else{
			return variable_ptr();
		}
	}
};

int main(){
	
	test_code code;
	
	test_lookup lookup;
	
	
	
	runtime_context context(&code);
	context.global.resize(5);
	
	char buff[1024];
	do{
		fgets(buff, 1024, stdin);
		try{
			//testRPN(buff);
			tokenizer parser(buff, buff + strlen(buff));
			expression_ptr exp = build_expression(lookup, parser, true);
			exp->as_void(context);
			
			printf("a=%s, b=%s, c=%s, d=%s, e=%s\n",
				variable::to_string(context.global[0]).c_str(), 
				variable::to_string(context.global[1]).c_str(),
				variable::to_string(context.global[2]).c_str(),
				variable::to_string(context.global[3]).c_str(),
				variable::to_string(context.global[4]).c_str()
			);
		}catch(donkey::exception& e){
			printf("EXCEPTION: %s\n", e.what());
		}
	}while(strcmp(buff, "\n"));

	return 0;
}

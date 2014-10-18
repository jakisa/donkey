#include "variable_compiler.hpp"
#include "expression_builder.hpp"
#include "compiler_helpers.hpp"

namespace donkey{

expression_ptr compile_variable(scope& target, tokenizer& parser, bool is_public){
	if(target.is_switch()){
		unexpected_error(*parser);
	}

	expression_ptr ret;
	
	++parser;
	
	while(parser && *parser != ";"){
		std::string name = parse_allowed_name(target, parser);
		
		expression_ptr e;
		
		if(target.is_global()){
			e = build_global_variable_expression(
				target.get_module_index(),
				target.get_next_var_index()
			);
		}else{
			e = build_local_variable_expression(target.get_next_var_index());
		}
		
		
		if(*parser == "="){
			++parser;
			ret = build_binary_expression(
				oper::assignment,
				e,
				build_expression(target, parser, false, true)
			);
			target.add_statement(expression_statement(ret));
		}else{
			ret.reset();
		}
		
		target.add_variable(name, is_public);
		
		if(*parser == ","){
			++parser;
		}
	}
	++parser;
	
	return ret;
}

}//donkey

#include "variable_compiler.hpp"
#include "expression_builder.hpp"
#include "compiler_helpers.hpp"

namespace donkey{

expression_ptr compile_variable(scope& target, tokenizer& parser){
	if(target.is_switch()){
		unexpected_error(parser.get_line_number(), *parser);
	}

	expression_ptr ret;
	
	++parser;
	
	while(parser && *parser != ";"){
		std::string name = parse_allowed_name(target, parser);
		target.add_variable(name);
		
		identifier_ptr id = target.get_identifier(name);
		
		int idx = (target.is_global() ?
				   static_cast<global_variable_identifier&>(*id).get_index() :
				   static_cast<local_variable_identifier&>(*id).get_index());
			
		
		if(*parser == "="){
			++parser;
			ret = build_binary_expression(
				oper::assignment,
				target.is_global() ? build_global_variable_expression(idx) : build_local_variable_expression(idx),
				build_expression(target, parser, false, true),
				parser.get_line_number()
			);
			target.add_statement(expression_statement(ret));
		}else{
			ret.reset();
		}
		if(*parser == ","){
			++parser;
		}
	}
	++parser;
	
	return ret;
}

}//donkey

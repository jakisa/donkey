#include "expression_compiler.hpp"
#include "expression_builder.hpp"
#include "compiler.hpp"

namespace donkey{
	

void compile_expression_statement(scope& target, tokenizer& parser){
	if(target.is_class()){
		unexpected_error(parser.get_line_number(), *parser);
	}
	target.add_statement(expression_statement(build_expression(target, parser, true)));
	parse(";", parser);
}


}//donkey
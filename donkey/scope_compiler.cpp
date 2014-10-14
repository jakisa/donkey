#include "scope_compiler.hpp"
#include "statement_compiler.hpp"

namespace donkey{

void compile_local_scope(scope& target, tokenizer& parser){
	if(target.is_class()){
		unexpected_error(parser.get_line_number(), *parser);
	}
	scope s(&target);
	for(++parser; parser;){
		if(*parser == "}"){
			++parser;
			target.add_statement(s.get_block());
			return;
		}
		compile_statement(s, parser);
	}
	syntax_error(parser.get_line_number(), "'}' expected");
}

}//donkey

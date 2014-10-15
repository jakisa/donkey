#include "jump_compilers.hpp"
#include "compiler.hpp"
#include "expression_builder.hpp"

namespace donkey {


void compile_break(scope& target, tokenizer& parser){
	if(!target.can_break()){
		semantic_error("unexpected break");
	}
	++parser;
	target.add_statement(break_statement);
	parse(";", parser);
}

void compile_continue(scope& target, tokenizer& parser){
	if(!target.can_break()){
		semantic_error("unexpected continue");
	}
	++parser;
	target.add_statement(continue_statement);
	parse(";", parser);
}

void compile_return(scope& target, tokenizer& parser){
	if(!target.in_function()){
		semantic_error("unexpected return");
	}
	++parser;
	target.add_statement(return_statement(build_expression(target, parser, true)));
	parse(";", parser);
}


}//donkey
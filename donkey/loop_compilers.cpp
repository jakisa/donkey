#include "compiler.hpp"
#include "loop_compilers.hpp"
#include "scope.hpp"
#include "tokenizer.hpp"
#include "statement_compiler.hpp"
#include "expression_builder.hpp"

namespace donkey{

void compile_cpp_for(scope& target, tokenizer& parser){
	scope outer(&target);
	
	compile_statement(outer, parser);
	
	expression_ptr e2 = build_expression(outer, parser, false);
	parse(";", parser);
	expression_ptr e3 = build_expression(outer, parser, true);
	
	parse(")", parser);
	
	scope s(&outer, false, false, true, true);
	compile_statement(s, parser);
	
	outer.add_statement(for_statement(build_null_expression(), e2, e3, s.get_block()));
	
	target.add_statement(outer.get_block());
}

void compile_c_for(scope& target, tokenizer& parser){
	expression_ptr e1 = build_expression(target, parser, true);
	parse(";", parser);
	
	expression_ptr e2 = build_expression(target, parser, false);
	parse(";", parser);
	expression_ptr e3 = build_expression(target, parser, true);
	
	parse(")", parser);
	
	scope s(&target, false, false, true, true);
	compile_statement(s, parser);
	
	target.add_statement(for_statement(e1, e2, e3, s.get_block()));
}

void compile_for(scope& target, tokenizer& parser){
	if(target.is_class()){
		unexpected_error(parser.get_line_number(), *parser);
	}
	++parser;
	
	parse("(", parser);
	
	if(*parser == "var"){
		compile_cpp_for(target, parser);
	}else{
		compile_c_for(target, parser);
	}
}

void compile_while(scope& target, tokenizer& parser){
	if(target.is_class()){
		unexpected_error(parser.get_line_number(), *parser);
	}
	++parser;
	parse("(", parser);
	expression_ptr e = build_expression(target, parser, false);
	parse(")", parser);
	scope s(&target, false, false, true, true);
	compile_statement(s, parser);
	target.add_statement(while_statement(e, s.get_block()));
}

void compile_do(scope& target, tokenizer& parser){
	if(target.is_class()){
		unexpected_error(parser.get_line_number(), *parser);
	}
	++parser;
	scope s(&target, false, false, true, true);
	compile_statement(s, parser);
	parse("while", parser);
	parse("(", parser);
	expression_ptr e = build_expression(target, parser, false);
	parse(")", parser);
	parse(";", parser);
	target.add_statement(do_statement(e, s.get_block()));
}

}//donkey

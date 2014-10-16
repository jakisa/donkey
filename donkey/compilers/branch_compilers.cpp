#include "branch_compilers.hpp"
#include "compiler_helpers.hpp"
#include "expression_builder.hpp"
#include "statement_compiler.hpp"

namespace donkey{

void compile_if(scope& target, tokenizer& parser){
	if(target.is_class()){
		unexpected_error(*parser);
	}
	std::vector<expression_ptr> es;
	std::vector<statement> ss;
	
	do{
		++parser;
		parse("(", parser);
		es.push_back(build_expression(target, parser, false));
		parse(")", parser);
		scope s(&target);
		compile_statement(s, parser);
		ss.push_back(s.get_block());
	}while(*parser == "elif");
	
	if(*parser == "else"){
		++parser;
		scope s(&target);
		compile_statement(s, parser);
		ss.push_back(s.get_block());
	}else{
		ss.push_back(&empty_statement);
	}
	
	if(es.size() == 1){
		if(ss.size() == 1){
			target.add_statement(simple_if_statement(es.front(), std::move(ss.front())));
		}else{
			target.add_statement(if_else_statement(es.front(), std::move(ss.front()), std::move(ss.back())));
		}
	}else{
		target.add_statement(if_statement(std::move(es), std::move(ss)));
	}
}

void compile_switch(scope& target, tokenizer& parser){
	if(target.is_class()){
		unexpected_error(*parser);
	}
	std::unordered_map<number, size_t> cases;
	size_t dflt = 0;
	bool has_dflt = false;
	
	++parser;
	
	parse("(", parser);
	
	expression_ptr e = build_expression(target, parser, false);
	
	parse(")", parser);
	
	parse("{", parser);
	
	scope s(&target, false, true, true, false);
	
	while(parser){
		if(*parser == "}"){
			++parser;
			if(!has_dflt){
				dflt = s.get_number_of_statements();
			}
			target.add_statement(switch_statement(e, s.get_statements(), std::move(cases), dflt));
			return;
		}
		if(*parser == "case"){
			++parser;
			number d = parse_number(*parser);
			if(isnan(d)){
				unexpected_error(*parser);
			}
			
			if(cases.find(d) != cases.end()){
				semantic_error("duplicated case " + *parser);
			}
			++parser;
			parse(":", parser);
			cases[d] = s.get_number_of_statements();
		}else if(*parser == "default"){
			++parser;
			if(has_dflt){
				semantic_error("duplicated default");
			}
			parse(":", parser);
			dflt = s.get_number_of_statements();
			has_dflt = true;
		}
		if(*parser != "case" && *parser != "default"){
			compile_statement(s, parser);
		}			
	}
	syntax_error("'}' expected");
}
	
}//donkey;

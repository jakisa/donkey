#include "donkey.hpp"
#include "tokenizer.hpp"
#include "errors.hpp"
#include "variables.hpp"
#include "expressions.hpp"
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include "statements.hpp"
#include "compiletime_context.hpp"
#include "donkey_function.h"
#include "expression_builder.hpp"

namespace donkey{

class scope{
	scope* _parent;
public:
	scope():
		_parent(nullptr){
	}

	scope(scope* parent):
		_parent(parent){
	}


};

class module: public scope{
};

typedef std::function<void(scope&, tokenizer&)> compiler_function;

typedef std::unordered_map<std::string, compiler_function > compiler_map;

using namespace std::placeholders;


const char* keywords[] = {
	"break",
	"case",
	"continue",
	"default",
	"do",
	"elif",
	"else",
	"for",
	"function",
	"if",
	"null",
	"return",
	"switch",
	"var",
	"while",
};

inline bool str_less(const char* l, const char* r){
	return strcmp(l, r) < 0;
}

inline bool is_keyword(const std::string& name){
	return std::binary_search(keywords, keywords + sizeof(keywords)/sizeof(keywords[0]), name.c_str(), &str_less);
}

class compiler::priv{
private:
	std::string _root;
	compiler_map _statement_compilers;


	void check_allowed_name(const std::string& name, scope&, tokenizer& parser){
		if(is_keyword(name)){
			syntax_error(parser.get_line_number(), (name + "is keyword").c_str());
		}
		/*
		if(target.find_variable(name)){
			syntax_error(parser.get_line_number(), (name + "is already declared").c_str());
		}*/
	}

	//TODO:
	void compile_function(scope& , tokenizer& ){

	}


	//TODO:
	void compile_variable_fun(scope& , tokenizer& ){

	}

	//TODO:
	void compile_variable_string(scope& , tokenizer& ){

	}

	//TODO:
	void compile_variable_number(scope& target, tokenizer& parser){
		++parser;

		if(parser.get_token_type() != tokenizer::tt_word){
			syntax_error(parser.get_line_number(), "identifier expected");
		}

		auto name = *parser++;

		check_allowed_name(name, target, parser);

		if(*parser != ";"){
			syntax_error(parser.get_line_number(), "; expected");
		}
	}

	void compile_variable(scope&, tokenizer&){
	}

	//TODO:
	void compile_class(scope& , tokenizer& ){

	}

	//TODO:
	void compile_interface(scope& , tokenizer& ){

	}

	//TODO:
	void compile_for(scope& , tokenizer& ){

	}

	//TODO:
	void compile_while(scope& , tokenizer& ){

	}

	//TODO:
	void compile_do(scope& , tokenizer& ){

	}

	//TODO:
	void compile_if(scope& , tokenizer& ){

	}

	//TODO:
	void compile_switch(scope& , tokenizer& ){

	}

	//TODO:
	void compile_return(scope& , tokenizer& ){

	}

	//TODO:
	void compile_pin(scope& , tokenizer& ){

	}

	//TODO:
	void compile_expression_statement(scope& , tokenizer&){

	}

	void compile_statement(scope& target, tokenizer& parser){
		auto it = _statement_compilers.find(*parser);
		if(it == _statement_compilers.end()){
			compile_expression_statement(target, parser);
		}else{
			it->second(target, parser);
		}
	}

	void compile_scope(scope& target, tokenizer& parser){
		for(++parser; parser;){
			if(*parser == "}"){
				++parser;
				return;
			}else{
				compile_statement(target, parser);
			}
		}
		syntax_error(parser.get_line_number(), "'}' expected");
	}
#define ADD_STATEMENT_COMPILER(n, f) _statement_compilers.emplace(n, std::bind(&compiler::priv:: f, this, _1, _2))

	void populate_compiler_maps(){
		ADD_STATEMENT_COMPILER("fun", compile_function);
		ADD_STATEMENT_COMPILER("var", compile_variable);
		ADD_STATEMENT_COMPILER("class", compile_class);
		ADD_STATEMENT_COMPILER("interface", compile_interface);
		ADD_STATEMENT_COMPILER("for", compile_for);
		ADD_STATEMENT_COMPILER("while", compile_while);
		ADD_STATEMENT_COMPILER("do", compile_do);
		ADD_STATEMENT_COMPILER("if", compile_if);
		ADD_STATEMENT_COMPILER("switch", compile_switch);
		ADD_STATEMENT_COMPILER("return", compile_return);
		ADD_STATEMENT_COMPILER("pin", compile_pin);
	}

#undef ADD_STATEMENT_COMPILER

public:
	priv(const char* root):
		_root(root){
		populate_compiler_maps();
	}

	bool compile_module(const char* module_name){
		FILE* fp = fopen((_root + module_name + ".dky").c_str(), "rb");

		if(!fp){
			return false;
		}

		fseek(fp, 0, SEEK_END);

		int len = ftell(fp);

		fseek(fp, 0, SEEK_SET);

		std::vector<char> v(len+2);
		v.front() = '{';
		v.back() = '}';

		fread(&v[1], 1, len, fp);

		fclose(fp);

		module m;
		try{
			tokenizer parser (&v[0], &v[0] + len);
			compile_scope(m, parser);
			return true;
		}catch(const exception&){
			return false;
		}
	}
};

compiler::compiler(const char* root):
	_private(new priv(root)){
}

bool compiler::compile_module(const char* module_name){
	return _private->compile_module(module_name);
}

compiler::~compiler(){
	delete _private;
}

}//namespace donkey

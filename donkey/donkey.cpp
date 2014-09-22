#include "donkey.hpp"
#include "tokenizer.hpp"
#include "errors.hpp"
#include "types.hpp"
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <functional>

namespace donkey{

class scope{
	typedef std::unique_ptr<type> type_ptr;
	scope* _parent;
	std::unordered_map<std::string, type_ptr> _types;
public:
	scope():
		_parent(nullptr){
		_types["void"] = type_ptr(new type_void());
		_types["number"] = type_ptr(new type_simple(simple_type::number));
		_types["string"] = type_ptr(new type_simple(simple_type::string));
	}

	scope(scope* parent):
		_parent(parent){
	}

	type* find_type(const std::string& name) const{
		auto it = _types.find(name);
		if(it != _types.end()){
			return it->second.get();
		}
		return _parent ? _parent->find_type(name) : nullptr;
	}

	type* add_type(const std::string& name){
		return _types[name].get();
	}

	type* add_global_type(const std::string& name){
		return _parent ? _parent->add_global_type(name) : _types[name].get();
	}
};

class module: public scope{
};

typedef std::function<void(scope&, tokenizer&)> compiler_function;

typedef std::unordered_map<std::string, compiler_function > compiler_map;

using namespace std::placeholders;


const char* keywords[] = {
	"array",
	"break",
	"case",
	"continue",
	"class",
	"default",
	"do",
	"elif",
	"else",
	"for",
	"fun",
	"if",
	"interface",
	"map",
	"number",
	"return",
	"string",
	"switch",
	"var",
	"void",
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


	void check_allowed_name(const std::string& name, scope& target, tokenizer& parser){
		if(is_keyword(name) || target.find_type(name)){
			syntax_error(parser.get_line_number(), "name is already keyword or type name");
		}
	}

	//TODO:
	void compile_function(scope& , tokenizer& ){

	}

	ref_type fetch_ref_type(tokenizer& tokenizer){
		if(*tokenizer == "ref"){
			++tokenizer;
			return ref_type::ref;
		}else if(*tokenizer == "cref"){
			++tokenizer;
			return ref_type::cref;
		}
		return ref_type::value;
	}

	//TODO:
	void compile_variable_fun(ref_type, scope& , tokenizer& ){

	}

	//TODO:
	void compile_variable_string(ref_type, scope& , tokenizer& ){

	}

	//TODO:
	void compile_variable_number(ref_type, scope& target, tokenizer& parser){
		++parser;

		if(parser.get_token_type() != tokenizer::tt_word){
			syntax_error(parser.get_line_number(), "identifier expected");
		}

		auto name = *parser++;

		check_allowed_name(name, target, parser);

		if(*parser == "="){

		}
	}

	//TODO:
	void compile_variable_array(ref_type, scope& , tokenizer& ){

	}

	//TODO:
	void compile_variable_map(ref_type, scope& , tokenizer&){

	}

	//TODO:
	void compile_variable_class(ref_type, type*, scope& , tokenizer& ){

	}

	void compile_variable(scope& target, tokenizer& parser){
		++parser;

		ref_type rt = fetch_ref_type(parser);

		if(parser.get_token_type() != tokenizer::tt_word){
			syntax_error(parser.get_line_number(), "type expected");
		}

		if(*parser == "fun"){
			compile_variable_fun(rt, target, parser);
		}else if(*parser == "string"){
			compile_variable_string(rt, target, parser);
		}else if(*parser == "number"){
			compile_variable_number(rt, target, parser);
		}else if(*parser == "array"){
			compile_variable_array(rt, target, parser);
		}else if(*parser == "map"){
			compile_variable_map(rt, target, parser);
		}else{
			type* tp = target.find_type(*parser);
			if(tp == nullptr){
				syntax_error(parser.get_line_number(), "unknown type");
			}else{
				compile_variable_class(rt, tp, target, parser);
			}
		}
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

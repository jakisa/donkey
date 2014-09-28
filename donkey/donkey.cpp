#include "donkey.hpp"
#include "tokenizer.hpp"
#include "identifiers.hpp"
#include "runtime_context.hpp"
#include "errors.hpp"
#include "statements.hpp"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>

namespace donkey{

class module{
	module(const module&) = delete;
	void operator=(const module&) = delete;
private:
	statement _s;
	size_t _globals_count;
public:
	module(statement s, int globals_count):
		_s(std::move(s)),
		_globals_count(globals_count){
	}
	void load(runtime_context& ctx){
		ctx.global = std::vector<variable_ptr>(_globals_count);
		_s(ctx);
	}
};

typedef std::shared_ptr<module> module_ptr;

class scope: public identifier_lookup{
private:
	std::unordered_map<std::string, identifier_ptr> _variables;
	std::vector<statement> _statements;
	scope* _parent;
	int _var_index;
	const int _initial_index;
public:
	scope(scope* parent, size_t var_index):
		_parent(parent),
		_var_index(var_index),
		_initial_index(var_index){
	}
	
	scope():
		scope(nullptr, 0){
	}

	virtual identifier_ptr get_identifier(std::string name) const override{
		auto it = _variables.find(name);
		return it != _variables.end() ? it->second : _parent ? _parent->get_identifier(name) : identifier_ptr();
	}
	
	bool is_global() const{
		return !_parent;
	}
	
	bool add_variable(std::string name){
		if(_variables.find(name) == _variables.end()){
			if(is_global()){
				_variables[name].reset(new global_variable_identifier(_var_index++));
			}else{
				_variables[name].reset(new local_variable_identifier(_var_index++));
			}
			return true;
		}
		return false;
	}
	
	void add_statement(statement s){
		_statements.push_back(std::move(s));
	}
	
	statement get_block(){
		if(is_global()){
			return statement(std::move(block_statement(_statements, 0)));
		}else{
			return statement(std::move(block_statement(_statements, _var_index - _initial_index)));
		}
	}
	
	size_t get_number_of_variables() const{
		return _var_index - _initial_index;
	}
};

class global_scope: public scope{
private:
	std::unordered_map<std::string, function_identifier_ptr> _functions;
public:
	bool has_function(std::string name) const{
		auto it = _functions.find(name);
		return it != _functions.end() &&  !it->second->is_empty();
	}
	
	void declare_function(std::string name){
		_functions[name].reset(new function_identifier());
	}
	
	void define_function(std::string name, function f){
		auto& id = _functions[name];
		
		if(id){
			id->set_function(f);
		}else{
			id.reset(new function_identifier(f));
		}
	}
	
	bool has_undefined_functions() const{
		for(const auto& p: _functions){
			if(!p.second->is_empty()){
				return true;
			}
		}
		return false;
	}
	
	virtual identifier_ptr get_identifier(std::string name) const override{
		auto it = _functions.find(name);
		return it != _functions.end() ? it->second : scope::get_identifier(name);
	}
};

typedef std::function<void(scope&, tokenizer&)> compiler_function;
typedef std::function<void(global_scope&, tokenizer&)> global_compiler_function;

typedef std::unordered_map<std::string, compiler_function > compiler_map;
typedef std::unordered_map<std::string, global_compiler_function > global_compiler_map;

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
	global_compiler_map _global_compilers;
	compiler_map _local_compilers;
	
	std::unordered_map<std::string, module_ptr> _modules;


	void check_allowed_name(const std::string& name, identifier_lookup& lookup, tokenizer& parser){
		if(is_keyword(name)){
			syntax_error(parser.get_line_number(), (name + "is keyword").c_str());
		}
		
		if(lookup.get_identifier(name)){
			syntax_error(parser.get_line_number(), (name + "is already declared").c_str());
		}
	}

	//TODO:
	void compile_function(global_scope& , tokenizer& ){
	}


	//TODO:
	void compile_variable_fun(scope& , tokenizer& ){

	}

	//TODO:
	void compile_variable_string(scope& , tokenizer& ){

	}

	//TODO:
	void compile_variable_number(scope&, tokenizer&){
	}
	
	//TODO:
	void compile_variable(scope&, tokenizer&){
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

	template<typename TARGET, class COMPILER_MAP>
	void compile_statement(TARGET& target, tokenizer& parser, const COMPILER_MAP& compilers){
		auto it = compilers.find(*parser);
		if(it == compilers.end()){
			if(is_keyword(*parser)){
				unexpected_error(parser.get_line_number(), *parser);
			}
			compile_expression_statement(target, parser);
		}else{
			it->second(target, parser);
		}
	}

	void compile_local_scope(scope& target, tokenizer& parser){
		for(++parser; parser;){
			if(*parser == "}"){
				++parser;
				return;
			}
			compile_statement(target, parser, _local_compilers);
		}
		syntax_error(parser.get_line_number(), "'}' expected");
	}
	
	module_ptr compile_module(tokenizer& parser){
		global_scope target;
		for(++parser; parser;){
			compile_statement(target, parser, _global_compilers);
		}
		
		return module_ptr(new module(target.get_block(), target.get_number_of_variables()));
	}
	
#define ADD_GLOBAL_COMPILER(n, f) _global_compilers.emplace(n, std::bind(&compiler::priv:: f, this, _1, _2))
#define ADD_LOCAL_COMPILER(n, f) _local_compilers.emplace(n, std::bind(&compiler::priv:: f, this, _1, _2))

	void populate_compiler_maps(){
		ADD_GLOBAL_COMPILER("function", compile_function);
		ADD_GLOBAL_COMPILER("var", compile_variable);
		ADD_GLOBAL_COMPILER("for", compile_for);
		ADD_GLOBAL_COMPILER("while", compile_while);
		ADD_GLOBAL_COMPILER("do", compile_do);
		ADD_GLOBAL_COMPILER("if", compile_if);
		ADD_GLOBAL_COMPILER("switch", compile_switch);
		
		ADD_LOCAL_COMPILER("var", compile_variable);
		ADD_LOCAL_COMPILER("for", compile_for);
		ADD_LOCAL_COMPILER("while", compile_while);
		ADD_LOCAL_COMPILER("do", compile_do);
		ADD_LOCAL_COMPILER("if", compile_if);
		ADD_LOCAL_COMPILER("switch", compile_switch);
		ADD_LOCAL_COMPILER("return", compile_return);
	}

#undef ADD_LOCAL_COMPILER
#undef ADD_GLOBAL_COMPILER

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

		size_t len = ftell(fp);

		fseek(fp, 0, SEEK_SET);

		std::vector<char> v(len);

		if(fread(&v[0], 1, len, fp) != len){
			return false;
		}

		fclose(fp);

		try{
			tokenizer parser (&v[0], &v[0] + len);
			_modules[module_name] = compile_module(parser);
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

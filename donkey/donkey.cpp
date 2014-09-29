#include "donkey.hpp"
#include "tokenizer.hpp"
#include "identifiers.hpp"
#include "runtime_context.hpp"
#include "errors.hpp"
#include "statements.hpp"
#include "expression_builder.hpp"
#include "donkey_function.h"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstring>

namespace donkey{

typedef std::function<variable_ptr(runtime_context&, size_t)> function;

class module: public code_container{
	module(const module&) = delete;
	void operator=(const module&) = delete;
private:
	std::vector<function> _functions;
	statement _s;
	size_t _globals_count;
public:
	module(statement&& s, int globals_count, std::vector<function>&& functions):
		_functions(std::move(functions)),
		_s(std::move(s)),
		_globals_count(globals_count){
	}
	void load(runtime_context& ctx){
		ctx.global = std::vector<variable_ptr>(_globals_count);
		_s(ctx);
	}
	
	virtual variable_ptr call_function_by_address(code_address address, runtime_context& ctx, size_t prms) const override{
		return _functions[address](ctx, prms);
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
	bool _in_function;
	bool _is_switch;
	bool _can_break;
	bool _can_continue;
public:
	scope(scope* parent, bool is_function = false, bool is_switch = false, bool can_break = false, bool can_continue = false):
		_parent(parent),
		_var_index(parent->is_global() ? 0 : parent->_var_index),
		_initial_index(parent->is_global() ? 0 : parent->_var_index),
		_in_function(is_function || _parent->in_function()),
		_is_switch(is_switch),
		_can_break(can_break || parent->can_break()),
		_can_continue(can_continue || parent->can_continue()){
	}
	
	scope():
		_parent(nullptr),
		_var_index(0),
		_initial_index(0),
		_in_function(false),
		_is_switch(false),
		_can_break(false),
		_can_continue(false){
	}

	virtual identifier_ptr get_identifier(std::string name) const override{
		auto it = _variables.find(name);
		return it != _variables.end() ? it->second : _parent ? _parent->get_identifier(name) : identifier_ptr();
	}
	
	virtual bool is_allowed(std::string name) const override{
		return _variables.find(name) == _variables.end();
	}
	
	bool is_global() const{
		return !_parent;
	}
	
	bool in_function() const{
		return _in_function;
	}
	
	bool is_switch() const{
		return _is_switch;
	}
	
	bool can_break() const{
		return _can_break;
	}
	
	bool can_continue() const{
		return _can_continue;
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
	
	template<typename T>
	void add_statement(T&& s){
		_statements.push_back(statement(s));
	}
	
	size_t get_number_of_statements() const{
		return _statements.size();
	}
	
	statement get_block(){
		if(get_number_of_variables() == 0 && _statements.size() < 2){
			if(_statements.empty()){
				return statement(empty_statement);
			}
			return std::move(_statements.front());
		}
		if(is_global()){
			return statement(block_statement(std::move(_statements), 0));
		}
		
		return statement(block_statement(std::move(_statements), _var_index - _initial_index));
	}
	
	std::vector<statement> get_statements(){
		return std::move(_statements);
	}
	
	size_t get_number_of_variables() const{
		return _var_index - _initial_index;
	}
};

class global_scope: public scope{
private:
	std::unordered_map<std::string, function_identifier_ptr> _functions;
	std::vector<function> _definitions;
public:
	bool has_function(std::string name) const{
		auto it = _functions.find(name);
		if(it == _functions.end()){
			return false;
		}
		code_address idx = it->second->get_function();
		return bool(_definitions[idx]);
	}
	
	void declare_function(std::string name){
		if(_functions.find(name) == _functions.end()){
			return;
		}
		_functions[name].reset(new function_identifier(_definitions.size()));
		_definitions.push_back(function());
	}
	
	void define_function(std::string name, function&& f){
		function_identifier_ptr& ptr = _functions[name];
		
		if(ptr){
			_definitions[ptr->get_function()] = std::move(f);
		}else{
			ptr.reset(new function_identifier(_definitions.size()));
			_definitions.push_back(std::move(f));
		}
	}
	
	std::string get_undefined_function() const{
		for(const auto& p: _functions){
			if(!_definitions[p.second->get_function()]){
				return p.first;
			}
		}
		return "";
	}
	
	virtual identifier_ptr get_identifier(std::string name) const override{
		auto it = _functions.find(name);
		return it != _functions.end() ? it->second : scope::get_identifier(name);
	}
	
	virtual bool is_allowed(std::string name) const override{
		return _functions.find(name) == _functions.end() && scope::is_allowed(name);
	}
	
	std::vector<function> get_functions(){
		return std::move(_definitions);
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

	static void parse(const std::string& token, tokenizer& parser){
		if(*parser != token){
			syntax_error(parser.get_line_number(), token + " expected");
		}
		++parser;
	}

	static std::string parse_allowed_name(tokenizer& parser){
		if(parser.get_token_type() != tokenizer::tt_word){
			unexpected_error(parser.get_line_number(), *parser);
		}
		if(is_keyword(*parser)){
			syntax_error(parser.get_line_number(), *parser + "is keyword");
		}
		return *(parser++);
	}

	static std::string parse_allowed_name(identifier_lookup& lookup, tokenizer& parser){
		if(!lookup.is_allowed(*parser)){
			syntax_error(parser.get_line_number(), *parser + "is already declared");
		}
		return parse_allowed_name(parser);
	}

	void compile_function(global_scope& target, tokenizer& parser){
		++parser;
		std::string name = parse_allowed_name(parser);
		parse("(", parser);
		
		std::vector<std::string> params;
		if(*parser == ")"){
			++parser;
			if(*parser == ";"){
				target.declare_function(name);
				++parser;
				return;
			}
		}else{
			bool first_param = true;
			do{
				if(!first_param){
					parse(",", parser);
				}
				params.push_back(parse_allowed_name(parser));
				first_param = false;
				
			}while(*parser != ")");
			
			++parser;
		}
		
		if(target.has_function(name)){
			semantic_error(parser.get_line_number(), name + " is already defined");
		}
		
		target.declare_function(name);
		
		scope function_scope(&target, true);
		
		for(const std::string& prm: params){
			if(!function_scope.add_variable(prm)){
				semantic_error(parser.get_line_number(), prm + " is already defined");
			}
		}
		
		function_scope.add_variable("%RETVAL%");
		
		parse("{", parser);
		
		while(parser){
			if(*parser == "}"){
				++parser;
				target.define_function(name, donkey_function(params.size(), function_scope.get_block()));
				return;
			}
			compile_statement(function_scope, parser, _local_compilers);
		}
		syntax_error(parser.get_line_number(), "'}' expected");
		
	}

	
	expression_ptr compile_variable(scope& target, tokenizer& parser){
		if(target.is_switch()){
			syntax_error(parser.get_line_number(), " declarations in switch are not allowed");
		}
	
		expression_ptr ret;
		
		++parser;
		
		while(parser && *parser != ";"){
			std::string name = parse_allowed_name(target, parser);
			target.add_variable(name);
			if(*parser == "="){
				++parser;
				ret = build_binary_expression(
					oper::assignment,
					identifier_to_expression(name, target),
					build_expression(target, parser, false, true)
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

	void compile_cpp_for(scope& target, tokenizer& parser){
		scope outer(&target);
		
		compile_statement(outer, parser, _local_compilers);
		
		expression_ptr e2 = build_expression(outer, parser, false);
		parse(";", parser);
		expression_ptr e3 = build_expression(outer, parser, true);
		
		parse(")", parser);
		
		scope s(&outer, false, false, true, true);
		compile_statement(s, parser, _local_compilers);
		
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
		compile_statement(s, parser, _local_compilers);
		
		target.add_statement(for_statement(e1, e2, e3, s.get_block()));
	}

	void compile_for(scope& target, tokenizer& parser){
		++parser;
		
		parse("(", parser);
		
		if(*parser == "var"){
			compile_cpp_for(target, parser);
		}else{
			compile_c_for(target, parser);
		}
	}

	void compile_while(scope& target, tokenizer& parser){
		++parser;
		parse("(", parser);
		expression_ptr e = build_expression(target, parser, false);
		parse(")", parser);
		scope s(&target, false, false, true, true);
		compile_statement(s, parser, _local_compilers);
		target.add_statement(while_statement(e, s.get_block()));
	}

	void compile_do(scope& target, tokenizer& parser){
		++parser;
		scope s(&target, false, false, true, true);
		compile_statement(s, parser, _local_compilers);
		parse("while", parser);
		parse("(", parser);
		expression_ptr e = build_expression(target, parser, false);
		parse(")", parser);
		parse(";", parser);
		target.add_statement(do_statement(e, s.get_block()));
	}

	void compile_if(scope& target, tokenizer& parser){
		std::vector<expression_ptr> es;
		std::vector<statement> ss;
		
		do{
			++parser;
			parse("(", parser);
			es.push_back(build_expression(target, parser, false));
			parse(")", parser);
			scope s(&target);
			compile_statement(s, parser, _local_compilers);
			ss.push_back(s.get_block());
		}while(*parser == "elif");
		
		if(*parser == "else"){
			++parser;
			scope s(&target);
			compile_statement(s, parser, _local_compilers);
			ss.push_back(s.get_block());
		}
		target.add_statement(if_statement(std::move(es), std::move(ss)));
	}

	void compile_switch(scope& target, tokenizer& parser){
		std::unordered_map<double, size_t> cases;
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
				double d = parse_double(*parser);
				if(isnan(d)){
					unexpected_error(parser.get_line_number(), *parser);
				}
				
				if(cases.find(d) != cases.end()){
					semantic_error(parser.get_line_number(), "duplicated case " + *parser);
				}
				++parser;
				parse(":", parser);
				cases[d] = s.get_number_of_statements();
			}else if(*parser == "default"){
				++parser;
				if(has_dflt){
					semantic_error(parser.get_line_number(), "duplicated default");
				}
				parse(":", parser);
				dflt = s.get_number_of_statements();
				has_dflt = true;
			}
			compile_statement(s, parser, _local_compilers);
		}
		syntax_error(parser.get_line_number(), "'}' expected");
	}

	void compile_break(scope& target, tokenizer& parser){
		if(!target.can_break()){
			semantic_error(parser.get_line_number(), "unexpected break");
		}
		++parser;
		target.add_statement(break_statement);
		parse(";", parser);
	}
	
	void compile_continue(scope& target, tokenizer& parser){
		if(!target.can_break()){
			semantic_error(parser.get_line_number(), "unexpected continue");
		}
		++parser;
		target.add_statement(continue_statement);
		parse(";", parser);
	}

	void compile_return(scope& target, tokenizer& parser){
		if(!target.in_function()){
			semantic_error(parser.get_line_number(), "unexpected return");
		}
		++parser;
		target.add_statement(return_statement(build_expression(target, parser, true)));
		parse(";", parser);
	}

	void compile_expression_statement(scope& target, tokenizer& parser){
		target.add_statement(expression_statement(build_expression(target, parser, true)));
		parse(";", parser);
	}

	template<typename TARGET, class COMPILER_MAP>
	void compile_statement(TARGET& target, tokenizer& parser, const COMPILER_MAP& compilers){
		auto it = compilers.find(*parser);
		if(it == compilers.end()){
			if(is_keyword(*parser)){
				unexpected_error(parser.get_line_number(), *parser);
			}
			compile_expression_statement(target, parser);
			return;
		}
		it->second(target, parser);
	}

	void compile_local_scope(scope& target, tokenizer& parser){
		scope s(&target);
		for(++parser; parser;){
			if(*parser == "}"){
				++parser;
				target.add_statement(s.get_block());
				return;
			}
			compile_statement(s, parser, _local_compilers);
		}
		syntax_error(parser.get_line_number(), "'}' expected");
	}
	
	module_ptr compile_module(tokenizer& parser){
		global_scope target;
		for(; parser;){
			compile_statement(target, parser, _global_compilers);
		}
		
		std::string not_defined = target.get_undefined_function();
		if(not_defined != ""){
			semantic_error(parser.get_line_number(), not_defined + " is not defined");
		}
		
		return module_ptr(new module(target.get_block(), target.get_number_of_variables(), target.get_functions()));
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
		ADD_GLOBAL_COMPILER("{", compile_local_scope);
		
		ADD_LOCAL_COMPILER("var", compile_variable);
		ADD_LOCAL_COMPILER("for", compile_for);
		ADD_LOCAL_COMPILER("while", compile_while);
		ADD_LOCAL_COMPILER("do", compile_do);
		ADD_LOCAL_COMPILER("if", compile_if);
		ADD_LOCAL_COMPILER("switch", compile_switch);
		ADD_LOCAL_COMPILER("return", compile_return);
		ADD_LOCAL_COMPILER("{", compile_local_scope);
		ADD_LOCAL_COMPILER("break", compile_break);
		ADD_LOCAL_COMPILER("continue", compile_continue);
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

		//try{
			tokenizer parser (&v[0], &v[0] + len);
			module_ptr m = compile_module(parser);
			_modules[module_name] = m;
			return true;
		//}catch(const exception&){
		//	return false;
		//}
	}
	
	bool execute_module(const char* module_name){
//		try{
			auto it = _modules.find(module_name);
			
			if(it == _modules.end()){
				return false;
			}
			
			runtime_context ctx(it->second.get());
			
			it->second->load(ctx);
			
			printf("%s\n", variable::to_string(ctx.global[0]).c_str());
			
			return true;
//		}catch(const exception&){
//			return false;
//		}
	}
};

compiler::compiler(const char* root):
	_private(new priv(root)){
}

bool compiler::compile_module(const char* module_name){
	return _private->compile_module(module_name);
}

bool compiler::execute_module(const char* module_name){
	return _private->execute_module(module_name);
}

compiler::~compiler(){
	delete _private;
}

}//namespace donkey

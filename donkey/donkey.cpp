#include "donkey.hpp"
#include "tokenizer.hpp"
#include "scope.hpp"
#include "module.hpp"
#include "donkey_function.h"
#include "expression_builder.hpp"

#include <algorithm>
#include <unordered_set>

namespace donkey{

typedef std::function<void(scope&, tokenizer&)> compiler_function;

typedef std::unordered_map<std::string, compiler_function > compiler_map;

using namespace std::placeholders;


const char* keywords[] = {
	"break",
	"case",
	"class",
	"continue",
	"default",
	"do",
	"elif",
	"else",
	"for",
	"function",
	"if",
	"new",
	"null",
	"number",
	"object",
	"ref",
	"return",
	"self",
	"string",
	"switch",
	"this",
	"var",
	"while",
};

inline bool str_less(const char* l, const char* r){
	return strcmp(l, r) < 0;
}

bool is_keyword(const std::string& name){
	return std::binary_search(keywords, keywords + sizeof(keywords)/sizeof(keywords[0]), name.c_str(), &str_less);
}

class compiler::priv{
private:
	std::string _root;
	compiler_map _compilers;
	
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
			syntax_error(parser.get_line_number(), *parser + " is keyword");
		}
		return *(parser++);
	}

	static std::string parse_allowed_name(identifier_lookup& lookup, tokenizer& parser){
		if(!lookup.is_allowed(*parser)){
			syntax_error(parser.get_line_number(), *parser + " is already declared");
		}
		return parse_allowed_name(parser);
	}
	
	void compile_method_stub(class_scope& target, tokenizer& parser){
		++parser;
		if(!target.is_allowed_member(*parser)){
			semantic_error(parser.get_line_number(), *parser + " is already defined");
		}
		std::string name = parse_allowed_name(parser);
		
		target.declare_method(name);
		
		parse("(", parser);
		
		for(int nesting = 1; parser && nesting; ++parser){
			if(*parser == "("){
				++nesting;
			}else if(*parser == ")"){
				--nesting;
			}
		}
		
		parse("{", parser);
		
		for(int nesting = 1; parser && nesting; ++parser){
			if(*parser == "{"){
				++nesting;
			}else if(*parser == "}"){
				--nesting;
			}
		}
	}
	
	void skip_constructor(tokenizer& parser){
		do{
			++parser;
		}while(parser && *parser != "{");
		
		parse("{", parser);
		
		for(int nesting = 1; parser && nesting; ++parser){
			if(*parser == "{"){
				++nesting;
			}else if(*parser == "}"){
				--nesting;
			}
		}
	}
	
	void skip_destructor(tokenizer& parser){
		do{
			++parser;
		}while(parser && *parser != "{");
		
		parse("{", parser);
		
		for(int nesting = 1; parser && nesting; ++parser){
			if(*parser == "{"){
				++nesting;
			}else if(*parser == "}"){
				--nesting;
			}
		}
	}
	
	const vtable* add_table(std::unordered_set<const vtable*>& tables, const vtable* table){
		if(tables.find(table) != tables.end()){
			return table->get_fields_size() ? table : nullptr;
		}
		tables.insert(table);
		
		std::vector<const vtable*> bases = table->get_bases();
		
		for(const vtable* base: bases){
			if(tables.find(base) != tables.end()){
				if(base->get_fields_size()){
					return base;
				}
			}else{
				tables.insert(base);
			}
		}
		
		return nullptr;
	}
	
	std::string check_diamond(const std::vector<std::string>& bases, const global_scope& scope){
		std::unordered_set<const vtable*> tables;
		
		for(const std::string& base: bases){
			const vtable* ret = add_table(tables, scope.get_vtable(base));
			if(ret){
				return ret->get_name();
			}
		}
		return "";
	}
	
	static variable default_constructor(const std::vector<vtable*>& bases, const variable& that, runtime_context& ctx, variable* params, size_t params_size){
		for(vtable* vt: bases){
			vt->call_base_constructor(that, ctx, params, params_size);
		}
		return variable();
	}
	
	static variable default_destructor(const std::vector<vtable*>& bases, const variable& that, runtime_context& ctx, variable*, size_t){
		for(size_t i = bases.size(); i > 0; --i){
			bases[i-1]->call_base_destructor(that, ctx);
		}
		return variable();
	}
	
	void compile_class(scope& target, tokenizer& parser){
		if(!target.is_global()){
			syntax_error(parser.get_line_number(), "local classes are not supported");
		}
		
		++parser;
		
		std::string name = parse_allowed_name(target, parser);
		
		class_scope ctarget(name, &target);
		
		std::vector<std::string> bases(1, "object");
		
		if(*parser == ":"){
			++parser;
			do{
				if(parser.get_token_type() != tokenizer::tt_word){
					unexpected_error(parser.get_line_number(), *parser);
				}
				std::string base = *parser;
				if(base == "number" || base == "string" || base == "function" || base == "null"){
					semantic_error(parser.get_line_number(), "cannot inherit from " + name);
				}
				if(!target.has_class(base)){
					semantic_error(parser.get_line_number(), "class " + base + " is undefined");
				}
				bases.push_back(base);
				++parser;
				if(*parser == ","){
					++parser;
				}
			}while(*parser != "{");
		}
		
		global_scope& gtarget = static_cast<global_scope&>(target);
		
		std::string diamond_base = check_diamond(bases, gtarget);
		
		if(!diamond_base.empty()){
			semantic_error(parser.get_line_number(), "non-empty diamond base " + diamond_base + " detected for class " + name);
		}
		
		parse("{", parser);
		
		bool constructor_defined = false;
		bool destructor_defined = false;
		
		for(tokenizer stub_parser = parser; *stub_parser != "}";){
			if(*stub_parser == "var"){
				compile_field(ctarget, stub_parser);
			}else if(*stub_parser == "function"){
				compile_method_stub(ctarget, stub_parser);
			}else if(*stub_parser == name){
				if(constructor_defined){
					semantic_error(parser.get_line_number(), "constructor is already defined");
				}
				skip_constructor(stub_parser);
				constructor_defined = true;
			}else if(*stub_parser == "~"){
				if(destructor_defined){
					semantic_error(parser.get_line_number(), "destructor is already defined");
				}
				skip_destructor(stub_parser);
				destructor_defined = true;
			}
		}
		
		if(!constructor_defined){
			std::vector<vtable*> bases_vt;
			for(const std::string& base: bases){
				bases_vt.push_back(gtarget.get_vtable(base));
			}
			ctarget.define_constructor(std::bind(&compiler::priv::default_constructor, bases_vt, _1, _2, _3, _4));
		}
		
		if(!destructor_defined){
			std::vector<vtable*> bases_vt;
			for(const std::string& base: bases){
				bases_vt.push_back(gtarget.get_vtable(base));
			}
			ctarget.define_destructor(std::bind(&compiler::priv::default_destructor, bases_vt, _1, _2, _3, _4));
		}
		
		gtarget.add_vtable(name, ctarget.create_vtable(bases));
		
		while(*parser != "}"){
			if(*parser == "var"){
				skip_field(parser);
			}else if(*parser == "function"){
				compile_function(ctarget, parser);
			}else if(*parser == name){
				compile_constructor(ctarget, parser, bases);
			}else if(*parser == "~"){
				compile_destructor(ctarget, parser, bases);
			}
		}
		
		++parser;
	}
	
	void compile_function_helper(std::string name, scope& target, tokenizer& parser,
		std::function<void(std::string, bool)> declare,
		std::function<void(std::string, scope&, size_t)> define,
		std::function<bool(std::string)> is_defined,
		std::function<void(scope&, tokenizer&)> pre_function){
		
		if(*parser == ";"){
			declare(name, true);
			++parser;
			return;
		}
		
		parse("(", parser);
		
		std::vector<std::string> params;
		bool first_param = true;
		
		while(*parser != ")"){
			if(!first_param){
				parse(",", parser);
			}
			params.push_back(parse_allowed_name(parser));
			first_param = false;
			
		}
		
		++parser;
		
		if(is_defined(name)){
			semantic_error(parser.get_line_number(), name + " is already defined");
		}
		
		declare(name, false);
		
		scope function_scope(&target, true);
		
		for(const std::string& prm: params){
			if(!function_scope.add_variable(prm)){
				semantic_error(parser.get_line_number(), prm + " is already defined");
			}
		}
		
		function_scope.add_variable("%RETVAL%");
		
		scope inner_scope(&function_scope);
		
		pre_function(inner_scope, parser);
		
		parse("{", parser);
		
		while(parser){
			if(*parser == "}"){
				++parser;
				function_scope.add_statement(inner_scope.get_block());
				define(name, function_scope, params.size());
				return;
			}
			compile_statement(inner_scope, parser);
		}
		syntax_error(parser.get_line_number(), "'}' expected");
	}

	static void declare_function(global_scope& target, std::string name, bool){
		target.declare_function(name);
	}

	static void define_function(global_scope& target, std::string name, scope& function_scope, size_t params_size){
		target.define_function(name, donkey_function(params_size, function_scope.get_local_vars(), function_scope.get_block()));
	}
	
	static void declare_method(tokenizer& parser, std::string, bool forward){
		if(forward){
			syntax_error(parser.get_line_number(), "forward declarations are not supported nor useful in class");
		}
	}
	
	static void define_method(class_scope& target, std::string name, scope& function_scope, size_t params_size){
		target.define_method(name, donkey_method(params_size, function_scope.get_local_vars(), function_scope.get_block()));
	}
	
	static void ignore_pre_function(scope&, tokenizer&){
	}

	void compile_function(scope& target, tokenizer& parser){
		if(target.is_global()){
			global_scope& gtarget = static_cast<global_scope&>(target);
			
			++parser;
			std::string name = parse_allowed_name(target, parser);
			
			compile_function_helper(name, target, parser,
				                    std::bind(&compiler::priv::declare_function, std::ref(gtarget), _1, _2),
				                    std::bind(&compiler::priv::define_function, std::ref(gtarget), _1, _2, _3),
				                    std::bind(&global_scope::has_function, std::cref(gtarget), _1),
				                    &compiler::priv::ignore_pre_function);
			return;
		}
		if(target.is_class()){
			class_scope& ctarget = static_cast<class_scope&>(target);
			
			++parser;
			std::string name = parse_allowed_name(parser);
			
			compile_function_helper(name, target, parser,
			                        std::bind(&compiler::priv::declare_method, std::ref(parser), _1, _2),
			                        std::bind(&compiler::priv::define_method, std::ref(ctarget), _1, _2, _3),
			                        std::bind(&class_scope::has_method, std::cref(ctarget), _1),
			                        &compiler::priv::ignore_pre_function);
			return;
		}
	
		unexpected_error(parser.get_line_number(), *parser);
	}
	
	static void declare_constructor(tokenizer& parser, std::string, bool forward){
		if(forward){
			syntax_error(parser.get_line_number(), "forward declaration of constructor is not supported not useful");
		}
	}
	
	static void define_constructor(class_scope& target, std::string, scope& function_scope, size_t params_size){
		target.define_constructor(donkey_method(params_size, function_scope.get_local_vars(), function_scope.get_block()));
	}
	
	static bool has_constructor(std::string){
		return false;
	}
	
	static void compile_pre_constructor(const std::vector<std::string>& bases, scope& inner, tokenizer& parser){
		std::unordered_map<std::string, bool> constructed;
		
		for(const std::string& base_name: bases){
			constructed.emplace(base_name, false);
		}
		
		if(*parser == ":"){
			++parser;
			while(parser && *parser != "{"){
				std::string base_name = parse_allowed_name(parser);
				
				auto it = constructed.find(base_name);
				
				if(it == constructed.end()){
					semantic_error(parser.get_line_number(), base_name + " is not base of this class");
				}
				
				if(it->second){
					semantic_error(parser.get_line_number(), base_name + " is already constructed");
				}
				
				parse("(", parser);
				
				if(*parser == ")"){
					inner.add_statement(base_default_constructor_statement(inner.get_vtable(base_name)));
				}else{
					std::vector<expression_ptr> params;
					while(parser && *parser != ")"){
						params.push_back(build_expression(inner, parser, false, true));
						if(*parser == ","){
							++parser;
						}
					}
					inner.add_statement(base_constructor_statement(inner.get_vtable(base_name), std::move(params)));
				}
				
				parse(")", parser);
				
				it->second = true;
				
				if(*parser == ","){
					++parser;
				}
			}
		}
		
		for(const std::string& base_name: bases){
			if(!constructed[base_name]){
				inner.add_statement(base_default_constructor_statement(inner.get_vtable(base_name)));
			}
		}
		
	}
	
	void compile_constructor(class_scope& target, tokenizer& parser, const std::vector<std::string>& bases){
		++parser;
		
		compile_function_helper("", target, parser,
		                        std::bind(&compiler::priv::declare_constructor, std::ref(parser), _1, _2),
		                        std::bind(&compiler::priv::define_constructor, std::ref(target), _1, _2, _3),
		                        &compiler::priv::has_constructor,
		                        std::bind(&compiler::priv::compile_pre_constructor, std::cref(bases), _1, _2)
		);
	}
	
	static void declare_destructor(tokenizer& parser, std::string, bool forward){
		if(forward){
			syntax_error(parser.get_line_number(), "forward declaration of destructor is not supported not useful");
		}
	}
	
	static void define_destructor(class_scope& target, tokenizer& parser, const std::vector<std::string>& bases, std::string, scope& function_scope, size_t params_size){
		if(params_size){
			syntax_error(parser.get_line_number(), "destructor cannot have parameters");
		}
		
		for(size_t i = bases.size(); i != 0; --i){
			function_scope.add_statement(base_destructor_statement(target.get_vtable(bases[i-1])));
		}
		
		target.define_destructor(donkey_method(params_size, function_scope.get_local_vars(), function_scope.get_block()));
	}
	
	static bool has_destructor(std::string){
		return false;
	}
	
	void compile_destructor(class_scope& target, tokenizer& parser, const std::vector<std::string>& bases){
		++parser;
		if(*parser != target.get_current_class()){
			unexpected_error(parser.get_line_number(), *parser);
		}
		++parser;
		
		compile_function_helper("", target, parser,
		                        std::bind(&compiler::priv::declare_destructor, std::ref(parser), _1, _2),
		                        std::bind(&compiler::priv::define_destructor, std::ref(target), std::ref(parser), std::cref(bases), _1, _2, _3),
		                        &compiler::priv::has_destructor,
		                        &compiler::priv::ignore_pre_function
		);
		
	}
	
	void compile_field(class_scope& target, tokenizer& parser){
		
		++parser;
		
		while(parser && *parser != ";"){
			std::string name = parse_allowed_name(parser);
			if(!target.is_allowed_member(name)){
				semantic_error(parser.get_line_number(), *parser + " is already defined");
			}
			target.add_field(name);
			if(*parser == ","){
				++parser;
			}
		}
		++parser;
	}
	
	void skip_field(tokenizer& parser){
		++parser;
		
		while(parser && *parser != ";"){
			++parser; //name;
			if(*parser == ","){
				++parser;
			}
		}
		++parser;
	}
	
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
			
			int idx = target.is_global() ?
				static_cast<global_variable_identifier&>(*id).get_index() :
				target.is_function() ?
					static_cast<parameter_identifier&>(*id).get_index() :
					static_cast<local_variable_identifier&>(*id).get_index();
				
			
			if(*parser == "="){
				++parser;
				ret = build_binary_expression(
					oper::assignment,
					target.is_global() ?
						build_global_variable_expression(idx) : 
						target.is_function() ?
							build_parameter_expression(idx) :
							build_local_variable_expression(idx),
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

	void compile_if(scope& target, tokenizer& parser){
		if(target.is_class()){
			unexpected_error(parser.get_line_number(), *parser);
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
			unexpected_error(parser.get_line_number(), *parser);
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
			if(*parser != "case" && *parser != "default"){
				compile_statement(s, parser);
			}			
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
		if(target.is_class()){
			unexpected_error(parser.get_line_number(), *parser);
		}
		target.add_statement(expression_statement(build_expression(target, parser, true)));
		parse(";", parser);
	}

	void compile_statement(scope& target, tokenizer& parser){
		auto it = _compilers.find(*parser);
		if(it == _compilers.end()){
			if(is_keyword(*parser)){
				unexpected_error(parser.get_line_number(), *parser);
			}
			compile_expression_statement(target, parser);
			return;
		}
		it->second(target, parser);
	}

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
	
	module_ptr compile_module(tokenizer& parser){
		global_scope target;
		for(; parser;){
			compile_statement(target, parser);
		}
		
		std::string not_defined = target.get_undefined_function();
		if(not_defined != ""){
			semantic_error(parser.get_line_number(), not_defined + " is not defined");
		}
		
		return module_ptr(new module(target.get_block(), target.get_number_of_variables(), target.get_local_vars(), target.get_functions(), target.get_vtables()));
	}
	
#define ADD_COMPILER(n, f) _compilers.emplace(n, std::bind(&compiler::priv:: f, this, _1, _2))

	void populate_compiler_maps(){
		ADD_COMPILER("function", compile_function);
		ADD_COMPILER("var", compile_variable);
		ADD_COMPILER("for", compile_for);
		ADD_COMPILER("while", compile_while);
		ADD_COMPILER("do", compile_do);
		ADD_COMPILER("if", compile_if);
		ADD_COMPILER("switch", compile_switch);
		ADD_COMPILER("return", compile_return);
		ADD_COMPILER("{", compile_local_scope);
		ADD_COMPILER("break", compile_break);
		ADD_COMPILER("continue", compile_continue);
		ADD_COMPILER("class", compile_class);
		ADD_COMPILER("this", compile_expression_statement);
		ADD_COMPILER("null", compile_expression_statement);
		ADD_COMPILER("self", compile_expression_statement);
		ADD_COMPILER("new", compile_expression_statement);
	}

#undef ADD_COMPILER

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
			
			runtime_context ctx(it->second.get(), it->second->get_globals_count());
			
			it->second->load(ctx);
			
			variable that = ctx.global(0);
			
			printf("%s\n", get_vtable(ctx, that)->call_member(that, ctx, nullptr, 0, "toString").to_string().c_str());
			
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

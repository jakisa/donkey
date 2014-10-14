#include "function_compiler.hpp"
#include "compiler.hpp"
#include "statement_compiler.hpp"
#include "donkey_function.h"
#include "compiler_helpers.hpp"
#include "expression_builder.hpp"

using namespace std::placeholders;

namespace donkey{

inline void compile_function_helper(std::string name, scope& target, tokenizer& parser,
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

inline void declare_function(global_scope& target, std::string name, bool){
	target.declare_function(name);
}

inline void define_function(global_scope& target, std::string name, scope& function_scope, size_t params_size){
	target.define_function(name, donkey_function(params_size, function_scope.get_block()));
}

inline void declare_method(tokenizer& parser, std::string, bool forward){
	if(forward){
		syntax_error(parser.get_line_number(), "forward declarations are not supported nor useful in class");
	}
}

inline void define_method(class_scope& target, std::string name, scope& function_scope, size_t params_size){
	target.define_method(name, donkey_method(params_size, function_scope.get_block()));
}

inline void ignore_pre_function(scope&, tokenizer&){
}

void compile_function(scope& target, tokenizer& parser){
	if(target.is_global()){
		global_scope& gtarget = static_cast<global_scope&>(target);
		
		++parser;
		std::string name = parse_allowed_name(target, parser);
		
		compile_function_helper(name, target, parser,
								std::bind(&declare_function, std::ref(gtarget), _1, _2),
								std::bind(&define_function, std::ref(gtarget), _1, _2, _3),
								std::bind(&global_scope::has_function, std::cref(gtarget), _1),
								&ignore_pre_function);
		return;
	}
	if(target.is_class()){
		class_scope& ctarget = static_cast<class_scope&>(target);
		
		++parser;
		std::string name = parse_allowed_name(parser);
		
		compile_function_helper(name, target, parser,
								std::bind(&declare_method, std::ref(parser), _1, _2),
								std::bind(&define_method, std::ref(ctarget), _1, _2, _3),
								std::bind(&class_scope::has_method, std::cref(ctarget), _1),
								&ignore_pre_function);
		return;
	}

	unexpected_error(parser.get_line_number(), *parser);
}

inline void declare_constructor(tokenizer& parser, std::string, bool forward){
	if(forward){
		syntax_error(parser.get_line_number(), "forward declaration of constructor is not supported not useful");
	}
}

inline void define_constructor(class_scope& target, std::string, scope& function_scope, size_t params_size){
	target.define_constructor(donkey_method(params_size, function_scope.get_block()));
}

inline bool has_constructor(std::string){
	return false;
}

inline void compile_pre_constructor(const std::vector<std::string>& bases, scope& inner, tokenizer& parser){
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
							std::bind(&declare_constructor, std::ref(parser), _1, _2),
							std::bind(&define_constructor, std::ref(target), _1, _2, _3),
							&has_constructor,
							std::bind(&compile_pre_constructor, std::cref(bases), _1, _2)
	);
}

inline void declare_destructor(tokenizer& parser, std::string, bool forward){
	if(forward){
		syntax_error(parser.get_line_number(), "forward declaration of destructor is not supported not useful");
	}
}

inline void define_destructor(class_scope& target, tokenizer& parser, const std::vector<std::string>& bases, std::string, scope& function_scope, size_t params_size){
	if(params_size){
		syntax_error(parser.get_line_number(), "destructor cannot have parameters");
	}
	
	for(size_t i = bases.size(); i != 0; --i){
		function_scope.add_statement(base_destructor_statement(target.get_vtable(bases[i-1])));
	}
	
	target.define_destructor(donkey_method(params_size, function_scope.get_block()));
}

inline bool has_destructor(std::string){
	return false;
}

void compile_destructor(class_scope& target, tokenizer& parser, const std::vector<std::string>& bases){
	++parser;
	if(*parser != target.get_current_class()){
		unexpected_error(parser.get_line_number(), *parser);
	}
	++parser;
	
	compile_function_helper("", target, parser,
							std::bind(&declare_destructor, std::ref(parser), _1, _2),
							std::bind(&define_destructor, std::ref(target), std::ref(parser), std::cref(bases), _1, _2, _3),
							&has_destructor,
							&ignore_pre_function
	);
	
}

}//donkey;
#include "class_compiler.hpp"
#include "compiler_helpers.hpp"
#include "function_compiler.hpp"
#include <unordered_set>

using namespace std::placeholders;

namespace donkey{

inline variable default_constructor(const std::vector<vtable*>& bases, const variable& that, runtime_context& ctx, size_t params_size){
	for(vtable* vt: bases){
		vt->call_base_constructor(that, ctx, params_size);
	}
	return variable();
}

inline variable default_destructor(const std::vector<vtable*>& bases, const variable& that, runtime_context& ctx, size_t){
	for(size_t i = bases.size(); i > 0; --i){
		bases[i-1]->call_base_destructor(that, ctx);
	}
	return variable();
}

inline void compile_field(class_scope& target, tokenizer& parser){
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

inline void skip_field(tokenizer& parser){
	++parser;
	
	while(parser && *parser != ";"){
		++parser; //name;
		if(*parser == ","){
			++parser;
		}
	}
	++parser;
}

inline void compile_method_stub(class_scope& target, tokenizer& parser){
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

inline void skip_constructor(tokenizer& parser){
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

inline void skip_destructor(tokenizer& parser){
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

inline const vtable* add_table(std::unordered_set<const vtable*>& tables, const vtable* table){
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

inline std::string check_diamond(const std::vector<std::string>& bases, const global_scope& scope){
	std::unordered_set<const vtable*> tables;
	
	for(const std::string& base: bases){
		const vtable* ret = add_table(tables, scope.get_vtable(base));
		if(ret){
			return ret->get_name();
		}
	}
	return "";
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
		ctarget.define_constructor(std::bind(&default_constructor, bases_vt, _1, _2, _3));
	}
	
	if(!destructor_defined){
		std::vector<vtable*> bases_vt;
		for(const std::string& base: bases){
			bases_vt.push_back(gtarget.get_vtable(base));
		}
		ctarget.define_destructor(std::bind(&default_destructor, bases_vt, _1, _2, _3));
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

}//donkey

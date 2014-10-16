#ifndef compiler_helpers_hpp
#define compiler_helpers_hpp

#include "identifiers.hpp"
#include "tokenizer.hpp"
#include "errors.hpp"
#include "compiler.hpp"
#include "scope.hpp"
#include <utility>

namespace donkey {

inline std::string parse_allowed_name(identifier_lookup& lookup, tokenizer& parser){
	if(!lookup.is_allowed(*parser)){
		syntax_error(*parser + " is already declared");
	}
	return parse_allowed_name(parser);
}

inline vtable* get_class(const identifier_lookup& lookup, tokenizer& parser){
	if(parser.get_token_type() != tokenizer::tt_word){
		unexpected_error(*parser);
	}
	
	identifier_ptr id = lookup.get_identifier(*parser);
	
	if(!id){
		unexpected_error(*parser);
	}
	
	switch(id->get_type()){
		case identifier_type::module:
			{
				++parser;
				parse("::", parser);
				return get_class(static_cast<module_identifier&>(*id).get_lookup(), parser);
			}
		case identifier_type::classname:
			++parser;
			return static_cast<class_identifier&>(*id).get_vtable();
		default:
			semantic_error(*parser + " is not class");
			return nullptr;
	}
}


}//donkey

#endif // compiler_helpers_hpp

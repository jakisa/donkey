#ifndef compiler_helpers_hpp
#define compiler_helpers_hpp

#include "identifiers.hpp"
#include "tokenizer.hpp"
#include "errors.hpp"
#include "compiler.hpp"
#include <utility>

namespace donkey {

inline std::string parse_allowed_name(identifier_lookup& lookup, tokenizer& parser){
	if(!lookup.is_allowed(*parser)){
		syntax_error(*parser + " is already declared");
	}
	return parse_allowed_name(parser);
}

inline std::pair<std::string, std::string> get_class(std::string module, tokenizer& parser){
	if(parser.get_token_type() != tokenizer::tt_word){
		unexpected_error(*parser);
	}
	std::string classname = *parser;
	++parser;
	if(*parser == "::"){
		++parser;
		if(parser.get_token_type() != tokenizer::tt_word){
			unexpected_error(*parser);
		}
		module = classname;
		classname = module + "::" + *parser;
		++parser;
	}else{
		classname = module + "::" + classname;
	}
	return std::make_pair(std::move(module), std::move(classname));
}


}//donkey

#endif // compiler_helpers_hpp

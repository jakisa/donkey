#ifndef compiler_helpers_hpp
#define compiler_helpers_hpp

#include "identifiers.hpp"
#include "tokenizer.hpp"
#include "errors.hpp"
#include "compiler.hpp"

namespace donkey {

inline std::string parse_allowed_name(identifier_lookup& lookup, tokenizer& parser){
	if(!lookup.is_allowed(*parser)){
		syntax_error(parser.get_line_number(), *parser + " is already declared");
	}
	return parse_allowed_name(parser);
}

}//donkey

#endif // compiler_helpers_hpp

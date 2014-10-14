#ifndef __compiler_hpp__
#define __compiler_hpp__

#include <algorithm>
#include <string.h>
#include "tokenizer.hpp"
#include "errors.hpp"

namespace donkey{

template<int dummy>
struct tkeywords{
	static const char* arr[];
};

template<int dummy>
const char* tkeywords<dummy>::arr[] = {
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

typedef tkeywords<0> keywords;

inline bool str_less(const char* l, const char* r){
	return strcmp(l, r) < 0;
}

inline bool is_keyword(const std::string& name){
	return std::binary_search(keywords::arr, keywords::arr + sizeof(keywords::arr)/sizeof(keywords::arr[0]), name.c_str(), &str_less);
}

inline void parse(const std::string& token, tokenizer& parser){
	if(*parser != token){
		syntax_error(parser.get_line_number(), token + " expected");
	}
	++parser;
}

inline std::string parse_allowed_name(tokenizer& parser){
	if(parser.get_token_type() != tokenizer::tt_word){
		unexpected_error(parser.get_line_number(), *parser);
	}
	if(is_keyword(*parser)){
		syntax_error(parser.get_line_number(), *parser + " is keyword");
	}
	return *(parser++);
}

}//donkey

#endif /* __compiler_hpp__*/


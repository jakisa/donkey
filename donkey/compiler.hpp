#ifndef __compiler_hpp__
#define __compiler_hpp__

#include <algorithm>
#include <string.h>
#include "tokenizer.hpp"
#include "errors.hpp"

namespace donkey{

template<int dummy>
struct tkeywords{
	enum{
		count = 29
	};
	static const char* arr[count];
};

template<int dummy>
const char* tkeywords<dummy>::arr[count] = {
	"array",
	"break",
	"case",
	"const",
	"class",
	"continue",
	"default",
	"do",
	"elif",
	"else",
	"final",
	"for",
	"function",
	"if",
	"import",
	"new",
	"null",
	"number",
	"object",
	"public",
	"ref",
	"return",
	"self",
	"string",
	"switch",
	"this",
	"using",
	"var",
	"while",
};

typedef tkeywords<0> keywords;

inline bool str_less(const char* l, const char* r){
	return strcmp(l, r) < 0;
}

inline bool is_keyword(const std::string& name){
	return std::binary_search(keywords::arr, keywords::arr + keywords::count, name.c_str(), &str_less);
}

inline void parse(const std::string& token, tokenizer& parser){
	if(*parser != token){
		syntax_error(token + " expected");
	}
	++parser;
}

inline std::string parse_allowed_name(tokenizer& parser){
	if(parser.get_token_type() != tokenizer::tt_word){
		unexpected_error(*parser);
	}
	if(is_keyword(*parser)){
		syntax_error(*parser + " is keyword");
	}
	return *(parser++);
}

}//donkey

#endif /* __compiler_hpp__*/


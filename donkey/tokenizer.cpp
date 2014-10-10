#include "tokenizer.hpp"
#include <cctype>
#include <algorithm>
#include "errors.hpp"

#include <cstdio>

namespace donkey{

static const char* two_letter_operators[] = {
	"!=",
	"%=",
	"&&",
	"&=",
	"*=",
	"++",
	"+=",
	"--",
	"-=",
	"->",
	".."
	"/=",
	"::",
	"<<",
	"<=",
	"==",
	">=",
	">>",
	"?\?",
	"\\=",
	"^=",
	"|=",
	"||",
};

static const char* three_letter_operators[] = {
	"..=",
	"<<=",
	">>=",
	"?\?=",
};

static const char* skip_whitespaces(const char* current, const char* end){
	while(current < end && isspace(*current)){
		++current;
	}
	return current;
}

inline bool test_two(const char* current, const char* end, char ch0, char ch1){
	return current + 1 < end && current[0] == ch0 && current[1] == ch1;
}

inline bool test_three(const char* current, const char* end, char ch0, char ch1, char ch2){
	return current + 2 < end && current[0] == ch0 && current[1] == ch1 && current[2] == ch2;
}

inline bool is_anu(char ch){
	return isalnum(ch) || ch == '_';
}

static tokenizer::token_type get_next_token_type(const char* current, const char* end){
	if(current == end){
		return tokenizer::tt_eof;
	}
	if(test_two(current, end, '/', '/')){
		return tokenizer::tt_line_comment;
	}
	if(test_two(current, end, '/', '*')){
		return tokenizer::tt_block_comment;
	}
	if(*current == '"'){
		return tokenizer::tt_string;
	}
	if(isdigit(*current)){
		return tokenizer::tt_number;
	}
	if(is_anu(*current)){
		return tokenizer::tt_word;
	}
	return tokenizer::tt_operator;
}

static int get_line_number(const char* begin, const char*, const char* current){
	return std::count(begin, current, '\n');
}

#define PARSE_ERROR(str) parse_error(get_line_number(begin, end, current), str)

static void skip_line_comment(const char*, const char* end, const char*& current){
	for(current += 2; current != end && *current != '\n'; ++current);
	
	if(current != end){
		++current;
	}
}

static void skip_block_comment(const char* begin, const char* end, const char*& current){
	for(current += 2; current != end; ++current){
		if(test_two(current, end, '*', '/')){
			current += 2;
			return;
		}
	}
	PARSE_ERROR("block comment is not closed");
}

static std::string fetch_string(const char* begin, const char* end, const char*& current){
	std::string ret;
	bool escape = false;
	for(++current; current != end; ++current){
		if(escape){
			ret += *current;
			escape = false;
		}else{
			switch(*current){
				case '"':
					++current;
					return '"' + ret + '"';
				case '\\':
					escape = true;
				default:
					ret += *current;
					break;
			}
		}
	}
	PARSE_ERROR("string literal is not closed");
	return "";
}

std::string tokenizer::unquoted_string(const std::string& token, int line_number){
	std::string ret;
	bool escape = false;
	for(size_t i=1; i + 1 < token.size(); ++i){
		if(escape){
			switch(token[i]){
				case '\\':
					ret += '\\';
					break;
				case '"':
					ret += '"';
					break;
				case 't':
					ret += '\t';
					break;
				case 'r':
					ret += '\r';
					break;
				case 'n':
					ret += '\n';
				case '0':
					ret += '\0';
					break;
				default:
					parse_error(line_number, "unknown escape sequence");
					break;
			}
			escape = false;
		}else{
			switch(token[i]){
				case '\\':
					escape = true;
					break;
				case '\t':
					parse_error(line_number, "tab character in string literal");
					break;
				case '\r':
				case '\n':
					parse_error(line_number, "newline in string literal");
					break;
				default:
					ret += token[i];
					break;
			}
		}
	}
	return ret;
}


static std::string fetch_number(const char*, const char* end, const char*& current){
	const char* old = current;
	for(++current; current != end; ++current){
		if(*current == '.'){
			if(current + 1 != end && !isdigit(*(current+1))){
				break;
			}
		}else if(!is_anu(*current)){
			break;
		}
	}
	return std::string(old, current);
}

static std::string fetch_word(const char*, const char* end, const char*& current){
	const char* old = current;
	for(++current; current != end; ++current){
		if(!is_anu(*current)){
			break;
		}
	}
	return std::string(old, current);
}

static std::string fetch_operator(const char*, const char* end, const char*& current){
	if(current < end - 2){
		if(std::binary_search(
				three_letter_operators,
				three_letter_operators + sizeof(three_letter_operators)/sizeof(three_letter_operators[0]),
				current,
				[](const char* l, const char* r){
					return std::lexicographical_compare(l, l+3, r, r+3);
				}
			)){
			const char* old = current;
			current += 3;
			return std::string(old, current);
		}
	}
	if(current < end - 1){
		if(std::binary_search(
				two_letter_operators,
				two_letter_operators + sizeof(two_letter_operators)/sizeof(two_letter_operators[0]),
				current,
				[](const char* l, const char* r){
					return std::lexicographical_compare(l, l+2, r, r+2);
				}
			)){
			const char* old = current;
			current += 2;
			return std::string(old, current);
		}
	}
	
	return std::string(1, *(current++));
}

#undef PARSE_ERROR

void tokenizer::_fetch_next_token(){
	bool skip = true;
	while(skip){
		_current = skip_whitespaces(_current, _end);
		_tt = get_next_token_type(_current, _end);
		switch(_tt){
			case tt_line_comment:
				skip_line_comment(_begin, _end, _current);
				break;
			case tt_block_comment:
				skip_block_comment(_begin, _end, _current);
				break;
			default:
				skip = false;
		}
	}
	switch(_tt){
		case tt_string:
			_token = fetch_string(_begin, _end, _current);
			break;
		case tt_number:
			_token = fetch_number(_begin, _end, _current);
			break;
		case tt_word:
			_token = fetch_word(_begin, _end, _current);
			if(_token == "new"){
				_tt = tt_operator;
			}
			break;
		case tt_operator:
			_token = fetch_operator(_begin, _end, _current);
			break;
		default:
			_token = "";
			break;
	}
}

int tokenizer::get_line_number() const{
	return donkey::get_line_number(_begin, _end, _current);
}

}//namespace donkey


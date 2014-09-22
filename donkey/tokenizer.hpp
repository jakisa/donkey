#ifndef __tokenizer_hpp__
#define __tokenizer_hpp__

#include <string>

namespace donkey{

class tokenizer{
public:
	enum token_type{
		tt_line_comment,
		tt_block_comment,
		tt_string,
		tt_number,
		tt_operator,
		tt_word,
		tt_eof,
	};
private:
	const char* const _begin;
	const char* const _end;
	const char* _current;
	token_type _tt;
	std::string _token;
	
	void _fetch_next_token();
	
public:
	tokenizer(const char* begin, const char* end):
		_begin(begin),
		_end(end){
		reset();
	}

	void reset(){
		_current = _begin;
		_fetch_next_token();
	}
	
	explicit operator bool() const {
		return _tt != tt_eof;
	}
	
	const std::string& operator*() const{
		return _token;
	}
	
	const std::string* operator->() const{
		return &_token;
	}
	
	tokenizer& operator++(){
		if(_current < _end){
			_fetch_next_token();
		}
		return *this;
	}
	
	tokenizer operator++(int){
		tokenizer ret = *this;
		(*this)++;
		return ret;
	}
    
    token_type get_token_type() const{
    	return _tt;
    }
    
    int get_line_number() const;
};

}//namespace donkey

#endif /*__tokenizer_hpp__*/


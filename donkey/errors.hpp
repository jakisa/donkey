#ifndef __errors_hpp__
#define __errors_hpp__

#include <exception>
#include <string>

namespace donkey{

class exception: public std::exception{
private:
	std::string _what;
public:
	exception(std::string what):
		_what(std::move(what)){
	}
	
	virtual const char* what() const throw() override{
		return _what.c_str();
	}
};

class exception_raw: public std::exception{
private:
	std::string _what;
public:
	exception_raw(std::string what):
		_what(std::move(what)){
	}
	
	virtual const char* what() const throw() override{
		return _what.c_str();
	}
	
	void throw_formatted(std::string file, int line) const{
		std::string str = _what;
		
		size_t fpos = str.find("%FILE%");
		if(fpos != std::string::npos){
			str = str.replace(fpos, 6, file);
		}
		
		size_t lpos = str.find("%LINE%");
		if(lpos != std::string::npos){
			str = str.replace(lpos, 6, std::to_string(line));
		}
		
		throw donkey::exception(str);
	}
};

inline void error(std::string error_type, std::string message){
	throw exception_raw(std::string(error_type) + " in %FILE%:%LINE%: " + message);
}

inline void parse_error(std::string message){
	error("Parse error" , message);
}

inline void syntax_error(std::string message){
	error("Syntax error", message);
}

inline void semantic_error(std::string message){
	error("Semantic error", message);
}

inline void unexpected_error(std::string message){
	syntax_error("unexpected " + (message.empty() ? std::string("end of file") : message));
}

inline void runtime_error(std::string message){
	throw exception("Runtime error: " + message);
}


}//namespace donkey

#endif /*__errors_hpp__*/


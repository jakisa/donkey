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

inline void error(int line_number, std::string error_type, std::string message){
	throw exception(std::string(error_type) + " in " + std::to_string(line_number+1) + ": " + message);
}

inline void parse_error(int line_number, std::string message){
	error(line_number, "Parse error", message);	
}

inline void syntax_error(int line_number, std::string message){
	error(line_number, "Syntax error", message);
}

inline void semantic_error(int line_number, std::string message){
	error(line_number, "Semantic error", message);
}

inline void unexpected_error(int line_number, std::string message){
	syntax_error(line_number, "unexpected " + (message.empty() ? std::string("end of file") : message));
}

inline void runtime_error(std::string message){
	throw exception("Runtime error: " + message);
}


}//namespace donkey

#endif /*__errors_hpp__*/


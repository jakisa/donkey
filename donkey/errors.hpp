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
		_what(what.c_str()){
	}
	
	virtual const char* what() const throw() override{
		return _what.c_str();
	}
};

inline void error(int line_number, const char* error_type, const char* message){
	throw exception(std::string(error_type) + " in " + std::to_string(line_number) + ":" + message);
}

inline void parse_error(int line_number, const char* message){
	error(line_number, "Parse error", message);	
}

inline void syntax_error(int line_number, const char* message){
	error(line_number, "Syntax error", message);
}


}//namespace donkey

#endif /*__errors_hpp__*/


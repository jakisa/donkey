#ifndef __errors_hpp__
#define __errors_hpp__

#include <string>
#include <exception>

namespace donkey{

class exception: public std::exception{
private:
	std::string _what;
public:
	exception(std::string what);
	
	virtual const char* what() const throw() override;
	
	const std::string& what_str() const;
};

class exception_raw: public std::exception{
private:
	std::string _what;
public:
	exception_raw(std::string what);
	
	virtual const char* what() const throw() override;
	
	void throw_formatted(std::string file, size_t line) const;
};

class runtime_exception: public exception{
public:
	runtime_exception(std::string what);
	
	void add_stack_trace(std::string trace) const;
};

void parse_error(std::string message);

void syntax_error(std::string message);

void semantic_error(std::string message);

inline void unexpected_error(std::string message){
	syntax_error("unexpected " + (message.empty() ? std::string("end of file") : message));
}

void runtime_error(std::string message);


}//namespace donkey

#endif /*__errors_hpp__*/


#include "errors.hpp"

namespace donkey{

exception::exception(std::string what):
	_what(std::move(what)){
}

const char* exception::what() const throw(){
	return _what.c_str();
}

const std::string& exception::what_str() const{
	return _what;
}

exception_raw::exception_raw(std::string what):
	_what(std::move(what)){
}

const char* exception_raw::what() const throw(){
	return _what.c_str();
}

void exception_raw::throw_formatted(std::string file, size_t line) const{
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

runtime_exception::runtime_exception(std::string what):
	exception(std::move(what)){
}

void runtime_exception::add_stack_trace(std::string trace) const{
	std::string str = what_str();
	str += "\n\tat " + std::move(trace);
	throw runtime_exception(std::move(str));
}

static void error(std::string error_type, std::string message){
	throw exception_raw(std::string(error_type) + " in %FILE%:%LINE%: " + message);
}

void parse_error(std::string message){
	error("Parse error" , message);
}

void syntax_error(std::string message){
	error("Syntax error", message);
}

void semantic_error(std::string message){
	error("Semantic error", message);
}

void runtime_error(std::string message){
	throw runtime_exception("Runtime error: " + std::move(message));
}


void add_stack_trace(const runtime_exception& ex, std::string trace){
	ex.add_stack_trace(std::move(trace));
}

void throw_formatted(const exception_raw& ex, std::string filename, size_t line_number){
	ex.throw_formatted(filename, line_number);
}

}//namespace donkey


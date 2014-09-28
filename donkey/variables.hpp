#ifndef __variables_hpp__
#define __variables_hpp__

#include <string>
#include <memory>
#include <utility>
#include <cmath>

#include "runtime_context.hpp"
#include "types.hpp"
#include "errors.hpp"
#include "helpers.hpp"

namespace donkey{

class variable;
typedef std::shared_ptr<variable> variable_ptr;

class variable: public std::enable_shared_from_this<variable>{
	variable(const variable&) = delete;
	void operator=(const variable&) = delete;
protected:
	variable(){
	}
public:
	virtual variable_ptr as_param(){
		return shared_from_this();
	}

	virtual const char* get_type_name() = 0;

	virtual type get_type() = 0;

	virtual bool equals(variable_ptr oth){
		return shared_from_this() == oth;
	}

	virtual bool equals(double){
		return false;
	}

	virtual bool equals(std::string){
		return false;
	}

	virtual bool less(variable_ptr oth){
		runtime_error(std::string("cannot compare ") + get_type_name() + " and " + get_type_name(oth));
		return false;
	}

	virtual bool less(double){
		runtime_error(std::string("cannot compare ") + get_type_name() + " and number");
		return false;
	}

	virtual bool less(std::string){
		runtime_error(std::string("cannot compare ") + get_type_name() + " and string");
		return false;
	}

	virtual bool greater(variable_ptr oth){
		runtime_error(std::string("cannot compare ") + get_type_name() + " and " + get_type_name(oth));
		return false;
	}

	virtual bool greater(double){
		runtime_error(std::string("cannot compare ") + get_type_name() + " and number");
		return false;
	}

	virtual bool greater(std::string){
		runtime_error(std::string("cannot compare ") + get_type_name() + " and string");
		return false;
	}

	virtual std::string to_string(){
		return std::string(get_type_name()) + " " + std::to_string((long)this);
	}

	static const char* get_type_name(variable_ptr v){
		return v ? v->get_type_name() : "null";
	}

	static type get_type(variable_ptr v){
		return v ? v->get_type() : type::nothing;
	}

	static bool equals(variable_ptr l, variable_ptr r){
		return l ? l->equals(r) : (r ? false : true);
	}

	static bool equals(variable_ptr l, double r){
		return l ? l->equals(r) : false;
	}

	static bool equals(variable_ptr l, std::string r){
		return l ? l->equals(r) : false;
	}

	static bool less(variable_ptr l, variable_ptr r){
		if(!l){
			runtime_error(std::string("cannot compare null and ") + get_type_name(r));
		}
		return l->less(r);
	}

	static bool less(variable_ptr l, double r){
		if(!l){
			runtime_error(std::string("cannot compare null and number"));
		}
		return l->less(r);
	}

	static bool less(variable_ptr l, std::string r){
		if(!l){
			runtime_error(std::string("cannot compare null and string"));
		}
		return l->less(r);
	}

	static bool greater(variable_ptr l, variable_ptr r){
		if(!l){
			runtime_error(std::string("cannot compare null and ") + get_type_name(r));
		}
		return l->greater(r);
	}

	static bool greater(variable_ptr l, double r){
		if(!l){
			runtime_error(std::string("cannot compare null and number"));
		}
		return l->greater(r);
	}

	static bool greater(variable_ptr l, std::string r){
		if(!l){
			runtime_error(std::string("cannot compare null and string"));
		}
		return l->greater(r);
	}

	static std::string to_string(variable_ptr v){
		return v ? v->to_string() : "null";
	}
	
	static variable_ptr as_param(variable_ptr v){
		return v ? v->as_param() : variable_ptr();
	}

	virtual ~variable(){
	}
};


class number_variable: public variable{
private:
	double _value;
public:
	number_variable(double value = 0):
		_value(value){
	}

	virtual variable_ptr as_param() override{
		return variable_ptr(new number_variable(_value));
	}

	double& value(){
		return _value;
	}

	virtual const char* get_type_name() override{
		return "number";
	}

	virtual type get_type() override{
		return type::number;
	}

	virtual bool equals(variable_ptr oth) override{
		return variable::get_type(oth) == type::number && std::static_pointer_cast<number_variable>(oth)->_value == _value;
	}

	virtual bool equals(double n) override{
		return _value == n;
	}

	virtual bool less(variable_ptr oth) override{
		if(variable::get_type(oth) == type::number){
			return std::static_pointer_cast<number_variable>(oth)->_value < _value;
		}
		return variable::less(oth);
	}

	virtual bool less(double n) override{
		return _value < n;
	}

	virtual bool greater(variable_ptr oth) override{
		if(variable::get_type(oth) == type::number){
			return std::static_pointer_cast<number_variable>(oth)->_value > _value;
		}
		return variable::less(oth);
	}

	virtual bool greater(double n) override{
		return _value > n;
	}

	virtual std::string to_string() override{
		return donkey::to_string(_value);
	}
};

class string_variable: public variable{
private:
	std::string _value;
public:
	string_variable(std::string value = ""):
		_value(std::move(value)){
	}

	const std::string& value(){
		return _value;
	}

	virtual const char* get_type_name() override{
		return "string";
	}

	virtual type get_type() override{
		return type::string;
	}

	virtual bool equals(variable_ptr oth) override{
		return variable::get_type(oth) == type::string && std::static_pointer_cast<string_variable>(oth)->_value == _value;
	}

	virtual bool equals(std::string s) override{
		return _value == s;
	}

	virtual bool less(variable_ptr oth) override{
		if(variable::get_type(oth) == type::string){
			return std::static_pointer_cast<string_variable>(oth)->_value < _value;
		}
		return variable::less(oth);
	}

	virtual bool less(std::string s) override{
		return _value < s;
	}

	virtual bool greater(variable_ptr oth) override{
		if(variable::get_type(oth) == type::string){
			return std::static_pointer_cast<string_variable>(oth)->_value > _value;
		}
		return variable::less(oth);
	}

	virtual bool greater(std::string s) override{
		return _value > s;
	}

	virtual std::string to_string() override{
		return _value;
	}

};


class function_variable: public variable{
private:
	function _value;
public:
	function_variable(function value):
		_value(std::move(value)){
	}

	function& value(){
		return _value;
	}

	virtual const char* get_type_name() override{
		return "function";
	}

	virtual type get_type() override{
		return type::function;
	}
};


}//namespace donkey

#endif /* __variables_hpp__ */

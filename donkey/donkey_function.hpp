#ifndef __donkey_function_h__
#define __donkey_function_h__

#include "runtime_context.hpp"
#include "statements.hpp"

namespace donkey{

class donkey_function{
private:
	size_t _params_count;
	statement _body;
	std::string _name;
public:
	donkey_function(const donkey_function& orig):
		_params_count(orig._params_count),
		_body(orig._body),
		_name(orig._name){
	}

	donkey_function(donkey_function&& orig):
		_params_count(orig._params_count),
		_body(std::move(orig._body)),
		_name(std::move(orig._name)){
	}
	
	donkey_function(const std::string& name, size_t params_count, statement&& body):
		_params_count(params_count),
		_body(std::move(body)),
		_name(name){
	}
	variable operator()(runtime_context& ctx, size_t params_count) const{
		try{
			function_stack_manipulator _(ctx, _params_count, params_count);
			
			_body(ctx);
			
			return ctx.top();
		}catch(const runtime_exception& e){
			e.add_stack_trace(_name);
		}
		return variable();
	}
};


class donkey_method{
private:
	size_t _params_count;
	statement _body;
	std::string _name;
public:
	donkey_method(const donkey_method& orig):
		_params_count(orig._params_count),
		_body(orig._body),
		_name(orig._name){
	}

	donkey_method(donkey_method&& orig):
		_params_count(orig._params_count),
		_body(std::move(orig._body)),
		_name(orig._name){
	}
	
	donkey_method(const std::string& name, size_t params_count, statement&& body):
		_params_count(params_count),
		_body(std::move(body)),
		_name(name){
	}
	variable operator()(const variable& that, runtime_context& ctx, size_t params_count) const{
		try{
			function_stack_manipulator _(ctx, _params_count, params_count, &that);
			
			_body(ctx);
			
			return ctx.top();
		}catch(const runtime_exception& e){
			e.add_stack_trace(_name);
		}
		return variable();
	}
};

}

#endif /*__donkey_function_h__*/


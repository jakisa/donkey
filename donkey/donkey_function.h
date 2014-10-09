#ifndef __donkey_function_h__
#define __donkey_function_h__

#include "runtime_context.hpp"
#include "statements.hpp"

namespace donkey{

class donkey_function{
private:
	size_t _params_count;
	statement _body;
public:
	donkey_function(const donkey_function& orig):
		_params_count(orig._params_count),
		_body(orig._body){
	}

	donkey_function(donkey_function&& orig):
		_params_count(orig._params_count),
		_body(std::move(orig._body)){
	}
	
	donkey_function(size_t params_count, statement&& body):
		_params_count(params_count),
		_body(std::move(body)){
	}
	variable operator()(runtime_context& ctx, size_t params_count) const{
		function_stack_manipulator _(ctx, _params_count, params_count);
		
		_body(ctx);
		
		return ctx.top();
	}
};


class donkey_method{
private:
	size_t _params_count;
	statement _body;
public:
	donkey_method(const donkey_method& orig):
		_params_count(orig._params_count),
		_body(orig._body){
	}

	donkey_method(donkey_method&& orig):
		_params_count(orig._params_count),
		_body(std::move(orig._body)){
	}
	
	donkey_method(size_t params_count, statement&& body):
		_params_count(params_count),
		_body(std::move(body)){
	}
	variable operator()(variable& that, runtime_context& ctx, size_t params_count) const{
		function_stack_manipulator _(ctx, _params_count, params_count, &that);
		
		_body(ctx);
		
		return ctx.top();
	}
};

}

#endif /*__donkey_function_h__*/


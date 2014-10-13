#ifndef __donkey_function_h__
#define __donkey_function_h__

#include "runtime_context.hpp"
#include "statements.hpp"

namespace donkey{

class donkey_function{
private:
	size_t _params_count;
	size_t _locals_count;
	statement _body;
public:
	donkey_function(const donkey_function& orig):
		_params_count(orig._params_count),
		_locals_count(orig._locals_count),
		_body(orig._body){
	}

	donkey_function(donkey_function&& orig):
		_params_count(orig._params_count),
		_locals_count(orig._locals_count),
		_body(std::move(orig._body)){
	}
	
	donkey_function(size_t params_count, size_t locals_count, statement&& body):
		_params_count(params_count),
		_locals_count(locals_count),
		_body(std::move(body)){
	}
	variable operator()(runtime_context& ctx, variable* params, size_t params_count) const{
		size_t dflt_cnt = _params_count > params_count ? _params_count - params_count : 0;
		
		STACK_ALLOC(dflt, dflt_cnt);
		
		STACK_ALLOC(locals, _locals_count);
		
		function_stack_manipulator _(ctx, locals, params, params_count, dflt, nullptr);
		
		_body(ctx);
		
		return ctx.retval();
	}
};


class donkey_method{
private:
	size_t _params_count;
	size_t _locals_count;
	statement _body;
public:
	donkey_method(const donkey_method& orig):
		_params_count(orig._params_count),
		_locals_count(orig._locals_count),
		_body(orig._body){
	}

	donkey_method(donkey_method&& orig):
		_params_count(orig._params_count),
		_locals_count(orig._locals_count),
		_body(std::move(orig._body)){
	}
	
	donkey_method(size_t params_count, size_t locals_count, statement&& body):
		_params_count(params_count),
		_locals_count(locals_count),
		_body(std::move(body)){
	}
	variable operator()(const variable& that, runtime_context& ctx, variable* params, size_t params_count) const{
		size_t dflt_cnt = _params_count > params_count ? _params_count - params_count : 0;
		
		STACK_ALLOC(dflt, dflt_cnt);
		
		STACK_ALLOC(locals, _locals_count);
		
		function_stack_manipulator _(ctx, locals, params, params_count, dflt, &that);
		
		_body(ctx);
		
		return ctx.retval();
	}
};

}

#endif /*__donkey_function_h__*/


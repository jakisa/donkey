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
	donkey_function(donkey_function&& orig):
		_params_count(orig._params_count),
		_body(std::move(orig._body)){
	}
	donkey_function(size_t params_count, statement& body):
		_params_count(params_count),
		_body(std::move(body)){
	}
	variable_ptr operator()(runtime_context& ctx, size_t params_count) const{
		for(; params_count > _params_count; --params_count){
			ctx.pop();
		}
		for(; params_count < _params_count; ++params_count){
			ctx.push(variable_ptr());
		}
		ctx.push(variable_ptr());
		
		_body(ctx);
		
		return ctx.pop();
	}
};

}

#endif /*__donkey_function_h__*/


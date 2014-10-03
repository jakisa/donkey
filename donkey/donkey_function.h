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
	template<typename T>
	donkey_function(T&& orig):
		_params_count(orig._params_count),
		_body(std::move(orig._body)){
	}
	template<typename T>
	donkey_function(size_t params_count, T&& body):
		_params_count(params_count),
		_body(std::move(body)){
	}
	variable operator()(runtime_context& ctx, size_t params_count) const{
		for(; params_count > _params_count; --params_count){
			ctx.pop();
		}
		for(; params_count < _params_count; ++params_count){
			ctx.push(variable());
		}
		
		ctx.function_stack_bottom = ctx.stack.size() - _params_count;
		ctx.retval_stack_index = ctx.stack.size();
		
		ctx.push(variable());
		
		_body(ctx);
		
		variable ret = ctx.top();
		ctx.pop();
		return ret;
	}
};

}

#endif /*__donkey_function_h__*/


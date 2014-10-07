#ifndef __context_hpp__
#define __context_hpp__

#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

#include "variables.hpp"

namespace donkey{

class module;

struct runtime_context{
	runtime_context(const runtime_context&) = delete;
	void operator=(const runtime_context&) = delete;
	
	std::vector<variable> stack;
	
	size_t function_stack_bottom;
	size_t retval_stack_index;
	
	variable* that;
	
	const module* code;

	runtime_context(const module* code):
		function_stack_bottom(0),
		retval_stack_index(-1),
		that(nullptr),
		code(code){
	}

	void push(variable v){
		stack.push_back(v);
	}

	void pop(){
		stack.pop_back();
	}
	variable& top(){
		return stack.back();
	}
};

variable call_function_by_address(code_address addr, runtime_context& ctx, size_t params_size);
	
vtable* get_vtable(runtime_context& ctx, std::string name);



class stack_restorer{
	stack_restorer(const stack_restorer&) = delete;
	void operator=(const stack_restorer&) = delete;
private:
	size_t _function_stack_bottom;
	size_t _retval_stack_index;
	size_t _stack_size;
	variable* _that;
	runtime_context& _ctx;
public:
	stack_restorer(runtime_context& ctx):
		_function_stack_bottom(ctx.function_stack_bottom),
		_retval_stack_index(ctx.retval_stack_index),
		_stack_size(ctx.stack.size()),
		_that(ctx.that),
		_ctx(ctx){
	}
	~stack_restorer(){
		_ctx.stack.resize(_stack_size);
		_ctx.retval_stack_index = _retval_stack_index;
		_ctx.function_stack_bottom = _function_stack_bottom;
		_ctx.that = _that;
	}
};

}//namespace donkey


#endif /*__context_hpp__*/


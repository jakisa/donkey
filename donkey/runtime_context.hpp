#ifndef __context_hpp__
#define __context_hpp__

#include <vector>
#include <deque>
#include <memory>
#include <functional>
#include <cstdint>

#include "variables.hpp"

namespace donkey{

class runtime_context;

class code_container{
	code_container(const code_container&) = delete;
	void operator=(const code_container&) = delete;
public:	
	code_container(){
	}
	virtual stack_var call_function_by_address(code_address, runtime_context&, size_t) const = 0;
	virtual ~code_container(){
	}
};


struct runtime_context{
	runtime_context(const runtime_context&) = delete;
	void operator=(const runtime_context&) = delete;
	std::vector<stack_var> global;
	std::deque<stack_var> stack;
	
	size_t function_stack_bottom;
	size_t retval_stack_index;
	
	const code_container* code;

	runtime_context(const code_container* code):
		function_stack_bottom(0),
		retval_stack_index(-1),
		code(code){
	}

	void push(stack_var v){
		stack.push_back(v);
	}

	void pop(){
		stack.pop_back();
	}
	stack_var& top(){
		return stack.back();
	}
};



class stack_restorer{
	stack_restorer(const stack_restorer&) = delete;
	void operator=(const stack_restorer&) = delete;
private:
	size_t _function_stack_bottom;
	size_t _retval_stack_index;
	size_t _stack_size;
	runtime_context& _ctx;
public:
	stack_restorer(runtime_context& ctx):
		_function_stack_bottom(ctx.function_stack_bottom),
		_retval_stack_index(ctx.retval_stack_index),
		_stack_size(ctx.stack.size()),
		_ctx(ctx){
	}
	~stack_restorer(){
		_ctx.stack.resize(_stack_size);
		_ctx.retval_stack_index = _retval_stack_index;
		_ctx.function_stack_bottom = _function_stack_bottom;
	}
};

}//namespace donkey


#endif /*__context_hpp__*/


#ifndef __context_hpp__
#define __context_hpp__

#include <vector>
#include <memory>
#include <functional>

namespace donkey{

class variable;
typedef std::shared_ptr<variable> variable_ptr;

struct runtime_context{
	runtime_context(const runtime_context&) = delete;
	void operator=(const runtime_context&) = delete;
	
	std::vector<variable_ptr> global;
	std::vector<variable_ptr> stack;
	
	size_t function_stack_bottom;
	size_t retval_stack_index;

	runtime_context():
		function_stack_bottom(0),
		retval_stack_index(-1){
	}

	void push(variable_ptr v){
		stack.push_back(v);
	}

	variable_ptr pop(){
		variable_ptr ret = stack.back();
		stack.pop_back();
		return ret;
	}
	variable_ptr& top(){
		return stack.back();
	}
};

typedef std::function<variable_ptr(runtime_context&, size_t)> function;

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


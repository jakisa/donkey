#ifndef __runtime_context_hpp__
#define __runtime_context_hpp__

#include <vector>
#include <memory>
#include <functional>
#include <cstdint>
#include <set>

#include "variables.hpp"
#include "stack.hpp"

namespace donkey{

class runtime_context{
	friend class stack_pusher;
	friend class stack_remover;
	friend class constructor_stack_manipulator;
	friend class function_stack_manipulator;
	
	runtime_context(const runtime_context&) = delete;
	void operator=(const runtime_context&) = delete;
private:
	stack _stack;
	size_t _function_stack_bottom;
	size_t _retval_stack_index;
	const variable* _that;
	std::set<std::string>* _constructed;

	void push_default(size_t cnt){
		_stack.add_size(cnt);
	}

	void push(variable&& v){
		_stack.push(std::move(v));
	}
	
	void check_size(size_t sz){
		_stack.check_size(sz);
	}
	
	void pop(size_t cnt){
		_stack.pop(cnt);
	}
	
	size_t stack_size(){
		return _stack.size();
	}
	
	void store_stack(std::vector<variable>& vs, size_t cnt){
		for(size_t i = 0; i < cnt; ++i){
			vs.push_back(std::move(_stack.top(i)));
		}
		_stack.pop(cnt);
	}
	void restore_stack(std::vector<variable>& vs){
		for(size_t i = vs.size(); i > 0; --i){
			_stack.push(std::move(vs[i-1]));
		}
	}
public:
	runtime_context(size_t stack_size):
		_stack(stack_size),
		_function_stack_bottom(0),
		_retval_stack_index(-1),
		_that(nullptr),
		_constructed(nullptr){
	}
	
	variable& top(size_t idx = 0){
		return _stack.top(idx);
	}
	
	variable& local(size_t idx){
		return _stack[_function_stack_bottom + idx];
	}
	
	void set_retval(variable&& v){
		_stack[_retval_stack_index] = std::move(v);
	}

	
	const variable* that(){
		return _that;
	}
	
	bool is_constructed(const std::string& str) const{
		return _constructed->find(str) != _constructed->end();
	}
	
	bool is_destroyed(const std::string& str) const{
		return is_constructed(str);
	}
	
	void set_constructed(const std::string& str) const{
		_constructed->insert(str);
	}
	
	void set_destroyed(const std::string& str) const{
		set_constructed(str);
	}
};

class stack_pusher{
	stack_pusher(const stack_pusher&) = delete;
	void operator=(const stack_pusher&) = delete;
private:
	runtime_context& _ctx;
	size_t _cnt;
public:
	stack_pusher(runtime_context& _ctx, size_t sz):
		_ctx(_ctx),
		_cnt(0){
		_ctx.check_size(sz);
	}
	
	void push(variable&& v){
		_ctx.push(std::move(v));
		++_cnt;
	}
	
	void push_default(size_t cnt){
		_ctx.push_default(cnt);
		_cnt += cnt;
	}
	
	~stack_pusher(){
		_ctx.pop(_cnt);
	}
};

class stack_remover{
	stack_remover(const stack_remover&) = delete;
	void operator=(const stack_remover&) = delete;
private:
	std::vector<variable> _removed;
	runtime_context& _ctx;
public:
	stack_remover(runtime_context& ctx, size_t remove_cnt):
		_ctx(ctx){
		if(remove_cnt){
			_removed.reserve(remove_cnt);
			_ctx.store_stack(_removed, remove_cnt);
		}
	}
	
	~stack_remover(){
		if(_removed.size()){
			_ctx.restore_stack(_removed);
		}
	}
};

class constructor_stack_manipulator{
	constructor_stack_manipulator(const constructor_stack_manipulator&) = delete;
	void operator=(const constructor_stack_manipulator&) = delete;
private:
	std::set<std::string> _constructed;
	std::set<std::string>* _old_constructed;
	runtime_context& _ctx;
public:
	constructor_stack_manipulator(runtime_context& ctx):
		_old_constructed(ctx._constructed),
		_ctx(ctx){
		ctx._constructed = &_constructed;
	}
	~constructor_stack_manipulator(){
		_ctx._constructed = _old_constructed;
	}
};

typedef constructor_stack_manipulator destructor_stack_manipulator;

class function_stack_manipulator{
	function_stack_manipulator(const function_stack_manipulator&) = delete;
	void operator=(const function_stack_manipulator&) = delete;
private:
	stack_remover _remover;
	stack_pusher _pusher;
	runtime_context& _ctx;
	size_t _function_stack_bottom;
	size_t _retval_stack_index;
	const variable* _that;
public:
	function_stack_manipulator(runtime_context& ctx, size_t expected_params, size_t passed_params, const variable* that = nullptr):
		_remover(ctx, expected_params < passed_params ? passed_params - expected_params : 0),
		_pusher(ctx, expected_params > passed_params ? expected_params - passed_params + 1 : 1),
		_ctx(ctx),
		_function_stack_bottom(ctx._function_stack_bottom),
		_retval_stack_index(ctx._retval_stack_index),
		_that(ctx._that){
		
		if(expected_params > passed_params){
			_pusher.push_default(expected_params - passed_params);
		}
		_ctx._function_stack_bottom = _ctx.stack_size() - expected_params;
		_ctx._retval_stack_index = _ctx.stack_size();
		
		_ctx._that = that;
		
		_pusher.push_default(1);
	}
	
	~function_stack_manipulator(){
		_ctx._function_stack_bottom = _function_stack_bottom;
		_ctx._retval_stack_index = _retval_stack_index;
		_ctx._that = _that;
	}
};

variable call_function_by_address(code_address addr, runtime_context& ctx, size_t params_size);

variable& global_variable(runtime_context& ctx, uint32_t module_index, uint32_t var_index);

}//namespace donkey


#endif /*__runtime_context_hpp__*/


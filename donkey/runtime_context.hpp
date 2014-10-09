#ifndef __context_hpp__
#define __context_hpp__

#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

#include "variables.hpp"

namespace donkey{

class module;


class runtime_context{
	friend class stack_pusher;
	friend class stack_remover;
	friend class function_stack_manipulator;
	
	runtime_context(const runtime_context&) = delete;
	void operator=(const runtime_context&) = delete;
private:
	std::vector<variable> _globals;
	std::vector<variable> _stack;
	const module* _code;
	size_t _function_stack_bottom;
	size_t _retval_stack_index;
	variable* _that;

	void push_default(size_t cnt){
		_stack.resize(_stack.size() + cnt);
	}

	void push(variable&& v){
		_stack.push_back(std::move(v));
	}
	
	template<class It>
	void push(It begin, It end){
		_stack.insert(_stack.end(), begin, end);
	}
	void pop(size_t cnt){
		_stack.erase(_stack.end() - cnt, _stack.end());
	}
	
	void store_stack(std::vector<variable>& vs, size_t cnt){
		for(size_t i = _stack.size() - cnt; i != _stack.size(); ++i){
			vs.push_back(std::move(_stack[i]));
		}
		_stack.resize(_stack.size() - cnt);
	}
	void restore_stack(std::vector<variable>& vs){
		for(variable& v: vs){
			_stack.push_back(std::move(v));
		}
	}
public:
	runtime_context(const module* code, size_t globals_count):
		_globals(globals_count),
		_code(code),
		_function_stack_bottom(0),
		_retval_stack_index(-1),
		_that(nullptr){
	}
	
	const module* code(){
		return _code;
	}
	
	variable& top(size_t idx = 0){
		return _stack[_stack.size() - idx - 1];
	}
	
	variable& local(size_t idx){
		return _stack[_function_stack_bottom + idx];
	}
	
	variable& global(size_t idx){
		return _globals[idx];
	}
	
	void set_retval(variable&& v){
		_stack[_retval_stack_index] = std::move(v);
	}

	
	variable* that(){
		return _that;
	}
	
	stack_var_ptr by_ref(variable& v){
		variable* p = &v;
	
		if(!_stack.empty() && p >= &_stack.front() && p <= &_stack.back()){
			return stack_var_ptr{&_stack, size_t(p - &_stack.front()), nullptr};
		}
		if(!_globals.empty() && p >= &_globals.front() && p <= &_globals.back()){
			return stack_var_ptr{&_globals, size_t(p - &_globals.front()), nullptr};
		}
		
		return stack_var_ptr{nullptr, 0, p};
	}
	
};

class stack_pusher{
	stack_pusher(const stack_pusher&) = delete;
	void operator=(const stack_pusher&) = delete;
private:
	runtime_context& _ctx;
	size_t _cnt;
public:
	stack_pusher(runtime_context& _ctx):
		_ctx(_ctx),
		_cnt(0){
	}
	
	void push(variable&& v){
		_ctx.push(std::move(v));
		++_cnt;
	}
	
	template<class It>
	void push(It begin, It end){
		_ctx.push(begin, end);
		_cnt += (end - begin);
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
		_removed.reserve(remove_cnt);
		_ctx.store_stack(_removed, remove_cnt);
	}
	
	~stack_remover(){
		_ctx.restore_stack(_removed);
	}
};

class function_stack_manipulator{
	function_stack_manipulator(const function_stack_manipulator&) = delete;
	void operator=(const function_stack_manipulator&) = delete;
private:
	stack_remover _remover;
	stack_pusher _pusher;
	runtime_context& _ctx;
	size_t _function_stack_bottom;
	size_t _retval_stack_index;
	variable* _that;
public:
	function_stack_manipulator(runtime_context& ctx, size_t expected_params, size_t passed_params, variable* that = nullptr):
		_remover(ctx, expected_params < passed_params ? passed_params - expected_params : 0),
		_pusher(ctx),
		_ctx(ctx),
		_function_stack_bottom(ctx._function_stack_bottom),
		_retval_stack_index(ctx._retval_stack_index),
		_that(ctx._that){
		
		if(expected_params > passed_params){
			_pusher.push_default(expected_params - passed_params);
		}
		_ctx._function_stack_bottom = _ctx._stack.size() - expected_params;
		_ctx._retval_stack_index = _ctx._stack.size();
		
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
	
vtable* get_vtable(runtime_context& ctx, std::string name);
vtable* get_vtable(runtime_context& ctx, const variable& v);

}//namespace donkey


#endif /*__context_hpp__*/


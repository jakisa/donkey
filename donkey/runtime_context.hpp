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

class module;


class runtime_context{
	friend class stack_pusher;
	friend class stack_remover;
	friend class constructor_stack_manipulator;
	friend class function_stack_manipulator;
	friend class stack_ref_manipulator;
	
	runtime_context(const runtime_context&) = delete;
	void operator=(const runtime_context&) = delete;
private:
	std::vector<variable> _globals;
	stack _stack;
	const module* _code;
	size_t _function_stack_bottom;
	size_t _retval_stack_index;
	variable* _that;
	std::set<std::string>* _constructed;

	void push_default(size_t cnt){
		_stack.add_size(cnt);
	}

	void push(variable&& v){
		_stack.push(std::move(v));
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
	runtime_context(const module* code, size_t globals_count):
		_globals(globals_count),
		_code(code),
		_function_stack_bottom(0),
		_retval_stack_index(-1),
		_that(nullptr),
		_constructed(nullptr){
	}
	
	const module* code(){
		return _code;
	}
	
	variable& top(size_t idx = 0){
		return _stack.top(idx);
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
	
	bool is_constructed(const std::string& str) const{
		return _constructed->find(str) != _constructed->end();
	}
	
	void set_constructed(const std::string& str) const{
		_constructed->insert(str);
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
	
vtable* get_vtable(runtime_context& ctx, std::string name);
vtable* get_vtable(runtime_context& ctx, const variable& v);

}//namespace donkey


#endif /*__runtime_context_hpp__*/


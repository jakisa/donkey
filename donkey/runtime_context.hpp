#ifndef __runtime_context_hpp__
#define __runtime_context_hpp__

#include <vector>
#include <memory>
#include <functional>
#include <cstdint>
#include <set>

#include "variables.hpp"

namespace donkey{

class module;


class runtime_context{
	friend class function_stack_manipulator;
	friend class constructor_stack_manipulator;
	
	runtime_context(const runtime_context&) = delete;
	void operator=(const runtime_context&) = delete;
private:
	std::vector<variable> _globals;
	variable* _locals;
	variable* _params;
	size_t _params_cnt;
	variable* _dflt;
	const module* _code;
	const variable* _that;
	std::set<std::string>* _constructed;
	variable _retval;
public:
	runtime_context(const module* code, size_t globals_count):
		_globals(globals_count),
		_locals(nullptr),
		_params(nullptr),
		_params_cnt(0),
		_dflt(nullptr),
		_code(code),
		_that(nullptr),
		_constructed(nullptr){
	}
	
	~runtime_context(){
		for(size_t i = _globals.size(); i != 0; --i){
			_globals[i-1].reset();
		}
	}
	
	const module* code(){
		return _code;
	}
	
	variable& local(size_t idx){
		return _locals[idx];
	}
	
	variable& param(size_t idx){
		return idx < _params_cnt ? _params[idx] : _dflt[idx - _params_cnt];
	}
	
	variable& global(size_t idx){
		return _globals[idx];
	}
	
	variable& retval(){
		return _retval;
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
	runtime_context& _ctx;
	variable* _locals;
	variable* _params;
	size_t _params_cnt;
	variable* _dflt;
	const variable* _that;
public:
	function_stack_manipulator(runtime_context& ctx, variable* locals, variable* params, size_t params_cnt, variable* dflt, const variable* that):
		_ctx(ctx),
		_locals(ctx._locals),
		_params(ctx._params),
		_params_cnt(ctx._params_cnt),
		_dflt(ctx._dflt),
		_that(ctx._that){
		
		_ctx._locals = locals;
		_ctx._params = params;
		_ctx._params_cnt = params_cnt;
		_ctx._dflt = dflt;
		_ctx._that = that;
	}
	
	~function_stack_manipulator(){
		_ctx._that = _that;
		_ctx._dflt = _dflt;
		_ctx._params_cnt = _params_cnt;
		_ctx._params = _params;
		_ctx._locals = _locals;
		_ctx.retval().reset();
	}
};

class scope_stack_manipulator{
	scope_stack_manipulator(const scope_stack_manipulator&) = delete;
	void operator=(const scope_stack_manipulator&) = delete;
	runtime_context& _ctx;
	size_t _begin;
	size_t _end;
public:
	scope_stack_manipulator(runtime_context& ctx, size_t begin, size_t end):
		_ctx(ctx),
		_begin(begin),
		_end(end){
	}
	
	~scope_stack_manipulator(){
		for(size_t i = _end; i != _begin; --i){
			_ctx.local(i-1).reset();
		}
	}
};

variable call_function_by_address(code_address addr, runtime_context& ctx, variable* params, size_t params_size);
	
vtable* get_vtable(runtime_context& ctx, std::string name);
vtable* get_vtable(runtime_context& ctx, const variable& v);

}//namespace donkey


#endif /*__runtime_context_hpp__*/


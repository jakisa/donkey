#ifndef __module_hpp__
#define __module_hpp__

#include "function.hpp"
#include "runtime_context.hpp"
#include "statements.hpp"

namespace donkey{

class module final: public code_container{
	module(const module&) = delete;
	void operator=(const module&) = delete;
private:
	std::vector<function> _functions;
	std::unordered_map<std::string, vtable_ptr> _vtables;
	statement _s;
	size_t _globals_count;
public:
	module(statement&& s, int globals_count, std::vector<function>&& functions, std::unordered_map<std::string, vtable_ptr>&& vtables):
		_functions(std::move(functions)),
		_vtables(std::move(vtables)),
		_s(std::move(s)),
		_globals_count(globals_count){
	}
	void load(runtime_context& ctx){
		ctx.stack = std::vector<variable>(_globals_count);
		ctx.function_stack_bottom = ctx.stack.size();
		_s(ctx);
	}
	
	virtual variable call_function_by_address(code_address address, runtime_context& ctx, size_t prms) const override{
		return _functions[address.value](ctx, prms);
	}
	
	virtual vtable* get_vtable(std::string name) const override{
		auto it = _vtables.find(name);
		return it == _vtables.end() ? nullptr : it->second.get();
	}
};

typedef std::shared_ptr<module> module_ptr;


}//donkey

#endif /*__module_hpp__*/


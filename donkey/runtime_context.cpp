#include "runtime_context.hpp"
#include "module.hpp"

namespace donkey{

variable call_function_by_address(code_address addr, runtime_context& ctx, size_t params_size){
	return ctx.code->call_function_by_address(addr, ctx, params_size);
}
	
vtable* get_vtable(runtime_context& ctx, std::string name){
	return ctx.code->get_vtable(name);
}
	
}//donkey;

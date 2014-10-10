#include "runtime_context.hpp"
#include "module.hpp"

namespace donkey{

variable call_function_by_address(code_address addr, runtime_context& ctx, size_t params_size){
	return ctx.code()->call_function_by_address(addr, ctx, params_size);
}
	
vtable* get_vtable(runtime_context& ctx, std::string name){
	return ctx.code()->get_vtable(name);
}

vtable* get_vtable(runtime_context& ctx, const variable& v){
	vtable* vt = v.get_vtable();
	return vt ? vt : get_vtable(ctx, v.get_type_name());
}


void stack_ref_manipulator::_update_reference(size_t i){
	*_byref[i] = std::move(_ctx.top(_vars_cnt - _byref_idx[i] - 1));
}

}//donkey;

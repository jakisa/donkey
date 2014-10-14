#include "runtime_context.hpp"
#include "module.hpp"
#include "module_bundle.hpp"

namespace donkey{

variable call_function_by_address(code_address addr, runtime_context& ctx, size_t params_size){
	return static_cast<module_bundle&>(ctx).call_function_by_address(addr, params_size);
}
	
vtable* get_vtable(runtime_context& ctx, std::string module_name, std::string type_name){
	return static_cast<module_bundle&>(ctx).get_vtable(module_name, type_name);
}

variable& global_variable(runtime_context& ctx, uint32_t module_index, uint32_t var_index){
	return static_cast<module_bundle&>(ctx).global(module_index, var_index);
}

vtable* get_vtable(runtime_context& ctx, const variable& v){
	vtable* vt = v.get_vtable();
	return vt ? vt : get_vtable(ctx, v.get_module_name(), v.get_type_name());
}

}//donkey;

#ifndef __module_bundle_hpp__
#define __module_bundle_hpp__

#include "module.hpp"
#include "runtime_context.hpp"

namespace donkey{

class module_bundle: public runtime_context{
	module_bundle(const module_bundle&) = delete;
	void operator=(const module_bundle&) = delete;
private:
	std::vector<module_ptr> _modules;
	std::unordered_map<std::string, size_t> _modules_map;
	std::vector<variable*> _globals;
	std::unordered_map<std::string, vtable_ptr> _core_vtables;
public:
	module_bundle(size_t sz);
	
	bool module_in_progress(std::string name);
	
	module_ptr get_module(std::string name);
	
	size_t reserve_module(std::string name);
	
	void add_module(std::string name, module_ptr m);
	
	variable call_function_by_address(code_address addr, size_t params_size){
		return _modules[addr.get_module_index()]->call_function_by_index(addr.get_function_index(), *this, params_size);
	}
	
	variable& global(uint32_t module_idx, uint32_t var_idx){
		return _globals[module_idx][var_idx];
	}
	
	vtable* get_core_vtable(const std::string& type_name){
		auto it = _core_vtables.find(type_name);
		return it != _core_vtables.end() ? it->second.get() : nullptr;
	}
	
	vtable* get_vtable(const std::string& module_name, const std::string& type_name){
		return module_name.empty() ? get_core_vtable(type_name) : _modules[_modules_map[module_name]]->get_vtable(type_name);
	}
	
	~module_bundle();
};


}//donkey

#endif

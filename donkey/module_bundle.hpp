#ifndef __module_bundle_hpp__
#define __module_bundle_hpp__

#include "module.hpp"
#include "runtime_context.hpp"

#include <iostream>

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
	module_bundle(size_t sz):
		runtime_context(sz){
		
		vtable_ptr object_vtable = create_object_vtable();
		vtable_ptr string_vtable = create_string_vtable(*object_vtable);
		vtable_ptr number_vtable = create_number_vtable(*object_vtable);
		vtable_ptr null_vtable = create_null_vtable(*object_vtable);
		vtable_ptr function_vtable = create_function_vtable(*object_vtable);
		
		_core_vtables.emplace(object_vtable->get_name(), object_vtable);
		_core_vtables.emplace(string_vtable->get_name(), string_vtable);
		_core_vtables.emplace(number_vtable->get_name(), number_vtable);
		_core_vtables.emplace(null_vtable->get_name(), null_vtable);
		_core_vtables.emplace(function_vtable->get_name(), function_vtable);
	}
	
	size_t reserve_module(std::string name){
		_modules_map[name] = _modules.size();
		_modules.push_back(module_ptr());
		_globals.push_back(nullptr);
		
		return _modules.size() - 1;
	}
	void add_module(std::string name, module_ptr m){
		_modules[_modules_map[name]] = m;
		_globals[_modules_map[name]] = new variable[m->get_globals_count()];
		m->load(*this);
	}
	
	variable call_function_by_address(code_address addr, size_t params_size){
		return _modules[addr.get_module_index()]->call_function_by_index(addr.get_function_index(), *this, params_size);
	}
	
	variable& global(uint32_t module_idx, uint32_t var_idx){
		return _globals[module_idx][var_idx];
	}
	
	vtable* get_vtable(const std::string& module_name, const std::string& type_name){
		return module_name.empty() ? _core_vtables[type_name].get() : _modules[_modules_map[module_name]]->get_vtable(type_name);
	}
	
	~module_bundle(){
		std::cout << _globals[0][0].to_string() << std::endl;
	
		for(size_t i = _modules.size(); i; --i){
			for(size_t j = _modules[i-1]->get_globals_count(); j; --j){
				_globals[i-1][j-1].reset();
			}
			delete[] _globals[i-1];
		}
	}
};


}//donkey

#endif

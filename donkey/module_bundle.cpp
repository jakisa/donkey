#include "module_bundle.hpp"

#include <iostream>

namespace donkey{

module_bundle::module_bundle(size_t sz):
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

bool module_bundle::module_in_progress(std::string name){
	return _modules_map.find(name) != _modules_map.end();
}

module_ptr module_bundle::get_module(std::string name){
	auto it = _modules_map.find(name);
	if(it == _modules_map.end()){
		return module_ptr();
	}
	return _modules[it->second];
}

size_t module_bundle::reserve_module(std::string name){
	_modules_map[name] = _modules.size();
	_modules.push_back(module_ptr());
	_globals.push_back(nullptr);
	
	return _modules.size() - 1;
}
void module_bundle::add_module(std::string name, module_ptr m){
	_modules[_modules_map[name]] = m;
	_globals[_modules_map[name]] = new variable[m->get_globals_count()];
	m->load(*this);
}


module_bundle::~module_bundle(){
	std::cout << _globals[0][0].to_string() << std::endl;
	for(size_t i = 0; i < _modules.size(); ++i){
		for(size_t j = _modules[i]->get_globals_count(); j; --j){
			_globals[i][j-1].reset();
		}
		delete[] _globals[i];
	}
}



}//donkey;

#include "module_bundle.hpp"

#include <algorithm>

namespace donkey{

module_bundle::module_bundle(size_t sz):
	runtime_context(sz){
	
	_core_vtables.emplace(object_vtable()->get_name(), object_vtable());
	_core_vtables.emplace(string_vtable()->get_name(), string_vtable());
	_core_vtables.emplace(number_vtable()->get_name(), number_vtable());
	_core_vtables.emplace(null_vtable()->get_name(), null_vtable());
	_core_vtables.emplace(function_vtable()->get_name(), function_vtable());
	_core_vtables.emplace(array_vtable()->get_name(), array_vtable());
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

void module_bundle::unload_from(size_t idx){
	for(size_t i = idx; i < _modules.size(); ++i){
		if(_modules[i]){
			for(size_t j = _modules[i]->get_globals_count(); j; --j){
				_globals[i][j-1].reset();
			}
			delete[] _globals[i];
			_modules[i].reset();
		}else{
			delete[] _globals[i];
		}
	}
	
	
	for(auto it = _modules_map.begin(); it != _modules_map.end();){
		if(_modules[it->second]){
			++it;
		}else{
			it = _modules_map.erase(it);
		}
	}
	
	_modules.resize(idx);
	_globals.resize(idx);
}

module_bundle::~module_bundle(){
	for(size_t i = 0; i < _modules.size(); ++i){
		for(size_t j = _modules[i]->get_globals_count(); j; --j){
			_globals[i][j-1].reset();
		}
		delete[] _globals[i];
	}
}



}//donkey;

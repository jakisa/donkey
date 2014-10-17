#ifndef __native_module_hpp__
#define __native_module_hpp__

#include "native_object.hpp"
#include "vtable.hpp"
#include "function.hpp"
#include "module.hpp"

#include <unordered_set>
#include <unordered_map>
#include <string>

namespace donkey{


class native_module{
	native_module(const native_module&) = delete;
	void operator=(const native_module&) = delete;
private:
	std::string _name;
	size_t _idx;
	std::unordered_set<std::string> _globals;
	std::unordered_map<std::string, function> _functions;
	std::unordered_map<std::string, vtable_ptr> _vtables;
	statement _init;
public:
	native_module(const std::string& name, size_t idx):
		_name(name),
		_idx(idx),
		_init(&empty_statement){
	}
	
	void set_init(statement&& init){
		_init = std::move(init);
	}
	
	void add_global(std::string global){
		_globals.insert(global);
	}
	
	void add_function(std::string name, function f){
		_functions.emplace(name, f);
	}
	
	void add_vtable(std::string name, vtable_ptr vt){
		_vtables.emplace(name, vt);
	}
	
	
	module_ptr create_module() const{
		std::vector<function> functions;
		std::unordered_map<std::string, size_t> public_functions;
		functions.reserve(_functions.size());
		
		for(const auto& p: _functions){
			public_functions.emplace(p.first, functions.size());
			functions.push_back(p.second);
		}
		
		std::unordered_map<std::string, size_t> public_globals;
		
		for(const std::string& name: _globals){
			public_globals.emplace(name, public_globals.size());
		}
		
		std::unordered_map<std::string, vtable_ptr> vtables = _vtables;
		
		statement init = _init;
		
		return module_ptr(new module(
			std::move(init),
			_name,
			_idx,
			public_globals.size(),
			std::move(functions),
			std::move(vtables),
			std::move(public_functions),
			std::move(public_globals)
		));
	}
};

}//donkey


#endif /*__native_module_hpp__*/

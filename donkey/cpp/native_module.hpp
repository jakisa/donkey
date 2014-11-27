#ifndef __native_module_hpp__
#define __native_module_hpp__

#include "native_object.hpp"
#include "vtable.hpp"
#include "function.hpp"
#include "module.hpp"

#include <unordered_set>
#include <string>

namespace donkey{


class native_module{
	native_module(const native_module&) = delete;
	void operator=(const native_module&) = delete;
private:
	std::string _name;
	size_t _idx;
	std::unordered_map<std::string, size_t> _globals;
	std::unordered_map<std::string, function> _functions;
	std::unordered_map<std::string, vtable_ptr> _vtables;
	std::unordered_map<std::string, identifier_ptr> _constants;
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
	
	size_t add_global(std::string global){
		size_t ret = _globals.size();
		_globals.emplace(global, ret);
		return ret;
	}
	
	void add_function(std::string name, function f){
		_functions.emplace(name, f);
	}
	
	void add_vtable(vtable_ptr vt){
		_vtables.emplace(vt->get_name(), vt);
	}
	
	void add_constant(std::string name, number n){
		_constants[name].reset(new number_constant_identifier(name, n));
	}
	
	void add_constant(std::string name, std::string s){
		_constants[name].reset(new string_constant_identifier(name, s));
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
		
		for(const auto& p: _globals){
			public_globals.emplace(p.first, p.second);
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
			std::move(public_globals),
			std::unordered_map<std::string, identifier_ptr>(_constants)
		));
	}
};

}//donkey


#endif /*__native_module_hpp__*/

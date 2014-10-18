#include "module.hpp"

namespace donkey{

module::module(statement&& s,
	           std::string module_name,
	           size_t module_index,
	           size_t globals_count,
	           std::vector<function>&& functions,
	           std::unordered_map<std::string, vtable_ptr>&& vtables,
	           std::unordered_map<std::string, size_t>&& public_functions,
	           std::unordered_map<std::string, size_t>&& public_globals):
	_functions(std::move(functions)),
	_vtables(std::move(vtables)),
	_s(std::move(s)),
	_module_name(std::move(module_name)),
	_module_index(module_index),
	_globals_count(globals_count),
	_public_functions(std::move(public_functions)),
	_public_globals(std::move(public_globals)){
}

void module::load(runtime_context& ctx){
	try{
		_s(ctx);
	}catch(const runtime_exception& e){
		e.add_stack_trace(_module_name + "::(global)");
	}
}
	
identifier_ptr module::get_identifier(std::string name) const{
	if(name == _module_name){
		return identifier_ptr(new module_identifier(_module_name, *this));
	}
	auto cit = _vtables.find(name);
	if(cit != _vtables.end() && cit->second->is_public()){
		return identifier_ptr(new class_identifier(cit->second->get_name(), cit->second.get(), _module_name));
	}
	
	auto fit = _public_functions.find(name);
	if(fit != _public_functions.end()){
		return identifier_ptr(new function_identifier(fit->first, code_address(_module_index, fit->second)));
	}
	
	auto git = _public_globals.find(name);
	if(git != _public_globals.end()){
		return identifier_ptr(new global_variable_identifier(git->first, _module_index, git->second));
	}
	
	return identifier_ptr();
}


std::vector<identifier_ptr> module::get_all_public() const{
	std::vector<identifier_ptr> ret;
	for(const auto& p: _public_globals){
		ret.push_back(identifier_ptr(new global_variable_identifier(p.first, _module_index, p.second)));
	}
	for(const auto& p: _public_functions){
		ret.push_back(identifier_ptr(new function_identifier(p.first, code_address(_module_index, p.second))));
	}
	for(const auto& p: _vtables){
		if(p.second->is_public()){
			ret.push_back(identifier_ptr(new class_identifier(p.first, p.second.get(), _module_name )));
		}
	}
	return ret;
}


}//donkey

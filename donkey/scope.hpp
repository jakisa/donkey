#ifndef __scope_hpp__
#define __scope_hpp__

#include <unordered_map>
#include <vector>

#include "identifiers.hpp"
#include "statements.hpp"
#include "function.hpp"
#include "vtable.hpp"
#include "tokenizer.hpp"
#include "module_bundle.hpp"

namespace donkey{

class scope: public identifier_lookup{
private:
	std::unordered_map<std::string, identifier_ptr> _variables;
	std::unordered_map<std::string, size_t> _public_variables;
	std::vector<statement> _statements;
	scope* _parent;
	int _var_index;
	size_t _module_index;
	const int _initial_index;
	bool _is_function;
	bool _in_function;
	bool _is_switch;
	bool _can_break;
	bool _can_continue;
	bool _is_class;
	bool _in_class;
public:
	scope(scope* parent, bool is_function = false, bool is_switch = false, bool can_break = false, bool can_continue = false, bool is_class = false):
		_parent(parent),
		_var_index(parent->is_global() ? 0 : parent->_var_index),
		_module_index(_parent->get_module_index()),
		_initial_index(parent->is_global() ? 0 : parent->_var_index),
		_is_function(is_function),
		_in_function(is_function || (_parent->in_function() && !is_class)),
		_is_switch(is_switch),
		_can_break(can_break || (parent->can_break() && !is_class)),
		_can_continue(can_continue || (parent->can_continue() && !is_class)),
		_is_class(is_class),
		_in_class(is_class || _parent->in_class()){
	}
	
	scope(size_t module_index):
		_parent(nullptr),
		_var_index(0),
		_module_index(module_index),
		_initial_index(0),
		_is_function(false),
		_in_function(false),
		_is_switch(false),
		_can_break(false),
		_can_continue(false),
		_is_class(false),
		_in_class(false){
	}
	
	size_t get_module_index() const{
		return _module_index;
	}

	virtual identifier_ptr get_identifier(std::string name) const override{
		auto it = _variables.find(name);
		return it != _variables.end() ? it->second : _parent ? _parent->get_identifier(name) : identifier_ptr();
	}
	
	virtual bool is_allowed(std::string name) const override{
		return _variables.find(name) == _variables.end();
	}
	
	bool is_global() const{
		return !_parent;
	}
	
	bool in_function() const{
		return _in_function;
	}
	
	bool is_switch() const{
		return _is_switch;
	}
	
	bool is_class() const{
		return _is_class;
	}
	
	virtual bool in_class() const override{
		return _in_class;
	}
	
	bool can_break() const{
		return _can_break;
	}
	
	bool can_continue() const{
		return _can_continue;
	}
	
	bool add_variable(std::string name, bool is_public){
		if(_variables.find(name) == _variables.end()){
			if(is_public){
				_public_variables[name] = _var_index;
			}
			if(is_global()){
				_variables[name].reset(new global_variable_identifier(_module_index, _var_index++));
			}else{
				_variables[name].reset(new local_variable_identifier(_var_index++));
			}
			return true;
		}
		return false;
	}
	
	template<typename T>
	void add_statement(T&& s){
		_statements.push_back(statement(s));
	}
	
	size_t get_number_of_statements() const{
		return _statements.size();
	}
	
	statement get_block(){
		size_t vars_cnt = (is_global() || _is_function) ? 0 : get_number_of_variables();
	
		if(vars_cnt == 0){
			if(_statements.empty()){
				return statement(empty_statement);
			}else if(_statements.size() == 1){
				return std::move(_statements.front());
			}else{
				return statement(block_statement(std::move(_statements)));
			}
			return std::move(_statements.front());
		}
		
		return statement(vars_block_statement(std::move(_statements), vars_cnt));
	}
	
	std::vector<statement> get_statements(){
		return std::move(_statements);
	}
	
	size_t get_number_of_variables() const{
		return _var_index - _initial_index;
	}
	
	virtual std::string full_class_name(std::string name) const override{
		return _parent ? _parent->full_class_name(name) : "";
	}
	
	virtual vtable* get_vtable(std::string name) const override{
		return _parent ? _parent->get_vtable(name) : nullptr;
	}
	
	virtual std::string get_current_class() const override{
		return _parent ? _parent->get_current_class() : "";
	}
	
	virtual const std::string& get_module_name() const{
		return _parent->get_module_name();
	}
};

class global_scope: public scope{
private:
	module_bundle& _bundle;
	std::unordered_map<std::string, function_identifier_ptr> _functions;
	std::vector<function> _definitions;
	std::unordered_map<std::string, vtable_ptr> _vtables;
	std::string _module_name;
	
	std::unordered_map<std::string, size_t> _public_functions;
public:
	global_scope(module_bundle& bundle, std::string&& module_name, size_t module_index):
		scope(module_index),
		_bundle(bundle),
		_module_name(module_name){
	}
	bool has_function(std::string name) const{
		auto it = _functions.find(name);
		if(it == _functions.end()){
			return false;
		}
		code_address idx = it->second->get_function();
		return bool(_definitions[idx.get_function_index()]);
	}
	
	void declare_function(std::string name, bool is_public){
		if(_functions.find(name) != _functions.end()){
			return;
		}
		_functions[name].reset(new function_identifier(code_address(get_module_index(), _definitions.size())));
		if(is_public){
			_public_functions[name] = _definitions.size();
		}
		_definitions.push_back(function());
	}
	
	void define_function(std::string name, function&& f){
		function_identifier_ptr& ptr = _functions[name];
		
		if(ptr){
			_definitions[ptr->get_function().get_function_index()] = std::move(f);
		}else{
			ptr.reset(new function_identifier(code_address(get_module_index(), _definitions.size())));
			_definitions.push_back(std::move(f));
		}
	}
	
	std::string get_undefined_function() const{
		for(const auto& p: _functions){
			if(!_definitions[p.second->get_function().get_function_index()]){
				return p.first;
			}
		}
		return "";
	}

	std::vector<function> get_functions(){
		return std::move(_definitions);
	}
	
	std::unordered_map<std::string, vtable_ptr> get_vtables(){
		return std::move(_vtables);
	}
	
	virtual identifier_ptr get_identifier(std::string name) const override{
		auto it = _functions.find(name);
		if(it != _functions.end()){
			return it->second;
		}
		auto cit = _vtables.find(name);
		if(cit == _vtables.end()){
			cit = _vtables.find(_module_name + "::" + name);
		}
		return cit != _vtables.end() ? identifier_ptr(new class_identifier(cit->second->get_full_name())) : scope::get_identifier(name);
	}
	
	virtual bool is_allowed(std::string name) const override{
		auto it = _functions.find(name);
		if(it != _functions.end() && !_definitions[it->second->get_function().get_function_index()] ){
			return false;
		}
		return _vtables.find(_module_name + "::" + name) == _vtables.end() && scope::is_allowed(name);
	}

	bool add_vtable(vtable_ptr vt){
		return _vtables.emplace(vt->get_full_name(), vt).second;
	}
	
	virtual vtable* get_vtable(std::string name) const override{
		if(name == "object"){
			return _bundle.get_vtable("", "object");
		}
		
		auto it = _vtables.find(name);
		return it != _vtables.end() ? it->second.get() : scope::get_vtable(name);
	}
	
	virtual std::string full_class_name(std::string name) const override{
		auto cit = _vtables.find(name);
		if(cit == _vtables.end()){
			cit = _vtables.find(_module_name + "::"+ name);
		}
		return cit == _vtables.end() ? "" : cit->second->get_full_name();
	}
	
	virtual const std::string& get_module_name() const override{
		return _module_name;
	}
};


class class_scope: public scope{
private:
	std::unordered_map<std::string, method_ptr> _methods;
	std::unordered_map<std::string, size_t> _fields;
	method_ptr _constructor;
	method_ptr _destructor;
	std::string _name;
	size_t _fields_size;
	
	static variable variable_strong(variable& that, runtime_context&, size_t){
		return that.non_weak();
	}
	
	static variable variable_weak(variable& that, runtime_context&, size_t){
		return that.non_shared();
	}
public:
	class_scope(std::string name, scope* parent):
		scope(parent, false, false, false, false, true),
		_constructor(new method()),
		_destructor(new method()),
		_name(name),
		_fields_size(0){
	}
	bool has_method(std::string name) const{
		auto it = _methods.find(name);
		return it != _methods.end() && !it->second;
	}
	
	void declare_method(std::string name){
		_methods[name].reset(new method());
	}
	
	void define_method(std::string name, method&& m){
		*(_methods[name]) = std::move(m);
	}
	
	void define_constructor(method&& m){
		*_constructor = std::move(m);
	}
	
	void define_destructor(method&& m){
		*_destructor = std::move(m);
	}
	
	bool add_field(std::string name){
		if(_fields.find(name) == _fields.end()){
			_fields[name] = _fields_size++;
			return true;
		}
		return false;
	}
	
	bool is_allowed_member(std::string name) const{
		auto mit = _methods.find(name);
		if(mit != _methods.end() && mit->second){
			return false;
		}
		
		auto fit = _fields.find(name);
		if(fit != _fields.end()){
			return false;
		}
		return true;
	}
	
	vtable_ptr create_vtable(const std::vector<std::string>& bases, bool is_public) const{
		auto name = _name;
		auto module_name = get_module_name();
		auto methods = _methods;
		auto fields = _fields;
		vtable_ptr ret(new vtable(std::move(module_name), std::move(name), _constructor, _destructor, std::move(methods), std::move(fields), _fields_size, is_public));
		for(const std::string& base: bases){
			ret->derive_from(*get_vtable(base));
		}
		return ret;
	}
	
	virtual identifier_ptr get_identifier(std::string name) const override{
		if(name == _name || name == get_module_name() + "::" + name){
			return identifier_ptr(new class_identifier(get_module_name() + "::" + name));
		}
		return scope::get_identifier(name);
	}
	
	virtual std::string full_class_name(std::string name) const override{
		return name == _name  || name == get_module_name() + "::" + name ? get_module_name() + "::" + name : scope::full_class_name(name);
	}
	
	virtual std::string get_current_class() const override{
		return get_module_name() + "::" + _name;
	}
};

}//donkey

#endif /*__scope_hpp__*/


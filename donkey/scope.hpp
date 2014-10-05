#ifndef __scope_hpp__
#define __scope_hpp__

#include <unordered_map>
#include <vector>

#include "identifiers.hpp"
#include "statements.hpp"
#include "function.hpp"
#include "vtable.hpp"

namespace donkey{

class scope: public identifier_lookup{
private:
	std::unordered_map<std::string, identifier_ptr> _variables;
	std::vector<statement> _statements;
	scope* _parent;
	int _var_index;
	const int _initial_index;
	bool _is_function;
	bool _in_function;
	bool _is_switch;
	bool _can_break;
	bool _can_continue;
public:
	scope(scope* parent, bool is_function = false, bool is_switch = false, bool can_break = false, bool can_continue = false):
		_parent(parent),
		_var_index(parent->is_global() ? 0 : parent->_var_index),
		_initial_index(parent->is_global() ? 0 : parent->_var_index),
		_is_function(is_function),
		_in_function(is_function || _parent->in_function()),
		_is_switch(is_switch),
		_can_break(can_break || parent->can_break()),
		_can_continue(can_continue || parent->can_continue()){
	}
	
	scope():
		_parent(nullptr),
		_var_index(0),
		_initial_index(0),
		_is_function(false),
		_in_function(false),
		_is_switch(false),
		_can_break(false),
		_can_continue(false){
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
	
	bool can_break() const{
		return _can_break;
	}
	
	bool can_continue() const{
		return _can_continue;
	}
	
	bool add_variable(std::string name){
		if(_variables.find(name) == _variables.end()){
			if(is_global()){
				_variables[name].reset(new global_variable_identifier(_var_index++));
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
		if((get_number_of_variables() == 0 || _is_function || is_global()) && _statements.size() < 2){
			if(_statements.empty()){
				return statement(empty_statement);
			}
			return std::move(_statements.front());
		}
		if(is_global() || _is_function){
			return statement(block_statement(std::move(_statements), 0));
		}
		
		return statement(block_statement(std::move(_statements), _var_index - _initial_index));
	}
	
	std::vector<statement> get_statements(){
		return std::move(_statements);
	}
	
	size_t get_number_of_variables() const{
		return _var_index - _initial_index;
	}
};

class global_scope: public scope{
private:
	std::unordered_map<std::string, function_identifier_ptr> _functions;
	std::vector<function> _definitions;
	std::unordered_map<std::string, vtable_ptr> _vtables;
public:
	global_scope(){
		add_vtable("%STRING%", create_string_vtable());
		add_vtable("%NUMBER%", create_number_vtable());
		add_vtable("%NULL%", create_null_vtable());
		add_vtable("%FUNCTION%", create_function_vtable());
	}
	bool has_function(std::string name) const{
		auto it = _functions.find(name);
		if(it == _functions.end()){
			return false;
		}
		code_address idx = it->second->get_function();
		return bool(_definitions[idx.value]);
	}
	
	void declare_function(std::string name){
		if(_functions.find(name) != _functions.end()){
			return;
		}
		_functions[name].reset(new function_identifier(code_address{_definitions.size()}));
		_definitions.push_back(function());
	}
	
	void define_function(std::string name, function&& f){
		function_identifier_ptr& ptr = _functions[name];
		
		if(ptr){
			_definitions[ptr->get_function().value] = std::move(f);
		}else{
			ptr.reset(new function_identifier(code_address{_definitions.size()}));
			_definitions.push_back(std::move(f));
		}
	}
	
	std::string get_undefined_function() const{
		for(const auto& p: _functions){
			if(!_definitions[p.second->get_function().value]){
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
		return it != _functions.end() ? it->second : scope::get_identifier(name);
	}
	
	virtual bool is_allowed(std::string name) const override{
		auto it = _functions.find(name);
		return (it == _functions.end() || !_definitions[it->second->get_function().value] ) && scope::is_allowed(name);
	}

	bool add_vtable(std::string name, vtable_ptr vt){
		return _vtables.emplace(name, vt).second;
	}
};

}//donkey

#endif /*__scope_hpp__*/


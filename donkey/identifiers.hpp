#ifndef __identifiers_hpp__
#define __identifiers_hpp__

#include "runtime_context.hpp"
#include "function.hpp"

namespace donkey{

enum class identifier_type{
	global_variable,
	local_variable,
	function,
	classname,
	module,
};

class identifier{
	identifier(const identifier&) = delete;
	void operator=(const identifier&) = delete;
private:
	identifier_type _t;
protected:
	identifier(identifier_type t):
		_t(t){
	}
public:
	identifier_type get_type() const{
		return _t;
	}
	
	virtual ~identifier(){
	}
};

typedef std::shared_ptr<identifier> identifier_ptr;

class global_variable_identifier: public identifier{
private:
	size_t _module_idx;
	size_t _var_idx;
public:
	global_variable_identifier(size_t module_idx, size_t var_idx):
		identifier(identifier_type::global_variable),
		_module_idx(module_idx),
		_var_idx(var_idx){
	}
	
	size_t get_module_index() const{
		return _module_idx;
	}
	
	size_t get_var_index() const{
		return _var_idx;
	}
};

class local_variable_identifier: public identifier{
private:
	size_t _idx;
public:
	local_variable_identifier(size_t idx):
		identifier(identifier_type::local_variable),
		_idx(idx){
	}
	
	size_t get_index() const{
		return _idx;
	}
};

class function_identifier: public identifier{
private:
	code_address _f;
public:
	function_identifier(code_address f):
		identifier(identifier_type::function),
		_f(f){
	}
	
	const code_address& get_function() const{
		return _f;
	}
};

class class_identifier: public identifier{
private:
	std::string _name;
public:
	class_identifier(const std::string& name):
		identifier(identifier_type::classname),
		_name(name){
	}
	const std::string& get_name() const{
		return _name;
	}
};

class identifier_lookup;

class module_identifier: public identifier{
private:
	const identifier_lookup& _lookup;
public:
	module_identifier(const identifier_lookup& lookup):
		identifier(identifier_type::module),
		_lookup(lookup){
	}
	const identifier_lookup& get_lookup() const{
		return _lookup;
	}
};

typedef std::shared_ptr<function_identifier> function_identifier_ptr;

class vtable;

class identifier_lookup{
	identifier_lookup(const identifier_lookup&) = delete;
	void operator=(const identifier_lookup&) = delete;
protected:
	identifier_lookup(){
	}
public:
	virtual identifier_ptr get_identifier(std::string name) const = 0;
	
	virtual bool is_allowed(std::string name) const = 0;
	
	virtual bool in_class() const = 0;
	
	virtual std::string get_current_class() const = 0;
	
	virtual std::string full_class_name(std::string name) const = 0;
	
	virtual vtable* get_vtable(std::string name) const = 0;
	
	virtual const std::string& get_module_name() const = 0;
	
	virtual ~identifier_lookup(){
	}
};


}//namespace donkey

#endif /* __identifiers_hpp__*/

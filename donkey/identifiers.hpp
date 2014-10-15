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
	method,
	field,
};

class identifier{
	identifier(const identifier&) = delete;
	void operator=(const identifier&) = delete;
private:
	identifier_type _t;
	std::string _name;
protected:
	identifier(identifier_type t, const std::string& name):
		_t(t),
		_name(name){
	}
public:
	identifier_type get_type() const{
		return _t;
	}
	
	const std::string& get_name() const{
		return _name;
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
	global_variable_identifier(const std::string& name, size_t module_idx, size_t var_idx):
		identifier(identifier_type::global_variable, name),
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
	local_variable_identifier(const std::string& name, size_t idx):
		identifier(identifier_type::local_variable, name),
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
	function_identifier(const std::string& name, code_address f):
		identifier(identifier_type::function, name),
		_f(f){
	}
	
	const code_address& get_function() const{
		return _f;
	}
};

typedef std::shared_ptr<function_identifier> function_identifier_ptr;

class vtable;

class class_identifier: public identifier{
private:
	vtable* _vt;
	std::string _module_name;
public:
	class_identifier(const std::string& name, vtable* vt, const std::string& module_name):
		identifier(identifier_type::classname, name),
		_vt(vt),
		_module_name(module_name){
	}
	vtable* get_vtable() const{
		return _vt;
	}
	const std::string& get_module_name(){
		return _module_name;
	}
};

class identifier_lookup;

class module_identifier: public identifier{
private:
	const identifier_lookup& _lookup;
public:
	module_identifier(const std::string& name, const identifier_lookup& lookup):
		identifier(identifier_type::module, name),
		_lookup(lookup){
	}
	const identifier_lookup& get_lookup() const{
		return _lookup;
	}
};

typedef std::shared_ptr<module_identifier> module_identifier_ptr;

class method_identifier: public identifier{
private:
	method& _m;
	std::string _classname;
public:
	method_identifier(const std::string& name, method& m, std::string classname):
		identifier(identifier_type::method, name),
		_m(m),
		_classname(classname){
	}
	
	method& get_method() const{
		return _m;
	}
	
	const std::string& get_classname() const{
		return _classname;
	}
};

class field_identifier: public identifier{
private:
	size_t _f;
	std::string _classname;
public:
	field_identifier(const std::string& name, size_t f, std::string classname):
		identifier(identifier_type::field, name),
		_f(f),
		_classname(classname){
	}
	
	size_t get_field() const{
		return _f;
	}
	
	const std::string& get_classname() const{
		return _classname;
	}
};

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
	
	virtual vtable* get_vtable(std::string module, std::string name) const = 0;
	
	virtual const std::string& get_module_name() const = 0;
	
	virtual ~identifier_lookup(){
	}
};


}//namespace donkey

#endif /* __identifiers_hpp__*/

#ifndef __vtable_h__
#define __vtable_h__

#include "function.hpp"
#include "errors.hpp"
#include "variables.hpp"
#include "runtime_context.hpp"

#include <unordered_map>

namespace donkey{

class vtable;

struct base_class{
	const vtable* vt;
	size_t data_begin;
};

class vtable{
	void operator=(const vtable&) = delete;
private:
	std::string _name;
	std::unordered_map<std::string, base_class> _bases;
	std::unordered_map<std::string, method_ptr> _methods;
	std::unordered_map<std::string, size_t> _fields;
	method_ptr _constructor;
	size_t _fields_size;
	

	
	variable call_field(variable that, runtime_context& ctx, size_t params_size, const std::string& name) const{
		auto it = _fields.find(name);
		if(it == _fields.end()){
			runtime_error("method " + name + " is not defined for " + _name);
		}
		return that.nth_field(it->second).call(ctx, params_size);
	}
	
public:
	vtable(std::string&& name, method_ptr constructor, std::unordered_map<std::string, method_ptr>&& methods, std::unordered_map<std::string, size_t>&& fields, size_t fields_size):
		_name(std::move(name)),
		_methods(std::move(methods)),
		_fields(std::move(fields)),
		_constructor(constructor),
		_fields_size(fields_size){
	}
	
	void call_base_constructor(variable& that, runtime_context& ctx, size_t params_size) const{
		if(_constructor && !ctx.is_constructed(_name)){
			(*_constructor)(that, ctx, params_size);
			ctx.set_constructed(_name);
		}
	}
	
	void call_constructor(variable& that, runtime_context& ctx, size_t params_size) const{
		if(_constructor){
			constructor_stack_manipulator _(ctx);
			(*_constructor)(that, ctx, params_size);
		}
	}

	variable call_member(variable& that, runtime_context& ctx, size_t params_size, const std::string& name) const{
		auto it = _methods.find(name);
		if(it == _methods.end()){
			return call_field(that, ctx, params_size, name);
		}
		return (*it->second)(that, ctx, params_size);
	}
	
	variable& get_field(variable& that, const std::string& name) const{
		auto it = _fields.find(name);
		if(it == _fields.end()){
			runtime_error("field " + name + " is not defined for " + _name);
		}
		return that.nth_field(it->second);
	}
	
	variable call_method(variable& that, runtime_context& ctx, size_t params_size, const std::string& type, method& m){
		if(_name != type && _bases.find(type) == _bases.end()){
			runtime_error(_name + " is not derived from " + type);
		}
		
		return m(that, ctx, params_size);
	}
	
	variable& get_field(variable& that, const std::string& type, size_t idx){
		if(type == _name){
			return that.nth_field(idx);
		}
		auto it = _bases.find(type);
		if(it == _bases.end()){
			runtime_error(_name + " is not derived from " + type);
		}
		
		return that.nth_field(it->second.data_begin + idx);
	}
	
	bool has_member(const std::string& name) const{
		return has_method(name) || has_field(name);
	}
	
	bool has_method(const std::string& name) const{
		return _methods.find(name) != _methods.end();
	}
	
	bool has_field(const std::string& name) const{
		return _fields.find(name) != _fields.end();
	}
	
	method_ptr get_method(const std::string& name) const{
		return _methods.find(name)->second;
	}
	
	size_t get_field_index(const std::string& name) const{
		return _fields.find(name)->second;
	}
	
	size_t get_fields_size() const{
		return _fields_size;
	}
	
	void derive_from(const vtable& base){
		if(_bases.find(base._name) != _bases.end()){
			return;
		}
		
		size_t base_begin = _fields_size;
		
		_bases[base._name] = base_class{&base, base_begin};
		
		for(const auto& p: base._bases){
			_bases.insert(std::unordered_map<std::string, base_class>::value_type(p.first, base_class{p.second.vt, base_begin + p.second.data_begin}));
		}
		
		_fields_size += base._fields_size;
		
		for(const auto& p: base._methods){
			if(_methods.find(p.first) != _methods.end()){
				continue;
			}
			if(_fields.find(p.first) != _fields.end()){
				continue;
			}
			_methods[p.first] = p.second;
		}
		
		for(const auto& p: base._fields){
			if(_methods.find(p.first) != _methods.end()){
				continue;
			}
			if(_fields.find(p.first) != _fields.end()){
				continue;
			}
			
			_fields[p.first] = base_begin + p.second;
		}
	}
	
	const std::string& get_name() const{
		return _name;
	}
	
	std::vector<const vtable*> get_bases() const{
		std::vector<const vtable*> ret;
		for(const auto& p: _bases){
			ret.push_back(p.second.vt);
		}
		return ret;
	}
};

typedef std::shared_ptr<vtable> vtable_ptr;

vtable_ptr create_object_vtable();
vtable_ptr create_string_vtable(const vtable& base);
vtable_ptr create_number_vtable(const vtable& base);
vtable_ptr create_function_vtable(const vtable& base);
vtable_ptr create_null_vtable(const vtable& base);

}//donkey

#endif /*__vtable_h__*/

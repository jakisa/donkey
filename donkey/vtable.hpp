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
	std::string _full_name;
	std::string _module_name;
	std::string _name;
	std::unordered_map<std::string, base_class> _bases;
	std::unordered_map<std::string, method_ptr> _methods;
	std::unordered_map<std::string, size_t> _fields;
	method_ptr _constructor;
	method_ptr _destructor;
	size_t _fields_size;
	bool _is_public;
	bool _is_final;

	
	variable call_field(const variable& that, runtime_context& ctx, size_t params_size, const std::string& name) const{
		auto it = _fields.find(name);
		if(it == _fields.end()){
			runtime_error("method " + name + " is not defined for " + _full_name);
		}
		return that.nth_field(it->second).call(ctx, params_size);
	}
	
public:
	vtable(std::string&& module_name, std::string&& name, method_ptr constructor, method_ptr destructor,
	       std::unordered_map<std::string, method_ptr>&& methods, std::unordered_map<std::string, size_t>&& fields,
	       size_t fields_size, bool is_public, bool is_final):
		_full_name(module_name.empty() ? name : module_name + "::" + name),
		_module_name(std::move(module_name)),
		_name(std::move(name)),
		_methods(std::move(methods)),
		_fields(std::move(fields)),
		_constructor(constructor),
		_destructor(destructor),
		_fields_size(fields_size),
		_is_public(is_public),
		_is_final(is_final){
	}
	
	void call_base_constructor(const variable& that, runtime_context& ctx, size_t params_size) const{
		if(_constructor && !ctx.is_constructed(_full_name)){
			(*_constructor)(that, ctx, params_size);
			ctx.set_constructed(_full_name);
		}
	}
	
	void call_constructor(const variable& that, runtime_context& ctx, size_t params_size) const{
		if(_constructor){
			constructor_stack_manipulator _(ctx);
			(*_constructor)(that, ctx, params_size);
		}
	}
	
	void call_base_destructor(const variable& that, runtime_context& ctx) const{
		if(_destructor && !ctx.is_destroyed(_full_name)){
			(*_destructor)(that, ctx, 0);
			ctx.set_destroyed(_full_name);
		}
	}
	
	void call_destructor(const variable& that, runtime_context& ctx) const{
		if(_destructor){
			destructor_stack_manipulator _(ctx);
			(*_destructor)(that, ctx, 0);
		}
	}

	variable call_member(const variable& that, runtime_context& ctx, size_t params_size, const std::string& name) const{
		auto it = _methods.find(name);
		if(it == _methods.end()){
			return call_field(that, ctx, params_size, name);
		}
		return (*it->second)(that, ctx, params_size);
	}
	
	variable& get_field(const variable& that, const std::string& name) const{
		auto it = _fields.find(name);
		if(it == _fields.end()){
			runtime_error("field " + name + " is not defined for " + _full_name);
		}
		return that.nth_field(it->second);
	}
	
	variable call_method(const variable& that, runtime_context& ctx, size_t params_size, const std::string& type, method& m){
		if(_full_name != type && _bases.find(type) == _bases.end()){
			runtime_error(_full_name + " is not derived from " + type);
		}
		
		return m(that, ctx, params_size);
	}
	
	variable& get_field(const variable& that, const std::string& type, size_t idx){
		if(type == _full_name){
			return that.nth_field(idx);
		}
		auto it = _bases.find(type);
		if(it == _bases.end()){
			runtime_error(_full_name + " is not derived from " + type);
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
		if(_bases.find(base._full_name) != _bases.end()){
			return;
		}
		
		size_t base_begin = _fields_size;
		
		_bases[base._full_name] = base_class{&base, base_begin};
		
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
	
	const std::string& get_module_name() const{
		return _module_name;
	}
	
	const std::string& get_full_name() const{
		return _full_name;
	}
	
	std::vector<const vtable*> get_bases() const{
		std::vector<const vtable*> ret;
		for(const auto& p: _bases){
			ret.push_back(p.second.vt);
		}
		return ret;
	}
	
	bool is_public() const{
		return _is_public;
	}
	
	bool is_final() const{
		return _is_final;
	}
};

typedef std::shared_ptr<vtable> vtable_ptr;

vtable_ptr object_vtable();
vtable_ptr string_vtable();
vtable_ptr number_vtable();
vtable_ptr function_vtable();
vtable_ptr null_vtable();

}//donkey

#endif /*__vtable_h__*/

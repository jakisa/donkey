#include "vtable.hpp"

namespace donkey{

variable vtable::call_field(const variable& that, runtime_context& ctx, size_t params_size, const std::string& name) const{
	auto it = _fields.find(name);
	if(it == _fields.end()){
		runtime_error("method " + name + " is not defined for " + _full_name);
	}
	return that.nth_field(it->second).call(ctx, params_size);
}

void vtable::update_getter(){
	if(_getter){
		return;
	}
	auto it = _methods.find("opGet");
	if(it != _methods.end()){
		_getter = it->second;
	}
}

void vtable::update_setter(){
	if(_setter){
		return;
	}
	auto it = _methods.find("opSet");
	if(it != _methods.end()){
		_setter = it->second;
	}
}

variable vtable::call_member(const variable& that, runtime_context& ctx, size_t params_size, const std::string& name) const{
	auto it = _methods.find(name);
	if(it == _methods.end()){
		return call_field(that, ctx, params_size, name);
	}
	return (*it->second)(that, ctx, params_size);
}

bool vtable::try_call_member(const variable& that, runtime_context& ctx, size_t params_size, const std::string& name, variable& rslt) const{
	auto mit = _methods.find(name);
	if(mit != _methods.end()){
		rslt = (*mit->second)(that, ctx, params_size);
		return true;
	}
	
	auto fit = _fields.find(name);
	if(fit != _fields.end()){
		rslt = that.nth_field(fit->second).call(ctx, params_size);
		return true;
	}
	
	return false;
}

variable& vtable::get_field(const variable& that, const std::string& name) const{
	auto it = _fields.find(name);
	if(it == _fields.end()){
		runtime_error("field " + name + " is not defined for " + _full_name);
	}
	return that.nth_field(it->second);
}

void vtable::derive_from(const vtable& base){
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
	
	update_getter();
	update_setter();
}

std::vector<const vtable*> vtable::get_bases() const{
	std::vector<const vtable*> ret;
	for(const auto& p: _bases){
		ret.push_back(p.second.vt);
	}
	return ret;
}

variable vtable::create(runtime_context& ctx, size_t params_size) const{
	if(_creator){
		return _creator(ctx, params_size);
	}
	return variable();
}

void vtable::call_base_constructor(const variable& that, runtime_context& ctx, size_t params_size) const{
	if(_constructor && !ctx.is_constructed(_full_name)){
		(*_constructor)(that, ctx, params_size);
		ctx.set_constructed(_full_name);
	}
}

void vtable::call_constructor(const variable& that, runtime_context& ctx, size_t params_size) const{
	if(_constructor){
		constructor_stack_manipulator _(ctx);
		(*_constructor)(that, ctx, params_size);
	}
}

void vtable::call_base_destructor(const variable& that, runtime_context& ctx) const{
	if(_destructor && !ctx.is_destroyed(_full_name)){
		(*_destructor)(that, ctx, 0);
		ctx.set_destroyed(_full_name);
	}
}

void vtable::call_destructor(const variable& that, runtime_context& ctx) const{
	if(_destructor){
		destructor_stack_manipulator _(ctx);
		(*_destructor)(that, ctx, 0);
	}
}


}//donkey

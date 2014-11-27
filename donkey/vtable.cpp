#include "vtable.hpp"

namespace donkey{

#define UPDATE_METHOD(name)\
if(!name){\
	auto it = _methods.find(#name);\
	if(it != _methods.end()){\
		name = it->second.get();\
	}\
	if(_fields.find(#name) != _fields.end()){\
		semantic_error(#name "cannot be declared as field");\
	}\
}

void vtable::update_predefined_methods(){
	UPDATE_METHOD(opGet) UPDATE_METHOD(opSet) UPDATE_METHOD(opCall)
	UPDATE_METHOD(opEQ) UPDATE_METHOD(opNE)
	UPDATE_METHOD(opLT) UPDATE_METHOD(opGT) UPDATE_METHOD(opLE) UPDATE_METHOD(opGE)
	UPDATE_METHOD(opLTInv) UPDATE_METHOD(opGTInv) UPDATE_METHOD(opLEInv) UPDATE_METHOD(opGEInv)
	UPDATE_METHOD(opNot)
	UPDATE_METHOD(opAnd) UPDATE_METHOD(opOr) UPDATE_METHOD(opXor)
	UPDATE_METHOD(opAndInv) UPDATE_METHOD(opOrInv) UPDATE_METHOD(opXorInv)
	UPDATE_METHOD(opAndSet) UPDATE_METHOD(opOrSet) UPDATE_METHOD(opXorSet)
	UPDATE_METHOD(opPreInc) UPDATE_METHOD(opPreDec)
	UPDATE_METHOD(opPostInc) UPDATE_METHOD(opPostDec)
	UPDATE_METHOD(opPos) UPDATE_METHOD(opNeg)
	UPDATE_METHOD(opMul) UPDATE_METHOD(opDiv) UPDATE_METHOD(opIDiv) UPDATE_METHOD(opMod) UPDATE_METHOD(opAdd) UPDATE_METHOD(opSub) UPDATE_METHOD(opSL) UPDATE_METHOD(opSR)
	UPDATE_METHOD(opMulInv) UPDATE_METHOD(opDivInv) UPDATE_METHOD(opIDivInv) UPDATE_METHOD(opModInv) UPDATE_METHOD(opAddInv) UPDATE_METHOD(opSubInv) UPDATE_METHOD(opSLInv) UPDATE_METHOD(opSRInv)
	UPDATE_METHOD(opMulSet) UPDATE_METHOD(opDivSet) UPDATE_METHOD(opModSet) UPDATE_METHOD(opAddSet) UPDATE_METHOD(opSubSet) UPDATE_METHOD(opSLSet) UPDATE_METHOD(opSRSet)
	UPDATE_METHOD(clone) UPDATE_METHOD(strong) UPDATE_METHOD(weak) UPDATE_METHOD(toString) UPDATE_METHOD(toBool)
}

#undef UPDATE_METHOD

vtable::vtable(std::string&& module_name, std::string&& name, method_ptr constructor, method_ptr destructor,
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
	_is_final(is_final),
	_is_native(false){
	
	opGet=opSet=opCall=
	opEQ=opNE=
	opLT=opGT=opLE=opGE=
	opLTInv=opGTInv=opLEInv=opGEInv=
	opNot=
	opAnd=opOr=opXor=
	opAndInv=opOrInv=opXorInv=
	opAndSet=opOrSet=opXorSet=
	opPreInc=opPreDec=
	opPostInc=opPostDec=
	opPos=opNeg=
	opMul=opDiv=opIDiv=opMod=opAdd=opSub=opSL=opSR=
	opMulInv=opDivInv=opIDivInv=opModInv=opAddInv=opSubInv=opSLInv=opSRInv=
	opMulSet=opDivSet=opModSet=opAddSet=opSubSet=opSLSet=opSRSet = nullptr;
	
	clone=strong=weak=toString=toBool = nullptr;
	
	update_predefined_methods();
}

vtable::vtable(std::string&& module_name, std::string&& name, function creator, std::unordered_map<std::string, method_ptr>&& methods, bool is_public):
	_full_name(module_name.empty() ? name : module_name + "::" + name),
	_module_name(std::move(module_name)),
	_name(std::move(name)),
	_methods(std::move(methods)),
	_fields_size(0),
	_is_public(is_public),
	_is_final(true),
	_is_native(true),
	_creator(creator){
	
	opGet=opSet=opCall=
	opEQ=opNE=
	opLT=opGT=opLE=opGE=
	opLTInv=opGTInv=opLEInv=opGEInv=
	opNot=
	opAnd=opOr=opXor=
	opAndInv=opOrInv=opXorInv=
	opAndSet=opOrSet=opXorSet=
	opPreInc=opPreDec=
	opPostInc=opPostDec=
	opPos=opNeg=
	opMul=opDiv=opIDiv=opMod=opAdd=opSub=opSL=opSR=
	opMulInv=opDivInv=opIDivInv=opModInv=opAddInv=opSubInv=opSLInv=opSRInv=
	opMulSet=opDivSet=opModSet=opAddSet=opSubSet=opSLSet=opSRSet = nullptr;
	
	clone=strong=weak=toString=toBool = nullptr;
	
	update_predefined_methods();
}

variable vtable::call_field(const variable& that, runtime_context& ctx, size_t params_size, const std::string& name) const{
	auto it = _fields.find(name);
	if(it == _fields.end()){
		runtime_error("method " + name + " is not defined for " + _full_name);
	}
	return that.nth_field(it->second).call(ctx, params_size);
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
	
	update_predefined_methods();
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

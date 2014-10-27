#include "vtable.hpp"
#include "cpp/native_function.hpp"

#include <memory>

namespace donkey{

static variable object_to_string(const variable& that, runtime_context&, size_t){
	return variable(that.get_full_type_name());
}

static variable object_strong(const variable& that, runtime_context&, size_t){
	return that.non_weak();
}

static variable object_weak(const variable& that, runtime_context&, size_t){
	return that.non_shared();
}

static variable object_clone(const variable& that, runtime_context&, size_t){
	return that;
}

static variable object_equals(const variable& that, variable oth){
	var_type dt = that.get_data_type();
	
	if(dt != oth.get_data_type()){
		return variable(number(0));
	}
	
	switch(dt){
		case var_type::code_address:
			return variable(that.as_code_address_unsafe() == oth.as_code_address_unsafe());
		case var_type::nothing:
			return variable(1);
		default:
			return variable(that.as_reference_unsafe() == oth.as_reference_unsafe());
	}
}

static variable object_not_equals(const variable& that, variable oth){
	var_type dt = that.get_data_type();
	
	if(dt != oth.get_data_type()){
		return variable(number(1));
	}
	
	switch(dt){
		case var_type::code_address:
			return variable(that.as_code_address_unsafe() != oth.as_code_address_unsafe());
		case var_type::nothing:
			return variable(number(0));
		default:
			return variable(that.as_reference_unsafe() != oth.as_reference_unsafe());
	}
}


vtable_ptr object_vtable(){
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		methods.emplace("toString", method_ptr(new method(&object_to_string)));
		
		methods.emplace("strong", method_ptr(new method(&object_strong)));
		
		methods.emplace("weak", method_ptr(new method(&object_weak)));
		
		methods.emplace("clone", method_ptr(new method(&object_clone)));
		
		methods.emplace("opEQ", create_native_method("object::opEQ", &object_equals));
		
		methods.emplace("opNE", create_native_method("object::opNE", &object_not_equals));
		
		return new vtable("", "object", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0, true, false);
	}());
	return ret;
}

static variable number_to_bool(const variable& that, runtime_context&, size_t){
	return variable(that.as_number_unsafe() != 0);
}

static variable number_to_string(const variable& that, runtime_context&, size_t){
	return variable(donkey::to_string(that.as_number_unsafe()));
}

vtable_ptr number_vtable(){
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		methods.emplace("toBool", method_ptr(new method(&number_to_bool)));
		methods.emplace("toString", method_ptr(new method(&number_to_string)));
		
		vtable* vt = new vtable("", "number", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0, true, true);
		vt->derive_from(*object_vtable());
		return vt;
	}());
	return ret;
}

vtable_ptr function_vtable(){
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		vtable* vt = new vtable("", "function", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0, true, true);
		vt->derive_from(*object_vtable());
		return vt;
	}());
	return ret;
}

vtable_ptr null_vtable(){
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		vtable* vt = new vtable("", "null", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0, true, true);
		vt->derive_from(*object_vtable());
		return vt;
	}());
	return ret;
}


}//donkey




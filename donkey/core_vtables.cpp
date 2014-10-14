#include "vtable.hpp"
#include "native_function.hpp"

#include <memory>

namespace donkey{

static integer string_length(std::string that){
	return that.size();
}

static std::string string_substr(std::string that, integer pos, integer len){
	if((size_t)pos >= that.size()){
		return "";
	}
	return that.substr(pos, len);
}

static variable string_to_string(const variable& that, runtime_context&, size_t){
	return that;
}

static variable variable_to_string(const variable& that, runtime_context&, size_t){
	return variable(that.to_string());
}

static variable variable_strong(const variable& that, runtime_context&, size_t){
	return that.non_weak();
}

static variable variable_weak(const variable& that, runtime_context&, size_t){
	return that.non_shared();
}

vtable_ptr create_object_vtable(){
	std::unordered_map<std::string, method_ptr> methods;
	std::unordered_map<std::string, size_t> fields;
	
	methods.emplace("toString", method_ptr(new method(&variable_to_string)));
	
	methods.emplace("strong", method_ptr(new method(&variable_strong)));
	
	methods.emplace("weak", method_ptr(new method(&variable_weak)));
	
	return vtable_ptr(new vtable("", "object", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0));
}

vtable_ptr create_string_vtable(const vtable& base){
	std::unordered_map<std::string, method_ptr> methods;
	std::unordered_map<std::string, size_t> fields;
	
	methods.emplace("length", create_native_method(&string_length));
	
	methods.emplace("substr", create_native_method(&string_substr, std::make_tuple(0, integer(std::string::npos))));
	
	methods.emplace("toString", method_ptr(new method(&string_to_string)));
	
	vtable_ptr ret(new vtable("", "string", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0));
	ret->derive_from(base);
	return ret;
}

vtable_ptr create_number_vtable(const vtable& base){
	std::unordered_map<std::string, method_ptr> methods;
	std::unordered_map<std::string, size_t> fields;
	
	vtable_ptr ret(new vtable("", "number", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0));
	ret->derive_from(base);
	return ret;
}

vtable_ptr create_function_vtable(const vtable& base){
	std::unordered_map<std::string, method_ptr> methods;
	std::unordered_map<std::string, size_t> fields;
	
	vtable_ptr ret(new vtable("", "function", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0));
	ret->derive_from(base);
	return ret;
}

vtable_ptr create_null_vtable(const vtable& base){
	std::unordered_map<std::string, method_ptr> methods;
	std::unordered_map<std::string, size_t> fields;
	
	vtable_ptr ret(new vtable("", "null", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0));
	ret->derive_from(base);
	return ret;
}

}//donkey




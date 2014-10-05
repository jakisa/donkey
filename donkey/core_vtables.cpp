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

static variable string_to_string(variable& that, runtime_context&, size_t){
	return that;
}

static variable variable_to_string(variable& that, runtime_context&, size_t){
	return variable(that.to_string());
}

vtable_ptr create_string_vtable(){
	std::unordered_map<std::string, method> methods;
	std::unordered_map<std::string, size_t> fields;
	
	methods.emplace("length", create_native_method(&string_length));
	
	methods.emplace("substr", create_native_method(&string_substr, std::make_tuple(0, integer(std::string::npos))));
	
	methods.emplace("toString", &string_to_string);
	
	return vtable_ptr(new vtable(std::move(methods), std::move(fields)));
}

vtable_ptr create_number_vtable(){
	std::unordered_map<std::string, method> methods;
	std::unordered_map<std::string, size_t> fields;
	
	methods.emplace("toString", &variable_to_string);
	
	return vtable_ptr(new vtable(std::move(methods), std::move(fields)));
}

vtable_ptr create_function_vtable(){
	std::unordered_map<std::string, method> methods;
	std::unordered_map<std::string, size_t> fields;
	
	methods.emplace("toString", &variable_to_string);
	
	return vtable_ptr(new vtable(std::move(methods), std::move(fields)));
}

vtable_ptr create_null_vtable(){
	std::unordered_map<std::string, method> methods;
	std::unordered_map<std::string, size_t> fields;
	
	methods.emplace("toString", &variable_to_string);
	
	return vtable_ptr(new vtable(std::move(methods), std::move(fields)));
}

}//donkey




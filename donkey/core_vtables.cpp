#include "vtable.hpp"
#include "native_function.hpp"

#include <memory>

namespace donkey{

static int string_length(std::string that){
	return that.size();
}

static std::string string_substr(std::string that, int pos, int len){
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
	
	methods.emplace("length", method(native_method<int(std::string)>(&string_length)));
	
	methods.emplace("substr", method(native_method<std::string(std::string, int, int)>(&string_substr, std::make_tuple(0, std::string::npos))));
	
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




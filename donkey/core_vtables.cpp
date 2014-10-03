#include "vtable.hpp"
#include "native_function.hpp"

#include <memory>

namespace donkey{

static int string_length(std::string that){
	return that.size();
}

static int test_f(std::string, std::string s, double d, int i){
	printf("%s %f %d\n", s.c_str(), d, i);
	return 42;
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
	
	methods.emplace("testF", method(native_method<int(std::string, std::string, double, int)>(&test_f)));
	
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




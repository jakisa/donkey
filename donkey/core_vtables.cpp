#include "vtable.hpp"
#include "cpp/native_function.hpp"

#include <memory>

namespace donkey{

static variable variable_to_string(const variable& that, runtime_context&, size_t){
	return variable(that.to_string());
}

static variable variable_strong(const variable& that, runtime_context&, size_t){
	return that.non_weak();
}

static variable variable_weak(const variable& that, runtime_context&, size_t){
	return that.non_shared();
}



vtable_ptr object_vtable(){
	static vtable_ptr ret;
	if(!ret){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		methods.emplace("toString", method_ptr(new method(&variable_to_string)));
		
		methods.emplace("strong", method_ptr(new method(&variable_strong)));
		
		methods.emplace("weak", method_ptr(new method(&variable_weak)));
		
		ret.reset(new vtable("", "object", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0, true, false));
	}
	return ret;
}

vtable_ptr number_vtable(){
	static vtable_ptr ret;
	if(!ret){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		ret.reset(new vtable("", "number", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0, true, true));
		ret->derive_from(*object_vtable());
	}
	return ret;
}

vtable_ptr function_vtable(){
	static vtable_ptr ret;
	if(!ret){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		ret.reset(new vtable("", "function", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0, true, true));
		ret->derive_from(*object_vtable());
	}
	return ret;
}

vtable_ptr null_vtable(){
	static vtable_ptr ret;
	if(!ret){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		ret.reset(new vtable("", "null", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0, true, true));
		ret->derive_from(*object_vtable());
	}
	return ret;
}


}//donkey




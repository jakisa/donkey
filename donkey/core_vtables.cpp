#include "vtable.hpp"
#include "cpp/native_function.hpp"

#include <memory>

namespace donkey{

static variable object_to_string(const variable& that, runtime_context&, size_t){
	return variable(that.to_string());
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


vtable_ptr object_vtable(){
	static vtable_ptr ret;
	if(!ret){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		methods.emplace("toString", method_ptr(new method(&object_to_string)));
		
		methods.emplace("strong", method_ptr(new method(&object_strong)));
		
		methods.emplace("weak", method_ptr(new method(&object_weak)));
		
		methods.emplace("clone", method_ptr(new method(&object_clone)));
		
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




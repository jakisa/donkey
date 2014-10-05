#ifndef __vtable_h__
#define __vtable_h__

#include "function.hpp"
#include "errors.hpp"

#include <unordered_map>

namespace donkey{

class vtable{
	void operator=(const vtable&) = delete;
private:
	std::unordered_map<std::string, method> _methods;
	std::unordered_map<std::string, size_t> _fields;
	
	variable _call_method(variable that, runtime_context& ctx, size_t params_count, const std::string& name) const{
		auto it = _methods.find(name);
		if(it == _methods.end()){
			if(name == "toString"){
				return variable(that.to_string());
			}
			runtime_error("method " + name + " is not defined for " + that.get_type_name());
		}
		return it->second(that, ctx, params_count);
	}
	
	variable& _get_field(variable& that, const std::string& name) const{
		auto it = _fields.find(name);
		if(it == _fields.end()){
			runtime_error("field " + name + " is not defined for " + that.get_type_name());
		}
		return that.nth_field(it->second);
	}
	
public:
	vtable(std::unordered_map<std::string, method>&& methods, std::unordered_map<std::string, size_t>&& fields):
		_methods(std::move(methods)),
		_fields(std::move(fields)){
	}
	
	static variable& get_field(variable &that, runtime_context& ctx, const std::string &name){
		return get_vtable(ctx, that.get_vtable_name())->_get_field(that, name);
	}
	
	static variable call_method(variable that, runtime_context &ctx, size_t params_count, const std::string &name){
		return get_vtable(ctx, that.get_vtable_name())->_call_method(that, ctx, params_count, name);
	}
};

typedef std::shared_ptr<vtable> vtable_ptr;

vtable_ptr create_string_vtable();
vtable_ptr create_number_vtable();
vtable_ptr create_function_vtable();
vtable_ptr create_null_vtable();

}//donkey

#endif /*__vtable_h__*/

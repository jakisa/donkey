#ifndef __vtable_h__
#define __vtable_h__

#include "function.hpp"
#include "errors.hpp"
#include "variables.hpp"
#include "runtime_context.hpp"

#include <unordered_map>

namespace donkey{


class vtable{
	void operator=(const vtable&) = delete;
private:
	std::unordered_map<std::string, method> _methods;
	std::unordered_map<std::string, size_t> _fields;
	
	variable call_field(variable that, runtime_context& ctx, size_t params_size, const std::string& name) const{
		auto it = _fields.find(name);
		if(it == _fields.end()){
			if(name == "toString"){
				return variable(that.to_string());
			}
			runtime_error("method " + name + " is not defined for " + that.get_type_name());
		}
		return that.nth_field(it->second).call(ctx, params_size);
	}
	
public:
	vtable(std::unordered_map<std::string, method>&& methods, std::unordered_map<std::string, size_t>&& fields):
		_methods(std::move(methods)),
		_fields(std::move(fields)){
	}

	variable call_member(variable that, runtime_context& ctx, size_t params_size, const std::string& name) const{
		auto it = _methods.find(name);
		if(it == _methods.end()){
			return call_field(that, ctx, params_size, name);
		}
		return it->second(that, ctx, params_size);
	}
	
	variable& get_field(variable& that, const std::string& name) const{
		auto it = _fields.find(name);
		if(it == _fields.end()){
			runtime_error("field " + name + " is not defined for " + that.get_type_name());
		}
		return that.nth_field(it->second);
	}
	
	bool has_method(const std::string& name) const{
		return _methods.find(name) != _methods.end();
	}
	
	size_t get_fields_size() const{
		return _fields.size();
	}
};

typedef std::shared_ptr<vtable> vtable_ptr;

vtable_ptr create_string_vtable();
vtable_ptr create_number_vtable();
vtable_ptr create_function_vtable();
vtable_ptr create_null_vtable();

}//donkey

#endif /*__vtable_h__*/

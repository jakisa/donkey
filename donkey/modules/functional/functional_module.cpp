#include "functional_module.hpp"
#include "module.hpp"
#include "cpp/native_module.hpp"
#include <stdio.h>

namespace donkey{

class placeholder{
	placeholder(const placeholder&) = delete;
	void operator=(const placeholder&) = delete;
private:
	size_t _idx;
public:
	placeholder(size_t idx):
		_idx(idx){
	}
	
	static vtable_ptr vt(){
		static vtable_ptr ret([](){
			std::unordered_map<std::string, method_ptr> methods;
			
			vtable* vt = new vtable(
				"functional",
				"Placeholder",
				function(),
				std::move(methods),
				false
			);
			vt->derive_from(*object_vtable());
			return vt;
		}());
		
		return ret;
	}
	
	vtable* get_vtable(){
		return vt().get();
	}
	
	size_t get_idx() const{
		return _idx;
	}
};

class placeholders{
	placeholders(const placeholders&) = delete;
	void operator=(const placeholders&) = delete;
public:
	placeholders(){
	}
	
	static vtable_ptr vt(){
		static vtable_ptr ret([](){
			std::unordered_map<std::string, method_ptr> methods;
			
			methods.emplace("opGet", create_native_method("functional::_::get_item", &placeholders::get_item));
			
			vtable* vt = new vtable(
				"functional",
				"Placeholders",
				function(),
				std::move(methods),
				false
			);
			vt->derive_from(*object_vtable());
			return vt;
		}());
		
		return ret;
	}
	
	variable get_item(integer idx){
		return variable(new placeholder(size_t(idx)));
	}
	
	vtable* get_vtable(){
		return vt().get();
	}
};

static statement_retval init_functional(runtime_context& ctx, size_t module_idx, size_t placeholders_idx){
	variable& v_placeholders = global_variable(ctx, module_idx, placeholders_idx);
	
	v_placeholders = variable(new placeholders());
	
	return statement_retval::nxt;
}

module_ptr load_functional_module(size_t module_idx){
	using namespace std::placeholders;
	
	native_module m("functional", module_idx);
	
	m.add_vtable(placeholders::vt());
	
	size_t placeholders_idx = m.add_global("_");
	
	m.set_init(std::bind(&init_functional, _1, module_idx, placeholders_idx));
	
	return m.create_module();
}



}//donkey
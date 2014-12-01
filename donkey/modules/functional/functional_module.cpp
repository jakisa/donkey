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


static variable bind(runtime_context& ctx, size_t params_size){
	using namespace std::placeholders;
	
	if(params_size == 0){
		return variable();
	}
	variable f = ctx.top(params_size-1);
	
	std::vector<function> params;
	params.reserve(params_size-1);
	
	for(size_t i = 0; i != params_size-1; ++i){
		variable v = ctx.top(params_size - i - 2);
		if(v.get_vtable() == placeholder::vt().get()){
			size_t param_idx = v.as_t_unsafe<placeholder>()->get_idx();
			params.push_back([param_idx](runtime_context& ctx, size_t params_size){
				return param_idx < params_size ? ctx.top(params_size - param_idx - 1) : variable();
			});
		}else{
			params.push_back([v](runtime_context&, size_t){
				return v;
			});
		}
	}
	
	return variable(function([f, params](runtime_context& ctx, size_t params_size){
		std::vector<variable> evaluated;
		evaluated.reserve(params.size());
		for(function f: params){
			evaluated.push_back(f(ctx, params_size));
		}
		stack_pusher pusher(ctx, params.size());
		
		for(variable& v: evaluated){
			pusher.push(std::move(v));
		}
		
		return f.call(ctx, params.size());
	}));
}

module_ptr load_functional_module(size_t module_idx){
	native_module m("functional", module_idx);
	
	m.add_vtable(placeholders::vt());
	
	m.add_function("bind", &bind);
	
	size_t placeholders_idx = m.add_global("_");
	
	
	m.set_init([module_idx, placeholders_idx](runtime_context& ctx){
		variable& v_placeholders = global_variable(ctx, module_idx, placeholders_idx);
	
		v_placeholders = variable(new placeholders());
		
		return statement_retval::nxt;
	});
	
	return m.create_module();
}



}//donkey
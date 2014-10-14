#ifndef __functional_expressions_hpp__
#define __functional_expressions_hpp__

#include "expressions.hpp"

#include "vtable.hpp"

namespace donkey{

class function_call_expression final: public expression{
private:
	std::vector<expression_ptr> _params;
	std::vector<bool> _byref;
	expression_ptr _f;

public:
	function_call_expression(expression_ptr f, std::vector<expression_ptr> params, std::vector<size_t> byref):
		expression(expression_type::variant),
		_params(std::move(params)),
		_byref(params.size()),
		_f(f){
		
		for(auto sz: byref){
			_byref[sz] = true;
		}
	}
	
	virtual number as_number(runtime_context& ctx) final override{
		return as_param(ctx).as_number();
	}
	
	virtual std::string as_string(runtime_context& ctx) final override{
		return as_param(ctx).to_string();
	}

	virtual variable call(runtime_context& ctx, size_t params_size) final override{
		return as_param(ctx).call(ctx, params_size);
	}

	virtual variable as_param(runtime_context& ctx) final override{
		std::vector<variable*> refs(_params.size());
	
		stack_pusher pusher(ctx);
		
		for(size_t i = 0; i < _params.size(); ++i){
			if(_byref[i]){
				refs[i] = &(static_cast<lvalue_expression&>(*(_params[i])).as_lvalue(ctx));
				variable param = *refs[i];
				pusher.push(std::move(param));
			}else{
				pusher.push(_params[i]->as_param(ctx));
			}
		}
		
		variable ret = _f->call(ctx, _params.size());
		
		for(size_t i = 0; i < _params.size(); ++i){
			if(_byref[i]){
				*refs[i] = std::move(ctx.top(_params.size() - i - 1));
			}
		}
		
		return ret;
	}

	virtual void as_void(runtime_context & ctx) final override{
		as_param(ctx);
	}
};

class function_call_expression_byval final: public expression{
private:
	std::vector<expression_ptr> _params;
	expression_ptr _f;

public:
	function_call_expression_byval(expression_ptr f, std::vector<expression_ptr> params):
		expression(expression_type::variant),
		_params(std::move(params)),
		_f(f){
	}
	
	virtual number as_number(runtime_context& ctx) final override{
		return as_param(ctx).as_number();
	}
	
	virtual std::string as_string(runtime_context& ctx) final override{
		return as_param(ctx).to_string();
	}

	virtual variable call(runtime_context& ctx, size_t params_size) final override{
		return as_param(ctx).call(ctx, params_size);
	}

	virtual variable as_param(runtime_context& ctx) final override{
		stack_pusher pusher(ctx);
		
		for(size_t i = 0; i < _params.size(); ++i){
			pusher.push(_params[i]->as_param(ctx));
		}
		
		
		return _f->call(ctx, _params.size());
	}

	virtual void as_void(runtime_context & ctx) final override{
		as_param(ctx);
	}
};


class constructor_call_expression final: public expression{
private:
	std::string _module_name;
	std::string _type_name;
	std::vector<expression_ptr> _params;
	
	void init(variable& that, vtable* vt, runtime_context& ctx){
		stack_pusher pusher(ctx);
	
		for(size_t i = 0; i < _params.size(); ++i){
			pusher.push(_params[i]->as_param(ctx));
		}
		
		vt->call_constructor(that, ctx, _params.size());
	}
	
public:
	constructor_call_expression(std::string module_name, std::string type_name, std::vector<expression_ptr> params):
		expression(string_to_type(type_name)),
		_module_name(module_name),
		_type_name(type_name),
		_params(std::move(params)){
	}
	virtual variable as_param(runtime_context& ctx) override{
		vtable* vt = get_vtable(ctx, _module_name, _type_name);
		variable ret(vt, ctx);
		init(ret, vt, ctx);
		return ret;
	}
	virtual number as_number(runtime_context& ctx) final override{
		return as_param(ctx).as_number();
	}
	
	virtual std::string as_string(runtime_context& ctx) final override{
		return as_param(ctx).to_string();
	}

	virtual variable call(runtime_context& ctx, size_t params_size) override{
		return as_param(ctx).call(ctx, params_size);
	}

	virtual void as_void(runtime_context & ctx) final override{
		as_param(ctx);
	}
};


}//donkey

#endif /* __functional_expressions_hpp__ */

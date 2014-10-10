#ifndef __functional_expressions_hpp__
#define __functional_expressions_hpp__

#include "expressions.hpp"

#include "vtable.hpp"

namespace donkey{

class function_call_expression final: public expression{
private:
	std::vector<expression_ptr> _params;
	std::vector<char> _byref;
	expression_ptr _f;

public:
	function_call_expression(expression_ptr f, std::vector<expression_ptr> params, std::vector<char> byref):
		expression(expression_type::variant),
		_params(std::move(params)),
		_byref(std::move(byref)),
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
		stack_ref_manipulator pusher(ctx);
		
		for(size_t i = 0; i < _params.size(); ++i){
			if(_byref[i]){
				pusher.push_ref(static_cast<lvalue_expression&>(*(_params[i])).as_lvalue(ctx));
			}else{
				pusher.push(_params[i]->as_param(ctx));
			}
		}
		
		return _f->call(ctx, _params.size());
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
	std::string _type_name;
	std::vector<expression_ptr> _params;
	std::vector<char> _byref;
	
	void init(variable& that, vtable* vt, runtime_context& ctx){
		if(vt->has_method(_type_name)){
			
			stack_ref_manipulator pusher(ctx);
		
			for(size_t i = 0; i < _params.size(); ++i){
				if(_byref[i]){
					pusher.push_ref(static_cast<lvalue_expression&>(*(_params[i])).as_lvalue(ctx));
				}else{
					pusher.push(_params[i]->as_param(ctx));
				}
			}
			
			vt->call_member(that, ctx, _params.size(), _type_name);
		}
	}
	
public:
	constructor_call_expression(std::string type_name, std::vector<expression_ptr> params, std::vector<char> byref):
		expression(string_to_type(type_name)),
		_type_name(type_name),
		_params(std::move(params)),
		_byref(std::move(byref)){
	}
	virtual variable as_param(runtime_context& ctx) override{
		vtable* vt = get_vtable(ctx, _type_name);
		variable ret(vt);
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

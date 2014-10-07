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

	virtual variable make_call(runtime_context& ctx){
		stack_restorer _(ctx);
	
		for(size_t i = 0; i < _params.size(); ++i){
			if(_byref[i]){
				ctx.push(std::static_pointer_cast<lvalue_expression>(_params[i])->as_lvalue(ctx).by_ref(ctx.stack));
			}else{
				ctx.push(_params[i]->as_param(ctx).by_val());
			}
		}
		
		return _f->call(ctx, _params.size());
	}

public:
	function_call_expression(expression_ptr f, std::vector<expression_ptr> params, std::vector<char> byref):
		expression(expression_type::variant),
		_params(std::move(params)),
		_byref(std::move(byref)),
		_f(f){
	}
	
	virtual number as_number(runtime_context& ctx) final override{
		return make_call(ctx).as_number();
	}
	
	virtual std::string as_string(runtime_context& ctx) final override{
		return make_call(ctx).to_string();
	}

	virtual variable call(runtime_context& ctx, size_t params_size) override{
		return make_call(ctx).call(ctx, params_size);
	}

	virtual variable as_param(runtime_context& ctx) final override{
		return make_call(ctx);
	}

	virtual void as_void(runtime_context & ctx) final override{
		make_call(ctx);
	}
};

class constructor_call_expression final: public expression{
private:
	std::string _type_name;
	std::vector<expression_ptr> _params;
	std::vector<char> _byref;
public:
	constructor_call_expression(std::string type_name, std::vector<expression_ptr> params, std::vector<char> byref):
		expression(string_to_type(type_name)),
		_type_name(type_name),
		_params(std::move(params)),
		_byref(std::move(byref)){
	}
	virtual variable as_param(runtime_context& ctx) override{
		vtable* vt = get_vtable(ctx, _type_name);
		return variable(_type_name, vt->get_fields_size(), vt);
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

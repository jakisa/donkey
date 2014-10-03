#ifndef __functional_expressions_hpp__
#define __functional_expressions_hpp__

#include "expressions.hpp"

#include "vtable.hpp"

namespace donkey{

class functional_expression: public expression{
private:
	virtual variable make_call(runtime_context&) = 0;
public:
	functional_expression():
		expression(expression_type::variant){
	}
	
	virtual double as_number(runtime_context& ctx) final override{
		return make_call(ctx).as_number();
	}
	
	virtual std::string as_string(runtime_context& ctx) final override{
		return make_call(ctx).to_string();
	}

	virtual code_address as_function(runtime_context& ctx) final override{
		return make_call(ctx).as_function();
	}

	virtual variable as_param(runtime_context& ctx) final override{
		return make_call(ctx);
	}

	virtual void as_void(runtime_context & ctx) final override{
		make_call(ctx);
	}
};

class function_call_expression final: public functional_expression{
private:
	std::vector<expression_ptr> _params;
	std::vector<char> _byref;
	expression_ptr _f;

	virtual variable make_call(runtime_context& ctx) override{
		stack_restorer _(ctx);
	
		for(size_t i = 0; i < _params.size(); ++i){
			if(_byref[i]){
				ctx.push(std::static_pointer_cast<lvalue_expression>(_params[i])->as_lvalue(ctx).by_ref(ctx.stack));
			}else{
				ctx.push(_params[i]->as_param(ctx).by_val());
			}
		}
		
		return ctx.code->call_function_by_address(_f->as_function(ctx), ctx, _params.size());
	}

public:
	function_call_expression(expression_ptr f, std::vector<expression_ptr> params, std::vector<char> byref):
		_params(std::move(params)),
		_byref(std::move(byref)),
		_f(f){
	}
};

class method_call_expression final: public functional_expression{
private:
	std::vector<expression_ptr> _params;
	std::vector<char> _byref;
	std::string _name;
	expression_ptr _that;
	
	virtual variable make_call(runtime_context& ctx) override{
		variable that = _that->as_param(ctx);
		
		stack_restorer _(ctx);
	
		for(size_t i = 0; i < _params.size(); ++i){
			if(_byref[i]){
				ctx.push(std::static_pointer_cast<lvalue_expression>(_params[i])->as_lvalue(ctx).by_ref(ctx.stack));
			}else{
				ctx.push(_params[i]->as_param(ctx).by_val());
			}
		}
		
		return vtable::call_method(that, ctx, _params.size(), _name);
	}
public:
	method_call_expression(expression_ptr that, std::string name, std::vector<expression_ptr> params, std::vector<char> byref):
		_params(std::move(params)),
		_byref(byref),
		_name(name),
		_that(that){
	}
};

}//donkey

#endif /* __functional_expressions_hpp__ */

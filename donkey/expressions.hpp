#ifndef __expressions_hpp__
#define __expressions_hpp__

#include "runtime_context.hpp"
#include "errors.hpp"
#include "helpers.hpp"
#include "variables.hpp"

namespace donkey{

enum class expression_type{
	number,
	string,
	function,
	lvalue,
	variant,
};

class expression{
	expression(const expression&) = delete;
	void operator=(const expression&) = delete;
private:
	expression_type _t;
protected:
	expression(expression_type t):
		_t(t){
	}
public:
	virtual double as_number(runtime_context&){
		runtime_error("expression is not number");
		return nan("");
	}
	
	virtual std::string as_string(runtime_context& ctx){
		return to_string(as_number(ctx));
	}

	virtual function as_function(runtime_context&){
		runtime_error("expression is not function");
		return function();
	}

	virtual variable_ptr as_param(runtime_context&) = 0;

	virtual void as_void(runtime_context&) = 0;

	expression_type get_type(){
		return _t;
	}

	virtual ~expression(){
	}
};

typedef std::shared_ptr<expression> expression_ptr;

inline double& get_lnumber(variable_ptr v){
	if(variable::get_type(v) != type::number){
		runtime_error("variable is not number");
		return *((double*)nullptr);
	}
	return std::static_pointer_cast<number_variable>(v)->value();
}

class lvalue_expression: public expression{
protected:
	lvalue_expression():
		expression(expression_type::lvalue){
	}
public:
	virtual variable_ptr& as_lvalue(runtime_context&) = 0;

	virtual variable_ptr as_param(runtime_context& ctx) override final{
		return as_lvalue(ctx)->as_param();
	}

	virtual double as_number(runtime_context& ctx) override final{
		variable_ptr v = as_lvalue(ctx);
		if(variable::get_type(v) != type::number){
			runtime_error("expression is not number");
			return nan("");
		}
		return std::static_pointer_cast<number_variable>(v)->value();
	}

	virtual std::string as_string(runtime_context& ctx) override final{
		variable_ptr v = as_lvalue(ctx);
		if(variable::get_type(v) != type::string){
			runtime_error("expression is not string");
			return "";
		}
		return std::static_pointer_cast<string_variable>(v)->value();
	}
	
	virtual function as_function(runtime_context& ctx) override final{
		variable_ptr v = as_lvalue(ctx);
		if(variable::get_type(v) != type::function){
			runtime_error("expression is not function");
			return function();
		}
		return std::static_pointer_cast<function_variable>(v)->value();
	}
};

typedef std::shared_ptr<lvalue_expression> lvalue_expression_ptr;

class const_number_expression final: public expression{
private:
	double _d;
public:
	const_number_expression(double d):
		expression(expression_type::number),
		_d(d){
	}
	virtual double as_number(runtime_context&) override{
		return _d;
	}
	virtual variable_ptr as_param(runtime_context&) override{
		return variable_ptr(new number_variable(_d));
	}

	virtual void as_void(runtime_context&) override{
	}
};

class const_string_expression final: public expression{
private:
	std::string _s;
public:
	const_string_expression(std::string s):
		expression(expression_type::string),
		_s(std::move(s)){
	}
	virtual std::string as_string(runtime_context&) override{
		return _s;
	}
	virtual variable_ptr as_param(runtime_context&) override{
		return variable_ptr(new string_variable(_s));
	}

	virtual void as_void(runtime_context&) override{
	}
};

class const_function_expression final: public expression{
private:
	function _f;
public:
	const_function_expression(function f):
		expression(expression_type::function),
		_f(std::move(f)){
	}
	virtual function as_function(runtime_context&) override{
		return _f;
	}
	virtual std::string as_string(runtime_context&) override{
		return "const function";
	}
	virtual variable_ptr as_param(runtime_context&) override{
		return variable_ptr(new function_variable(_f));
	}

	virtual void as_void(runtime_context&) override{
	}
};


class local_variable_expression final: public lvalue_expression{
private:
	int _idx;
public:
	local_variable_expression(int idx):
		_idx(idx){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		return ctx.stack[ctx.stack.size() - 1 - _idx];
	}

	virtual void as_void(runtime_context&) override{
	}
};

class global_variable_expression final: public lvalue_expression{
private:
	int _idx;
public:
	global_variable_expression(int idx):
		_idx(idx){
	}
	virtual variable_ptr& as_lvalue(runtime_context & ctx) override{
		return ctx.global[_idx];
	}

	virtual void as_void(runtime_context&) override{
	}
};


class function_call_expression final: public expression{
private:
	function _f;
	std::vector<expression_ptr> _params;

	variable_ptr make_call(runtime_context& ctx){
		size_t sz = ctx.stack.size();
	
		for(auto it = _params.begin(); it != _params.end(); ++it){
			ctx.push((*it)->as_param(ctx));
		}
		
		variable_ptr ret = _f(ctx, _params.size());
		
		ctx.stack.resize(sz);
		
		return ret;
	}

public:
	function_call_expression(function f, std::vector<expression_ptr> params):
		expression(expression_type::variant),
		_f(std::move(f)),
		_params(params){
	}

	virtual double as_number(runtime_context& ctx){
		variable_ptr ret = make_call(ctx);
		if(variable::get_type(ret) == type::number){
			return std::static_pointer_cast<number_variable>(ret)->value();
		}
		return expression::as_number(ctx);
	}
	virtual std::string as_string(runtime_context& ctx){
		return variable::to_string(make_call(ctx));
	}

	virtual function as_function(runtime_context& ctx){
		variable_ptr ret = make_call(ctx);
		if(variable::get_type(ret) == type::function){
			return std::static_pointer_cast<function_variable>(ret)->value();
		}
		return expression::as_function(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx){
		return make_call(ctx);
	}

	virtual void as_void(runtime_context & ctx){
		make_call(ctx);
	}
};

class pre_inc_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e;
public:
	pre_inc_expression(lvalue_expression_ptr e):
		_e(e){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e->as_lvalue(ctx);
		++get_lnumber(v);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class post_inc_expression final: public expression{
private:
	lvalue_expression_ptr _e;
public:
	post_inc_expression(lvalue_expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override final{
		return get_lnumber(_e->as_lvalue(ctx))++;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override final{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override final{
		as_number(ctx);
	}
};

class pre_dec_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e;
public:
	pre_dec_expression(lvalue_expression_ptr e):
		_e(e){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e->as_lvalue(ctx);
		--get_lnumber(v);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class post_dec_expression final: public expression{
private:
	lvalue_expression_ptr _e;
public:
	post_dec_expression(lvalue_expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override final{
		return get_lnumber(_e->as_lvalue(ctx))--;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override final{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override final{
		as_number(ctx);
	}
};

class unary_plus_expression final: public expression{
private:
	expression_ptr _e;
public:
	unary_plus_expression(expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class unary_minus_expression final: public expression{
private:
	expression_ptr _e;
public:
	unary_minus_expression(expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override{
		return -_e->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class bitwise_not_expression final: public expression{
private:
	expression_ptr _e;
public:
	bitwise_not_expression(expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override{
		return ~int(_e->as_number(ctx));
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class logical_not_expression final: public expression{
private:
	expression_ptr _e;
public:
	logical_not_expression(expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override{
		return !_e->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class mul_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	mul_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) * _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class div_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	div_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) / _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class idiv_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	idiv_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) / (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class mod_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	mod_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return fmod(_e1->as_number(ctx),_e2->as_number(ctx));
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class plus_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	plus_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) + _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class minus_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	minus_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) - _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class concat_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	concat_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::string),
		_e1(e1),
		_e2(e2){
	}

	virtual std::string as_string(runtime_context& ctx) override{
		return _e1->as_string(ctx) + _e2->as_string(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new string_variable(as_string(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_string(ctx);
	}
};

class shiftl_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	shiftl_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) << (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class shiftr_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	shiftr_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) >> (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

inline bool is_variant_expression(expression_ptr e){
	return e->get_type() == expression_type::lvalue || e->get_type() == expression_type::variant;
}

inline variable_ptr get_variable(expression_ptr e, runtime_context& ctx){
	if(e->get_type() == expression_type::lvalue){
		return std::static_pointer_cast<lvalue_expression>(e)->as_lvalue(ctx);
	}
	return e->as_param(ctx);
}


inline bool less_than(expression_ptr e1, expression_ptr e2, runtime_context& ctx){
	if(e1->get_type() == expression_type::function || e2->get_type() == expression_type::function){
		runtime_error("cannot compare const functions");
	}
	if(is_variant_expression(e1)){
		if(is_variant_expression(e2)){
			return variable::less(get_variable(e1, ctx), get_variable(e2, ctx));
		}
		if(e2->get_type() == expression_type::number){
			return variable::less(get_variable(e1, ctx), e2->as_number(ctx));
		}
		return variable::less(get_variable(e1, ctx), e2->as_string(ctx));
	}
	if(is_variant_expression(e2)){
		if(e1->get_type() == expression_type::number){
			return variable::greater(get_variable(e2, ctx), e1->as_number(ctx));
		}
		return variable::greater(get_variable(e2, ctx), e1->as_string(ctx));
	}
	if(e1->get_type() == expression_type::number){
		if(e2->get_type() == expression_type::number){
			return e1->as_number(ctx) < e2->as_number(ctx);
		}

		runtime_error("cannot compare number and string");
		return false;
	}

	if(e2->get_type() == expression_type::string){
		return e1->as_string(ctx) < e2->as_string(ctx);
	}
	runtime_error("cannot compare string and number");
	return false;
}

inline bool equal_to(expression_ptr e1, expression_ptr e2, runtime_context& ctx){
	if(e1->get_type() == expression_type::function || e2->get_type() == expression_type::function){
		runtime_error("cannot compare const functions");
	}
	if(is_variant_expression(e1)){
		if(is_variant_expression(e2)){
			return variable::equals(get_variable(e1, ctx), get_variable(e2, ctx));
		}
		if(e2->get_type() == expression_type::number){
			return variable::equals(get_variable(e1, ctx), e2->as_number(ctx));
		}
		return variable::equals(get_variable(e1, ctx), e2->as_string(ctx));
	}
	if(is_variant_expression(e2)){
		if(e1->get_type() == expression_type::number){
			return variable::equals(get_variable(e2, ctx), e1->as_number(ctx));
		}
		return variable::equals(get_variable(e2, ctx), e1->as_string(ctx));
	}
	if(e1->get_type() == expression_type::number){
		if(e2->get_type() == expression_type::number){
			return e1->as_number(ctx) == e2->as_number(ctx);
		}
		return false;
	}

	if(e2->get_type() == expression_type::string){
		return e1->as_string(ctx) == e2->as_string(ctx);
	}
	return false;
}


class less_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	less_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return less_than(_e1, _e2, ctx) ? 1 : 0;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class greater_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	greater_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return less_than(_e2, _e1, ctx) ? 1 : 0;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class less_equal_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	less_equal_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return less_than(_e2, _e1, ctx) ? 0 : 1;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class greater_equal_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	greater_equal_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return less_than(_e1, _e2, ctx) ? 0 : 1;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class equal_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	equal_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return equal_to(_e1, _e2, ctx) ? 1 : 0;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class unequal_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	unequal_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return equal_to(_e1, _e2, ctx) ? 0 : 1;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class bitwise_and_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	bitwise_and_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) & (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class bitwise_xor_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	bitwise_xor_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) ^ (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class bitwise_or_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	bitwise_or_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) | (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class logical_and_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	logical_and_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) && _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class logical_or_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	logical_or_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) || _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

inline expression_type combined_expression_type(expression_ptr e1, expression_ptr e2){
	if(is_variant_expression(e1) || is_variant_expression(e2)){
		return expression_type::variant;
	}
	if(e1->get_type() == expression_type::function && e2->get_type() == expression_type::function){
		return expression_type::function;
	}
	if(e1->get_type() == expression_type::number && e2->get_type() == expression_type::number){
		return expression_type::number;
	}
	return expression_type::string;
}

class conditional_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
	expression_ptr _e3;
public:
	conditional_expression(expression_ptr e1, expression_ptr e2, expression_ptr e3):
		expression(combined_expression_type(e2, e3)),
		_e1(e1),
		_e2(e2),
		_e3(e3){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) ? _e2->as_number(ctx) : _e3->as_number(ctx);
	}
	
	virtual std::string as_string(runtime_context &ctx) override{
		return _e1->as_number(ctx) ? _e2->as_string(ctx) : _e3->as_string(ctx);
	}
	
	virtual function as_function(runtime_context &ctx) override{
		return _e1->as_number(ctx) ? _e2->as_function(ctx) : _e3->as_function(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return _e1->as_number(ctx) ? _e2->as_param(ctx) : _e3->as_param(ctx);
	}

	virtual void as_void(runtime_context& ctx) override{
		_e1->as_number(ctx) ? _e2->as_void(ctx) : _e3->as_void(ctx);
	}
};

class assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		if(variable::get_type(v) == type::number && _e2->get_type() == expression_type::number){
			double& n = get_lnumber(v);
			n = _e2->as_number(ctx);
		}else{
			v = _e1->as_param(ctx);
		}
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class mul_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	mul_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n *= _e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class div_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	div_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n /= _e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class idiv_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	idiv_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n = (int)n / (int)_e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class mod_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	mod_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n = fmod(n, _e2->as_number(ctx));
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class plus_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	plus_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n += _e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class minus_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	minus_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n -= _e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};


class concat_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	concat_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		v.reset(new string_variable(v->to_string() + _e2->as_string(ctx)));
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class shiftl_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	shiftl_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n = (int)n << (int)_e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class shiftr_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	shiftr_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n = (int)n >> (int)_e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class and_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	and_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n = (int)n & (int)_e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class xor_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	xor_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n = (int)n ^ (int)_e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class or_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	or_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e1->as_lvalue(ctx);
		double& n = get_lnumber(v);
		n = (int)n | (int)_e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class comma_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	comma_expression(expression_ptr e1, expression_ptr e2):
		expression(e2->get_type()),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_void(ctx), _e2->as_number(ctx);
	}
	
	virtual std::string as_string(runtime_context &ctx) override{
		return _e1->as_void(ctx), _e2->as_string(ctx);
	}
	
	virtual function as_function(runtime_context &ctx) override{
		return _e1->as_void(ctx), _e2->as_function(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return _e1->as_void(ctx), _e2->as_param(ctx);
	}

	virtual void as_void(runtime_context& ctx) override{
		_e1->as_void(ctx), _e2->as_void(ctx);
	}
};

}//namespace donkey

#endif /*__expressions_hpp__*/


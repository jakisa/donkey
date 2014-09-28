#ifndef __expressions_hpp__
#define __expressions_hpp__

#include "runtime_context.hpp"
#include "errors.hpp"
#include "helpers.hpp"
#include "variables.hpp"
#include <vector>

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

inline bool is_variant_expression(expression_ptr e){
	return e->get_type() == expression_type::lvalue || e->get_type() == expression_type::variant;
}

class lvalue_expression: public expression{
protected:
	lvalue_expression():
		expression(expression_type::lvalue){
	}
public:
	virtual variable_ptr& as_lvalue(runtime_context&) = 0;

	virtual variable_ptr as_param(runtime_context& ctx) override final{
		return variable::as_param(as_lvalue(ctx));
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
		return variable::to_string(v);
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

inline variable_ptr get_variable(expression_ptr e, runtime_context& ctx){
	if(e->get_type() == expression_type::lvalue){
		return std::static_pointer_cast<lvalue_expression>(e)->as_lvalue(ctx);
	}
	return e->as_param(ctx);
}

class null_expression final: public expression{
public:
	null_expression():
		expression(expression_type::variant){
	}
	virtual std::string as_string(runtime_context& ctx) override{
		return variable::to_string(as_param(ctx));
	}
	virtual variable_ptr as_param(runtime_context&) override{
		return variable_ptr();
	}
	virtual void as_void(runtime_context&) override{
	}
};



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
		return ctx.stack[ctx.function_stack_bottom +_idx];
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
	expression_ptr _f;
	std::vector<expression_ptr> _params;

	variable_ptr make_call(runtime_context& ctx){
		stack_restorer _(ctx);
	
		for(auto it = _params.begin(); it != _params.end(); ++it){
			ctx.push((*it)->as_param(ctx));
		}
		
		return _f->as_function(ctx)(ctx, _params.size());
	}

public:
	function_call_expression(expression_ptr f, std::vector<expression_ptr> params):
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


enum class oper{
	none                 =   0,
	operand              =   1,
	bracket_open         =   2,
	bracket_close        =   3,

	comma                =  20,
	or_assignment        =  40,
	xor_assignment       =  41,
	and_assignment       =  42,
	shiftr_assignment    =  43,
	shiftl_assignment    =  44,
	concat_assignment    =  45,
	minus_assignment     =  46,
	plus_assignment      =  47,
	mod_assignment       =  48,
	idiv_assignment      =  49,
	div_assignment       =  50,
	mul_assignment       =  51,
	assignment           =  52,
	conditional_question =  60,
	conditional_colon    =  61,
	logical_or           =  80,
	logical_and          = 100,
	unequal              = 120,
	equal                = 121,
	greater_equal        = 140,
	less_equal           = 141,
	greater              = 142,
	less                 = 143,
	shiftr               = 160,
	shiftl               = 161,
	bitwise_or           = 180,
	bitwise_xor          = 181,
	concat               = 182,
	minus                = 183,
	plus                 = 184,
	bitwise_and          = 185,
	mod                  = 200,
	idiv                 = 201,
	div                  = 202,
	mul                  = 203,
	logical_not          = 220,
	bitwise_not          = 221,
	unary_minus          = 222,
	unary_plus           = 223,
	pre_dec              = 224,
	pre_inc              = 225,
	post_dec             = 240,
	post_inc             = 241,
};

class identifier_lookup;

expression_ptr build_null_expression();
expression_ptr build_number_expression(double d);
expression_ptr build_string_expression(std::string str);
expression_ptr build_variable_expression(std::string name, const identifier_lookup& lookup);
expression_ptr build_function_expression(std::string name, std::vector<expression_ptr>& params, const identifier_lookup& lookup);

expression_ptr build_unary_expression(oper op, expression_ptr e1);
expression_ptr build_binary_expression(oper op, expression_ptr e1, expression_ptr e2);
expression_ptr build_ternary_expression(oper op, expression_ptr e1, expression_ptr e2, expression_ptr e3);


}//namespace donkey

#endif /*__expressions_hpp__*/


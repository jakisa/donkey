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

	virtual code_address as_function(runtime_context&){
		runtime_error("expression is not function");
		return -1;
	}

	virtual stack_var as_param(runtime_context&) = 0;

	virtual void as_void(runtime_context&) = 0;

	expression_type get_type(){
		return _t;
	}

	virtual ~expression(){
	}
};

typedef std::shared_ptr<expression> expression_ptr;

inline bool is_variant_expression(expression_ptr e){
	return e->get_type() == expression_type::lvalue || e->get_type() == expression_type::variant;
}

class lvalue_expression: public expression{
protected:
	lvalue_expression():
		expression(expression_type::lvalue){
	}
public:
	virtual stack_var& as_lvalue(runtime_context&) = 0;

	virtual stack_var as_param(runtime_context& ctx) override final{
		return as_lvalue(ctx);
	}

	virtual double as_number(runtime_context& ctx) override final{
		return as_lvalue(ctx).as_number();
	}

	virtual std::string as_string(runtime_context& ctx) override final{
		return as_lvalue(ctx).to_string();
	}
	
	virtual code_address as_function(runtime_context& ctx) override final{
		return as_lvalue(ctx).as_function();
	}
};

typedef std::shared_ptr<lvalue_expression> lvalue_expression_ptr;

class null_expression final: public expression{
public:
	null_expression():
		expression(expression_type::variant){
	}
	virtual std::string as_string(runtime_context& ctx) override{
		return as_param(ctx).to_string();
	}
	virtual stack_var as_param(runtime_context&) override{
		return stack_var();
	}
	virtual void as_void(runtime_context&) override{
	}
};



class const_number_expression final: public expression{
private:
	stack_var _d;
public:
	const_number_expression(double d):
		expression(expression_type::number),
		_d(d){
	}
	virtual double as_number(runtime_context&) override{
		return _d.as_stack_number_unsafe();
	}
	virtual stack_var as_param(runtime_context&) override{
		return _d;
	}

	virtual void as_void(runtime_context&) override{
	}
};

class const_string_expression final: public expression{
private:
	stack_var _s;
public:
	const_string_expression(std::string s):
		expression(expression_type::string),
		_s(s){
	}
	virtual std::string as_string(runtime_context&) override{
		return _s.to_string();
	}
	virtual stack_var as_param(runtime_context&) override{
		return _s;
	}

	virtual void as_void(runtime_context&) override{
	}
};

class const_function_expression final: public expression{
private:
	code_address _f;
public:
	const_function_expression(code_address f):
		expression(expression_type::function),
		_f(f){
	}
	virtual code_address as_function(runtime_context&) override{
		return _f;
	}
	virtual std::string as_string(runtime_context&) override{
		return "function";
	}
	virtual stack_var as_param(runtime_context&) override{
		return stack_var(_f);
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

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
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
	virtual stack_var& as_lvalue(runtime_context & ctx) override{
		return ctx.global[_idx];
	}

	virtual void as_void(runtime_context&) override{
	}
};

class function_call_expression final: public expression{
private:
	expression_ptr _f;
	std::vector<expression_ptr> _params;
	std::vector<bool> _byref;

	stack_var make_call(runtime_context& ctx){
		stack_restorer _(ctx);
	
		for(size_t i = 0; i < _params.size(); ++i){
			if(_byref[i]){
				ctx.push(std::static_pointer_cast<lvalue_expression>(_params[i])->as_lvalue(ctx).by_ref());
			}else{
				ctx.push(_params[i]->as_param(ctx).by_val());
			}
		}
		
		return ctx.code->call_function_by_address(_f->as_function(ctx), ctx, _params.size());
	}

public:
	function_call_expression(expression_ptr f, std::vector<expression_ptr> params, const std::vector<bool> byref):
		expression(expression_type::variant),
		_f(std::move(f)),
		_params(params),
		_byref(byref){
	}

	virtual double as_number(runtime_context& ctx){
		return make_call(ctx).as_number();
	}
	virtual std::string as_string(runtime_context& ctx){
		return make_call(ctx).to_string();
	}

	virtual code_address as_function(runtime_context& ctx){
		return make_call(ctx).as_function();
	}

	virtual stack_var as_param(runtime_context& ctx){
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

expression_ptr identifier_to_expression(std::string str, const identifier_lookup& lookup);

expression_ptr build_null_expression();
expression_ptr build_number_expression(double d);
expression_ptr build_string_expression(std::string str);
expression_ptr build_variable_expression(std::string name, const identifier_lookup& lookup);
expression_ptr build_function_expression(std::string name, std::vector<expression_ptr>& params, const std::vector<bool>& byref, const identifier_lookup& lookup);

expression_ptr build_unary_expression(oper op, expression_ptr e1);
expression_ptr build_binary_expression(oper op, expression_ptr e1, expression_ptr e2);
expression_ptr build_ternary_expression(oper op, expression_ptr e1, expression_ptr e2, expression_ptr e3);



}//namespace donkey

#endif /*__expressions_hpp__*/


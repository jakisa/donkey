#ifndef __expressions_hpp__
#define __expressions_hpp__

#include "runtime_context.hpp"
#include "errors.hpp"
#include "helpers.hpp"
#include "variables.hpp"
#include "vtable.hpp"
#include <vector>

namespace donkey{

enum class expression_type{
	number,
	string,
	function,
	lvalue,
	variant,
};

inline expression_type string_to_type(std::string name){
	if(name == "number"){
		return expression_type::number;
	}
	if(name == "string"){
		return expression_type::string;
	}
	if(name == "function"){
		return expression_type::function;
	}
	return expression_type::variant;
}

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
	virtual number as_number(runtime_context&){
		runtime_error("expression is not number");
		return nan("");
	}
	
	virtual std::string as_string(runtime_context& ctx){
		return to_string(as_number(ctx));
	}

	virtual variable call(runtime_context&, size_t){
		runtime_error("expression is not function");
		return variable();
	}

	virtual variable as_param(runtime_context&) = 0;

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
	virtual variable& as_lvalue(runtime_context&) = 0;

	virtual variable as_param(runtime_context& ctx) override{
		return as_lvalue(ctx);
	}

	virtual number as_number(runtime_context& ctx) override{
		return as_lvalue(ctx).as_number();
	}

	virtual std::string as_string(runtime_context& ctx) override{
		return as_lvalue(ctx).to_string();
	}
	
	virtual variable call(runtime_context& ctx, size_t params_size) override{
		return as_lvalue(ctx).call(ctx, params_size);
	}
};

typedef std::shared_ptr<lvalue_expression> lvalue_expression_ptr;

enum class oper{
	none                 =   0,
	operand              =   1,
	bracket_open         =   2,
	bracket_close        =   3,
	ref                  =   4,

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
	construct            = 226,
	post_dec             = 240,
	post_inc             = 241,
	dot                  = 242,
	call                 = 243,
	scope                = 260,
};


expression_ptr build_null_expression();

expression_ptr build_this_expression();

expression_ptr build_const_number_expression(double n);

expression_ptr build_const_string_expression(const std::string& str);

expression_ptr build_const_function_expression(code_address addr);

expression_ptr build_local_variable_expression(size_t idx);

expression_ptr build_global_variable_expression(size_t idx);

expression_ptr build_unary_expression(oper op, expression_ptr e1, int line_number);

expression_ptr build_binary_expression(oper op, expression_ptr e1, expression_ptr e2, int line_number);

expression_ptr build_ternary_expression(oper op, expression_ptr e1, expression_ptr e2, expression_ptr e3, int line_number);

expression_ptr build_member_expression(expression_ptr that, const std::string& member);

expression_ptr build_field_expression(expression_ptr that, const std::string& type, size_t idx);
expression_ptr build_method_expression(expression_ptr that, const std::string& type, method& m);

expression_ptr build_constructor_call_expression(const std::string& type_name, const std::vector<expression_ptr>& params, const std::vector<size_t>& byref);

expression_ptr build_function_call_expression(expression_ptr f, const std::vector<expression_ptr>& params, const std::vector<size_t>& byref);

}//namespace donkey

#endif /*__expressions_hpp__*/


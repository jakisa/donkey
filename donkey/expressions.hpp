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
	item,
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
	return e->get_type() == expression_type::lvalue || e->get_type() == expression_type::variant || e->get_type() == expression_type::item;
}

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

typedef std::pair<variable, variable> item_handle;

inline void set_item(runtime_context& ctx, const variable& that, variable idx, variable&& value){
	stack_pusher pusher(ctx);
	
	pusher.push(std::move(idx));
	pusher.push(std::move(value));
	
	get_vtable(ctx, that)->call_member(that, ctx, 2, "setItem");
}

inline variable get_item(runtime_context &ctx, const variable& that, variable idx){
	stack_pusher pusher(ctx);
	
	pusher.push(std::move(idx));
	
	return get_vtable(ctx, that)->call_member(that, ctx, 1, "getItem");
}

class item_expression: public expression{
protected:
	item_expression():
		expression(expression_type::item){
	}
	
	variable get_this_item(runtime_context& ctx){
		item_handle i = as_item(ctx);
		return get_item(ctx, i.first, std::move(i.second));
	}
	
public:
	virtual item_handle as_item(runtime_context&) = 0;
	
	virtual variable as_param(runtime_context& ctx) override{
		return get_this_item(ctx);
	}
	
	virtual number as_number(runtime_context& ctx) override{
		return get_this_item(ctx).as_number();
	}
	
	virtual std::string as_string(runtime_context& ctx) override{
		return get_this_item(ctx).to_string();
	}
	
	virtual variable call(runtime_context& ctx, size_t params_size) override{
		return get_this_item(ctx).call(ctx, params_size);
	}
	
	virtual void as_void(runtime_context& ctx) override{
		get_this_item(ctx);
	}
};


typedef std::shared_ptr<item_expression> item_expression_ptr;

enum class oper{
	none                 =   0,
	operand              =   1,
	bracket_open         =   2,
	bracket_close        =   3,
	ref                  =   4,
	subscript_open       =   5,
	subscript_close      =   6,

	comma                =  20,
	fallback_assignment  =  40,
	or_assignment        =  41,
	xor_assignment       =  42,
	and_assignment       =  43,
	shiftr_assignment    =  44,
	shiftl_assignment    =  45,
	concat_assignment    =  46,
	minus_assignment     =  47,
	plus_assignment      =  48,
	mod_assignment       =  49,
	idiv_assignment      =  50,
	div_assignment       =  51,
	mul_assignment       =  52,
	assignment           =  53,
	conditional_question =  60,
	conditional_colon    =  61,
	fallback             =  80,
	logical_or           = 100,
	logical_and          = 120,
	unequal              = 140,
	equal                = 141,
	greater_equal        = 160,
	less_equal           = 161,
	greater              = 162,
	less                 = 163,
	shiftr               = 180,
	shiftl               = 181,
	bitwise_or           = 200,
	bitwise_xor          = 201,
	concat               = 202,
	minus                = 203,
	plus                 = 204,
	bitwise_and          = 205,
	mod                  = 220,
	idiv                 = 221,
	div                  = 222,
	mul                  = 223,
	logical_not          = 240,
	bitwise_not          = 241,
	unary_minus          = 242,
	unary_plus           = 243,
	pre_dec              = 244,
	pre_inc              = 245,
	construct            = 246,
	post_dec             = 260,
	post_inc             = 261,
	dot                  = 262,
	call                 = 263,
	subscript            = 264,
	scope                = 280,
};


expression_ptr build_null_expression();

expression_ptr build_this_expression();

expression_ptr build_const_number_expression(double n);

expression_ptr build_const_string_expression(const std::string& str);

expression_ptr build_const_function_expression(code_address addr);

expression_ptr build_local_variable_expression(size_t idx);

expression_ptr build_global_variable_expression(size_t module_idx, size_t var_idx);

expression_ptr build_unary_expression(oper op, expression_ptr e1, int line_number);

expression_ptr build_binary_expression(oper op, expression_ptr e1, expression_ptr e2, int line_number);

expression_ptr build_ternary_expression(oper op, expression_ptr e1, expression_ptr e2, expression_ptr e3, int line_number);

expression_ptr build_member_expression(expression_ptr that, const std::string& member);

expression_ptr build_field_expression(expression_ptr that, const std::string& name, size_t idx);

expression_ptr build_method_expression(expression_ptr that, const std::string& name, method& m);

expression_ptr build_constructor_call_expression(const std::string& module_name, const std::string& type_name, const std::vector<expression_ptr>& params);

expression_ptr build_function_call_expression(expression_ptr f, const std::vector<expression_ptr>& params, const std::vector<size_t>& byref);

expression_ptr build_index_expression(expression_ptr e1, expression_ptr e2);

}//namespace donkey

#endif /*__expressions_hpp__*/


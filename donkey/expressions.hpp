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
	litem,
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
		return as_param(ctx).to_string(ctx);
	}

	virtual variable call(runtime_context&, size_t){
		runtime_error("expression is not function");
		return variable();
	}

	virtual variable as_param(runtime_context&) = 0;
	
	variable as_var(runtime_context& ctx){
		return as_param(ctx);
	}

	virtual bool as_bool(runtime_context&) = 0;

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
		return as_lvalue(ctx).to_string(ctx);
	}
	
	virtual variable call(runtime_context& ctx, size_t params_size) override{
		return as_lvalue(ctx).call(ctx, params_size);
	}
	
	variable& as_var(runtime_context& ctx){
		return as_lvalue(ctx);
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
	
	virtual bool as_bool(runtime_context& ctx) override{
		return as_lvalue(ctx).to_bool(ctx);
	}
};

typedef std::shared_ptr<lvalue_expression> lvalue_expression_ptr;

struct item_handle{
	item_handle(const item_handle& orig) = delete;
	void operator=(const item_handle& orig) = delete;
	
	typedef item_handle&& rvalue;

	variable that;
	variable index;
	item_handle(variable&& that, variable&& index):
		that(std::move(that)),
		index(std::move(index)){
	}
	
	
	item_handle(item_handle&& orig):
		that(std::move(orig.that)),
		index(std::move(orig.index)){
	}
};

struct l_item_handle{
	typedef l_item_handle&& rvalue;

	variable& that;
	variable index;
	l_item_handle(variable& that, variable&& index):
		that(that),
		index(std::move(index)){
	}
	
	l_item_handle(const l_item_handle& orig):
		that(orig.that),
		index(orig.index){
	}
	
	l_item_handle(l_item_handle&& orig):
		that(orig.that),
		index(std::move(orig.index)){
	}
};

inline void set_item(runtime_context &ctx, const variable& that, variable&& index, variable&& value){
	stack_pusher pusher(ctx, 2);
	
	pusher.push(std::move(value));
	pusher.push(std::move(index));
	
	vtable* vt = that.get_vtable();
	
	if(!vt->opSet){
		runtime_error("opSet is not defined for " + that.get_full_type_name());
	}
	
	(*vt->opSet)(that, ctx, 2);
}

inline variable get_item(runtime_context &ctx, const variable& that, variable&& index){
	stack_pusher pusher(ctx, 1);
	
	pusher.push(std::move(index));
	
	vtable* vt = that.get_vtable();
	
	if(!vt->opGet){
		runtime_error("opGet is not defined for " + that.get_full_type_name());
	}
	
	return (*vt->opGet)(that, ctx, 1);
}

template<class Handle>
class item_expression: public expression{
protected:
	item_expression():
		expression(std::is_same<Handle, item_handle>::value ? expression_type::item : expression_type::litem){
	}
	
	variable get_this_item(runtime_context& ctx){
		Handle i = as_item(ctx);
		return get_item(ctx, i.that, std::move(i.index));
	}
	
public:
	virtual Handle as_item(runtime_context&) = 0;
	
	virtual variable as_param(runtime_context& ctx) override{
		return get_this_item(ctx);
	}
	
	virtual number as_number(runtime_context& ctx) override{
		return get_this_item(ctx).as_number();
	}
	
	virtual std::string as_string(runtime_context& ctx) override{
		return get_this_item(ctx).to_string(ctx);
	}
	
	virtual variable call(runtime_context& ctx, size_t params_size) override{
		return get_this_item(ctx).call(ctx, params_size);
	}
	
	virtual bool as_bool(runtime_context& ctx) override{
		return get_this_item(ctx).to_bool(ctx);
	}
};


typedef std::shared_ptr<item_expression<item_handle> > item_expression_ptr;
typedef std::shared_ptr<item_expression<l_item_handle> > l_item_expression_ptr;

template<class Expression>
struct handle_version{
	typedef item_handle type;
};

template<>
struct handle_version<lvalue_expression_ptr>{
	typedef l_item_handle type;
};

template<typename Handle>
struct handle_version<std::shared_ptr<item_expression<Handle> > >{
	typedef Handle type;
};

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
	array_init           = 247,
	deref                = 248,
	post_dec             = 260,
	post_inc             = 261,
	dot                  = 262,
	call                 = 263,
	subscript            = 264,
	arrow                = 265,
	scope                = 280,
};


expression_ptr build_null_expression();

expression_ptr build_this_expression();

expression_ptr build_const_number_expression(double n);

expression_ptr build_const_string_expression(const std::string& str);

expression_ptr build_const_function_expression(code_address addr);

expression_ptr build_local_variable_expression(size_t idx);

expression_ptr build_global_variable_expression(size_t module_idx, size_t var_idx);

expression_ptr build_unary_expression(oper op, expression_ptr e1);

expression_ptr build_binary_expression(oper op, expression_ptr e1, expression_ptr e2);

expression_ptr build_ternary_expression(oper op, expression_ptr e1, expression_ptr e2, expression_ptr e3);

expression_ptr build_member_expression(expression_ptr that, const std::string& member);

expression_ptr build_field_expression(expression_ptr that, const std::string& name, size_t idx);

expression_ptr build_method_expression(expression_ptr that, const std::string& name, method& m);

expression_ptr build_constructor_call_expression(vtable* vt, const std::vector<expression_ptr>& params);

expression_ptr build_array_initializer(const std::vector<expression_ptr>& items);

expression_ptr build_function_call_expression(expression_ptr f, const std::vector<expression_ptr>& params, const std::vector<size_t>& byref);


}//namespace donkey

#endif /*__expressions_hpp__*/


#include "expressions.hpp"
#include "expressions/core_expressions.hpp"
#include "expressions/unary_expressions.hpp"
#include "expressions/arithmetic_expressions.hpp"
#include "expressions/string_expressions.hpp"
#include "expressions/relation_expressions.hpp"
#include "expressions/logical_expressions.hpp"
#include "expressions/assignment_expressions.hpp"
#include "expressions/sequential_expressions.hpp"
#include "expressions/ternary_expressions.hpp"
#include "expressions/functional_expressions.hpp"
#include "expressions/null_check_expressions.hpp"
#include "expressions/item_expressions.hpp"
#include "identifiers.hpp"

#include "expressions/operators.hpp"

namespace donkey{

inline lvalue_expression_ptr to_l(expression_ptr e){
	return std::static_pointer_cast<lvalue_expression>(e);
}

inline item_expression_ptr to_item(expression_ptr e){
	return std::static_pointer_cast<item_expression>(e);
}

template<template<typename> class E>
inline expression_ptr build_unary(expression_ptr e1){
	if(e1->get_type() == expression_type::lvalue){
		return expression_ptr(new E<lvalue_expression_ptr>(to_l(e1)));
	}else{
		return expression_ptr(new E<expression_ptr>(e1));
	}
}

template<class E>
inline expression_ptr build_unary(expression_ptr e1){
	return expression_ptr(new E(e1));
}

template<class E>
inline expression_ptr build_unary_l(expression_ptr e1){
	if(e1->get_type() != expression_type::lvalue){
		semantic_error("l-value expected");
	}
	return expression_ptr(new E(to_l(e1)));
}

template<class E>
inline expression_ptr build_unary_item(expression_ptr e1){
	return expression_ptr(new E(to_item(e1)));
}


template<template<typename, typename> class E>
inline expression_ptr build_binary(expression_ptr e1, expression_ptr e2){
	if(e1->get_type() == expression_type::lvalue){
		if(e2->get_type() == expression_type::lvalue){
			return expression_ptr(new E<lvalue_expression_ptr, lvalue_expression_ptr>(to_l(e1), to_l(e2)));
		}else{
			return expression_ptr(new E<lvalue_expression_ptr, expression_ptr>(to_l(e1), e2));
		}
	}else if(e2->get_type() == expression_type::lvalue){
		return expression_ptr(new E<expression_ptr, lvalue_expression_ptr>(e1, to_l(e2)));
	}else{
		return expression_ptr(new E<expression_ptr, expression_ptr>(e1, e2));
	}
}

template<class E>
inline expression_ptr build_binary(expression_ptr e1, expression_ptr e2){
	return expression_ptr(new E(e1, e2));
}

template<template<typename> class E>
inline expression_ptr build_binary_l(expression_ptr e1, expression_ptr e2){
	if(e1->get_type() == expression_type::lvalue){
		if(e2->get_type() == expression_type::lvalue){
			return expression_ptr(new E<lvalue_expression_ptr>(to_l(e1), to_l(e2)));
		}else{
			return expression_ptr(new E<expression_ptr>(to_l(e1), e2));
		}
	}else{
		semantic_error("l-value expected");
		return expression_ptr();
	}
}

template<class E>
inline expression_ptr build_binary_l(expression_ptr e1, expression_ptr e2){
	if(e1->get_type() != expression_type::lvalue){
		semantic_error("l-value expected");
	}
	return expression_ptr(new E(to_l(e1), e2));
}

template<template<typename> class E>
inline expression_ptr build_binary_item(expression_ptr e1, expression_ptr e2){
	if(e2->get_type() == expression_type::lvalue){
		return expression_ptr(new E<lvalue_expression_ptr>(to_item(e1), to_l(e2)));
	}else{
		return expression_ptr(new E<expression_ptr>(to_item(e1), e2));
	}
}

template<class E>
inline expression_ptr build_binary_item(expression_ptr e1, expression_ptr e2){
	return expression_ptr(new E(to_item(e1), e2));
}

template<class E>
inline expression_ptr build_ternary(expression_ptr e1, expression_ptr e2, expression_ptr e3){
	return expression_ptr(new E(e1, e2, e3));
}

expression_ptr build_unary_expression(oper op, expression_ptr e){
	if(e->get_type() == expression_type::item){
		switch(op){
			case oper::pre_dec:
				return build_unary_item<item_pre_dec_expression>(e);
			case oper::pre_inc:
				return build_unary_item<item_pre_inc_expression>(e);
			case oper::post_dec:
				return build_unary_item<item_post_dec_expression>(e);
			case oper::post_inc:
				return build_unary_item<item_post_inc_expression>(e);
			default:
				break;
		}
	}
	switch(op){
		case oper::logical_not:
			return build_unary<logical_not_expression>(e);
		case oper::bitwise_not:
			return build_unary<bitwise_not_expression>(e);
		case oper::unary_minus:
			return build_unary<unary_minus_expression>(e);
		case oper::unary_plus:
			return build_unary<unary_plus_expression>(e);
		case oper::pre_dec:
			return build_unary_l<pre_dec_expression>(e);
		case oper::pre_inc:
			return build_unary_l<pre_inc_expression>(e);
		case oper::post_dec:
			return build_unary_l<post_dec_expression>(e);
		case oper::post_inc:
			return build_unary_l<post_inc_expression>(e);
		case oper::deref:
			return build_unary<deref_expression>(e);
		default:
			return expression_ptr();
	}
}

expression_ptr build_binary_expression(oper op, expression_ptr e1, expression_ptr e2){
	if(e1->get_type() == expression_type::item){
		switch(op){
			case oper::fallback_assignment:
				return build_binary_item<item_fallback_assignment_expression>(e1, e2);
			case oper::or_assignment:
				return build_binary_item<item_assignment_expression>(e1, e2);
			case oper::xor_assignment:
				return build_binary_item<item_xor_assignment_expression>(e1, e2);
			case oper::and_assignment:
				return build_binary_item<item_and_assignment_expression>(e1, e2);
			case oper::shiftr_assignment:
				return build_binary_item<item_shiftr_assignment_expression>(e1, e2);
			case oper::shiftl_assignment:
				return build_binary_item<item_shiftl_assignment_expression>(e1, e2);
			case oper::concat_assignment:
				return build_binary_item<item_concat_assignment_expression>(e1, e2);
			case oper::minus_assignment:
				return build_binary_item<item_minus_assignment_expression>(e1, e2);
			case oper::plus_assignment:
				return build_binary_item<item_plus_assignment_expression>(e1, e2);
			case oper::mod_assignment:
				return build_binary_item<item_mod_assignment_expression>(e1, e2);
			case oper::idiv_assignment:
				return build_binary_item<item_idiv_assignment_expression>(e1, e2);
			case oper::div_assignment:
				return build_binary_item<item_div_assignment_expression>(e1, e2);
			case oper::mul_assignment:
				return build_binary_item<item_mul_assignment_expression>(e1, e2);
			case oper::assignment:
				return build_binary_item<item_assignment_expression>(e1, e2);
			default:
				break;
		}
	}
	switch(op){
		case oper::comma:
			return build_binary<comma_expression>(e1, e2);
		case oper::fallback_assignment:
			return build_binary_l<fallback_assignment_expression>(e1, e2);
		case oper::or_assignment:
			return build_binary_l<or_assignment_expression>(e1, e2);
		case oper::xor_assignment:
			return build_binary_l<xor_assignment_expression>(e1, e2);
		case oper::and_assignment:
			return build_binary_l<and_assignment_expression>(e1, e2);
		case oper::shiftr_assignment:
			return build_binary_l<shiftr_assignment_expression>(e1, e2);
		case oper::shiftl_assignment:
			return build_binary_l<shiftl_assignment_expression>(e1, e2);
		case oper::concat_assignment:
			return build_binary_l<concat_assignment_expression>(e1, e2);
		case oper::minus_assignment:
			return build_binary_l<minus_assignment_expression>(e1, e2);
		case oper::plus_assignment:
			return build_binary_l<plus_assignment_expression>(e1, e2);
		case oper::mod_assignment:
			return build_binary_l<mod_assignment_expression>(e1, e2);
		case oper::idiv_assignment:
			return build_binary_l<idiv_assignment_expression>(e1, e2);
		case oper::div_assignment:
			return build_binary_l<div_assignment_expression>(e1, e2);
		case oper::mul_assignment:
			return build_binary_l<mul_assignment_expression>(e1, e2);
		case oper::assignment:
			return build_binary_l<assignment_expression>(e1, e2);
		case oper::fallback:
			return build_binary<fallback_expression>(e1, e2);
		case oper::logical_or:
			return build_binary<logical_or_expression>(e1, e2);
		case oper::logical_and:
			return build_binary<logical_and_expression>(e1, e2);
		case oper::unequal:
			return build_binary<unequal_expression>(e1, e2);
		case oper::equal:
			return build_binary<equal_expression>(e1, e2);
		case oper::greater_equal:
			return build_binary<greater_equal_expression>(e1, e2);
		case oper::less_equal:
			return build_binary<less_equal_expression>(e1, e2);
		case oper::greater:
			return build_binary<greater_expression>(e1, e2);
		case oper::less:
			return build_binary<less_expression>(e1, e2);
		case oper::shiftr:
			return build_binary<shiftr_expression>(e1, e2);
		case oper::shiftl:
			return build_binary<shiftl_expression>(e1, e2);
		case oper::bitwise_or:
			return build_binary<bitwise_or_expression>(e1, e2);
		case oper::bitwise_xor:
			return build_binary<bitwise_xor_expression>(e1, e2);
		case oper::concat:
			return build_binary<concat_expression>(e1, e2);
		case oper::minus:
			return build_binary<minus_expression>(e1, e2);
		case oper::plus:
			return build_binary<plus_expression>(e1, e2);
		case oper::bitwise_and:
			return build_binary<bitwise_and_expression>(e1, e2);
		case oper::mod:
			return build_binary<mod_expression>(e1, e2);
		case oper::idiv:
			return build_binary<idiv_expression>(e1, e2);
		case oper::div:
			return build_binary<div_expression>(e1, e2);
		case oper::mul:
			return build_binary<mul_expression>(e1, e2);
		case oper::subscript:
			return build_binary<index_expression>(e1, e2);
		default:
			return expression_ptr();
	}
}


expression_ptr build_ternary_expression(oper op, expression_ptr e1, expression_ptr e2, expression_ptr e3){
	switch(op){
		case oper::conditional_question:
			return build_ternary<conditional_expression>(e1, e2, e3);
		default:
			return expression_ptr();
	}
}


expression_ptr build_function_call_expression(expression_ptr f, const std::vector<expression_ptr>& params, const std::vector<size_t>& byref){
	if(byref.empty()){
		return expression_ptr(new function_call_expression_byval(f, params));
	}else{
		return expression_ptr(new function_call_expression(f, params, byref));
	}
}


expression_ptr build_null_expression(){
	return expression_ptr(new null_expression());
}

expression_ptr build_this_expression(){
	return expression_ptr(new this_expression());
}

expression_ptr build_const_number_expression(double n){
	return expression_ptr(new const_number_expression(n));
}

expression_ptr build_const_string_expression(const std::string& str){
	return expression_ptr(new const_string_expression(str));
}

expression_ptr build_const_function_expression(code_address addr){
	return expression_ptr(new const_function_expression(addr));
}

expression_ptr build_local_variable_expression(size_t idx){
	return expression_ptr(new local_variable_expression(idx));
}

expression_ptr build_global_variable_expression(size_t module_idx, size_t var_idx){
	return expression_ptr(new global_variable_expression(module_idx, var_idx));
}

expression_ptr build_member_expression(expression_ptr that, const std::string& member){
	return expression_ptr(new member_expression(that, member));
}

expression_ptr build_field_expression(expression_ptr that, const std::string& name, size_t idx){
	return expression_ptr(new field_expression(that, name, idx));
}

expression_ptr build_method_expression(expression_ptr that, const std::string& name, method& m){
	return expression_ptr(new method_expression(that, name, m));
}

expression_ptr build_constructor_call_expression(vtable* vt, const std::vector<expression_ptr>& params){
	if(vt->is_native()){
		return expression_ptr(new creator_call_expression(vt, params));
	}else{
		return expression_ptr(new constructor_call_expression(vt, params));
	}
}

expression_ptr build_array_initializer(const std::vector<expression_ptr>& items){
	return expression_ptr(new array_creator_call_expression(items));
}

}//namespace donkey
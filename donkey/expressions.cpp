#include "expressions.hpp"
#include "unary_expressions.hpp"
#include "arithmetic_expressions.hpp"
#include "string_expressions.hpp"
#include "relation_expressions.hpp"
#include "logical_expressions.hpp"
#include "assignment_expressions.hpp"
#include "sequentional_expressions.hpp"
#include "ternary_expressions.hpp"
#include "identifiers.hpp"

namespace donkey{

inline expression_ptr identifier_to_expression(std::string str, const identifier_lookup& lookup){
	identifier_ptr id = lookup.get_identifier(str);
	if(!id){
		return expression_ptr();
	}
	switch(id->get_type()){
		case identifier_type::function:
			return expression_ptr(new const_function_expression(std::static_pointer_cast<function_identifier>(id)->get_function()));
		case identifier_type::global_variable:
			return expression_ptr(new global_variable_expression(std::static_pointer_cast<global_variable_identifier>(id)->get_index()));
		case identifier_type::local_variable:
			return expression_ptr(new local_variable_expression(std::static_pointer_cast<local_variable_identifier>(id)->get_index()));
		default:
			return expression_ptr();
	}
}

inline lvalue_expression_ptr to_l(expression_ptr e){
	return std::static_pointer_cast<lvalue_expression>(e);
}

template<class E>
inline expression_ptr build_unary(expression_ptr e1){
	return expression_ptr(new E(e1));
}

template<class E>
inline expression_ptr build_unary_l(expression_ptr e1){
	if(e1->get_type() != expression_type::lvalue){
		return expression_ptr();
	}
	return expression_ptr(new E(to_l(e1)));
}

template<class E>
inline expression_ptr build_binary(expression_ptr e1, expression_ptr e2){
	return expression_ptr(new E(e1, e2));
}

template<class E>
inline expression_ptr build_binary_l(expression_ptr e1, expression_ptr e2){
	if(e1->get_type() != expression_type::lvalue){
		return expression_ptr();
	}
	return expression_ptr(new E(to_l(e1), e2));
}

template<class E>
inline expression_ptr build_ternary(expression_ptr e1, expression_ptr e2, expression_ptr e3){
	return expression_ptr(new E(e1, e2, e3));
}

expression_ptr build_null_expression(){
	return expression_ptr(new null_expression());
}

expression_ptr build_number_expression(double d){
	return expression_ptr(new const_number_expression(d));
}

expression_ptr build_string_expression(std::string str){
	return expression_ptr(new const_string_expression(str));
}

expression_ptr build_variable_expression(std::string name, const identifier_lookup& lookup){
	return identifier_to_expression(name, lookup);
}

expression_ptr build_function_expression(std::string name, std::vector<expression_ptr>& params, const identifier_lookup& lookup){
	expression_ptr e = identifier_to_expression(name, lookup);
	if(e){
		return expression_ptr(new function_call_expression(e, params));
	}
	return expression_ptr();
}

expression_ptr build_unary_expression(oper op, expression_ptr e){
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
		default:
			return expression_ptr();
	}
}

expression_ptr build_binary_expression(oper op, expression_ptr e1, expression_ptr e2){
	switch(op){
		case oper::comma:
			return build_binary<comma_expression>(e1, e2);
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


}//namespace donkey
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
inline expression_ptr build_unary(std::stack<expression_ptr> stack){
	if(stack.empty()){
		return expression_ptr();
	}
	expression_ptr e1 = stack.top();
	stack.pop();
	return expression_ptr(new E(e1));
}

template<class E>
inline expression_ptr build_unary_l(std::stack<expression_ptr> stack){
	if(stack.empty()){
		return expression_ptr();
	}
	expression_ptr e1 = stack.top();
	stack.pop();
	if(e1->get_type() != expression_type::lvalue){
		return expression_ptr();
	}
	return expression_ptr(new E(to_l(e1)));
}

template<class E>
inline expression_ptr build_binary(std::stack<expression_ptr> stack){
	if(stack.size() < 2){
		return expression_ptr();
	}
	expression_ptr e2 = stack.top();
	stack.pop();
	expression_ptr e1 = stack.top();
	stack.pop();
	return expression_ptr(new E(e1, e2));
}

template<class E>
inline expression_ptr build_binary_l(std::stack<expression_ptr> stack){
	if(stack.size() < 2){
		return expression_ptr();
	}
	expression_ptr e2 = stack.top();
	stack.pop();
	expression_ptr e1 = stack.top();
	stack.pop();
	if(e1->get_type() != expression_type::lvalue){
		return expression_ptr();
	}
	return expression_ptr(new E(to_l(e1), e2));
}

template<class E>
inline expression_ptr build_ternary(std::stack<expression_ptr> stack){
	if(stack.size() < 3){
		return expression_ptr();
	}
	expression_ptr e3 = stack.top();
	stack.pop();
	expression_ptr e2 = stack.top();
	stack.pop();
	expression_ptr e1 = stack.top();
	stack.pop();
	return expression_ptr(new E(e1, e2, e3));
}

bool build_null_expression(std::stack<expression_ptr>& stack){
	stack.push(expression_ptr(new null_expression()));
	return true;
}

bool build_number_expression(std::stack<expression_ptr>& stack, double d){
	stack.push(expression_ptr(new const_number_expression(d)));
	return true;
}

bool build_string_expression(std::stack<expression_ptr>& stack, std::string str){
	stack.push(expression_ptr(new const_string_expression(str)));
	return true;
}

bool build_variable_expression(std::stack<expression_ptr>& stack, const identifier_lookup& lookup, std::string name){
	expression_ptr e = identifier_to_expression(name, lookup);
	if(e){
		stack.push(e);
		return true;
	}
	return false;
}

bool build_function_expression(std::stack<expression_ptr>& stack, const identifier_lookup& lookup, std::string name, int param_count){
	expression_ptr e = identifier_to_expression(name, lookup);
	if(e){
		std::vector<expression_ptr> params(param_count);
		for(int i = param_count - 1; i >= 0; --i){
			if(stack.empty()){
				return false;
			}
			params[i] = stack.top();
			stack.pop();
		}
		stack.push(expression_ptr(new function_call_expression(e, params)));
		return true;
	}
	return false;
}

bool build_operator_expression(std::stack<expression_ptr>& stack, oper op){
	expression_ptr e;
	switch(op){
		case oper::comma:
			e = build_binary<comma_expression>(stack);
			break;
		case oper::or_assignment:
			e = build_binary_l<or_assignment_expression>(stack);
			break;
		case oper::xor_assignment:
			e = build_binary_l<xor_assignment_expression>(stack);
			break;
		case oper::and_assignment:
			e = build_binary_l<and_assignment_expression>(stack);
			break;
		case oper::shiftr_assignment:
			e = build_binary_l<shiftr_assignment_expression>(stack);
			break;
		case oper::shiftl_assignment:
			e = build_binary_l<shiftl_assignment_expression>(stack);
			break;
		case oper::concat_assignment:
			e = build_binary_l<concat_assignment_expression>(stack);
			break;
		case oper::minus_assignment:
			e = build_binary_l<minus_assignment_expression>(stack);
			break;
		case oper::plus_assignment:
			e = build_binary_l<plus_assignment_expression>(stack);
			break;
		case oper::mod_assignment:
			e = build_binary_l<mod_assignment_expression>(stack);
			break;
		case oper::idiv_assignment:
			e = build_binary_l<idiv_assignment_expression>(stack);
			break;
		case oper::div_assignment:
			e = build_binary_l<div_assignment_expression>(stack);
			break;
		case oper::mul_assignment:
			e = build_binary_l<mul_assignment_expression>(stack);
			break;
		case oper::assignment:
			e = build_binary_l<assignment_expression>(stack);
			break;
		case oper::conditional_question:
			e = build_ternary<conditional_expression>(stack);
			break;
		case oper::logical_or:
			e = build_binary<logical_or_expression>(stack);
			break;
		case oper::logical_and:
			e = build_binary<logical_and_expression>(stack);
			break;
		case oper::unequal:
			e = build_binary<unequal_expression>(stack);
			break;
		case oper::equal:
			e = build_binary<equal_expression>(stack);
			break;
		case oper::greater_equal:
			e = build_binary<greater_equal_expression>(stack);
			break;
		case oper::less_equal:
			e = build_binary<less_equal_expression>(stack);
			break;
		case oper::greater:
			e = build_binary<greater_expression>(stack);
			break;
		case oper::less:
			e = build_binary<less_expression>(stack);
			break;
		case oper::shiftr:
			e = build_binary<shiftr_expression>(stack);
			break;
		case oper::shiftl:
			e = build_binary<shiftl_expression>(stack);
			break;
		case oper::bitwise_or:
			e = build_binary<bitwise_or_expression>(stack);
			break;
		case oper::bitwise_xor:
			e = build_binary<bitwise_xor_expression>(stack);
			break;
		case oper::concat:
			e = build_binary<concat_expression>(stack);
			break;
		case oper::minus:
			e = build_binary<minus_expression>(stack);
			break;
		case oper::plus:
			e = build_binary<plus_expression>(stack);
			break;
		case oper::bitwise_and:
			e = build_binary<bitwise_and_expression>(stack);
			break;
		case oper::mod:
			e = build_binary<mod_expression>(stack);
			break;
		case oper::idiv:
			e = build_binary<idiv_expression>(stack);
			break;
		case oper::div:
			e = build_binary<div_expression>(stack);
			break;
		case oper::mul:
			e = build_binary<mul_expression>(stack);
			break;
		case oper::logical_not:
			e = build_unary<logical_not_expression>(stack);
			break;
		case oper::bitwise_not:
			e = build_unary<bitwise_not_expression>(stack);
			break;
		case oper::unary_minus:
			e = build_unary<unary_minus_expression>(stack);
			break;
		case oper::unary_plus:
			e = build_unary<unary_plus_expression>(stack);
			break;
		case oper::pre_dec:
			e = build_unary_l<pre_dec_expression>(stack);
			break;
		case oper::pre_inc:
			e = build_unary_l<pre_inc_expression>(stack);
			break;
		case oper::post_dec:
			e = build_unary_l<post_dec_expression>(stack);
			break;
		case oper::post_inc:
			e = build_unary_l<post_inc_expression>(stack);
			break;
		default:
			e = expression_ptr();
			break;
	}
	if(e){
		stack.push(e);
		return true;
	}
	return false;
}


}//namespace donkey
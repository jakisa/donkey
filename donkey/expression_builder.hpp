#ifndef __expression_builder_hpp__
#define __expression_builder_hpp__

#include "expressions.hpp"
#include "identifiers.hpp"
#include "tokenizer.hpp"

namespace donkey{

namespace expression_builder{

enum class oper{
	param_comma,
	pre_inc,
	post_inc,
	pre_dec,
	post_dec,
	unary_plus,
	unary_minus,
	bitwise_not,
	logical_not,
	mul,
	div,
	idiv,
	mod,
	plus,
	minus,
	concat,
	shiftl,
	shiftr,
	less,
	greater,
	less_equal,
	greater_equal,
	equal,
	unequal,
	bitwise_and,
	bitwise_xor,
	bitwise_or,
	logical_and,
	logical_or,
	conditional,
	assignment,
	mul_assignment,
	div_assignment,
	idiv_assignment,
	mod_assignment,
	plus_assignment,
	minus_assignment,
	concat_assignment,
	shiftl_assignment,
	shiftr_assignment,
	and_assignment,
	xor_assignment,
	or_assignment,
	comma_expression
};

enum class part_type{
	ope,
	glb,
	loc,
	str,
	num,
	fun,
};

struct part{
	part_type type;
	std::string str;
	double num;
};

}//namespace expression_builder

inline expression_ptr build_expression(const identifier_lookup&, tokenizer& parser){
	
	return expression_ptr();
}

}//namespace donkey

#endif /* __expression_builder_hpp__ */

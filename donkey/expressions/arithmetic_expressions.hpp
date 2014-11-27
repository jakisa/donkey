#ifndef __arithmetic_expressions_hpp__
#define __arithmetic_expressions_hpp__

#include "expressions.hpp"
#include "operators.hpp"

namespace donkey{


NUMBER_BINARY_CPP(mul, mul, *)
NUMBER_BINARY_CPP(div, div, /)

variable idiv_full(const variable& l, const variable& r, runtime_context& ctx);
inline variable idiv(const variable& l, const variable& r, runtime_context& ctx) {
	if(l.get_var_type() == var_type::number && r.get_var_type() == var_type::number){
		return variable((integer)l.as_number_unsafe() / (integer)r.as_number_unsafe());
	}
	return idiv_full(l, r, ctx);
}

BIN_OPERATOR(idiv, idiv)

variable mod_full(const variable& l, const variable& r, runtime_context& ctx);
inline variable mod(const variable& l, const variable& r, runtime_context& ctx) {
	if(l.get_var_type() == var_type::number && r.get_var_type() == var_type::number){
		return variable(fmod(l.as_number_unsafe(), r.as_number_unsafe()));
	}
	return mod_full(l, r, ctx);
}
BIN_OPERATOR(mod, mod)

NUMBER_BINARY_CPP(plus, plus, +)
NUMBER_BINARY_CPP(minus, minus, -)
INTEGER_BINARY_CPP(shiftl, shiftl, <<)
INTEGER_BINARY_CPP(shiftr, shiftr, >>)
INTEGER_BINARY_CPP(bitwise_and, bitwise_and, &)
INTEGER_BINARY_CPP(bitwise_xor, bitwise_xor, ^)
INTEGER_BINARY_CPP(bitwise_or, bitwise_or, |)



}//donkey;

#endif /*__arithmetic_expressions_hpp__*/

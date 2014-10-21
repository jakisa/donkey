#ifndef __arithmetic_expressions_hpp__
#define __arithmetic_expressions_hpp__

#include "expressions.hpp"
#include "operators.hpp"

namespace donkey{


NUMBER_BINARY_CPP(mul, mul, *)
NUMBER_BINARY_CPP(div, div, /)

class idiv_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	idiv_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual number as_number(runtime_context& ctx) override{
		return (integer)_e1->as_number(ctx) / (integer)_e2->as_number(ctx);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
	
	virtual bool as_bool(runtime_context& ctx) override{
		return as_number(ctx) != 0;
	}
};

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

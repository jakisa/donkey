#ifndef __assignment_expressions_hpp__
#define __assignment_expressions_hpp__

#include "expressions.hpp"
#include "operators.hpp"

namespace donkey{

template<class E2>
class assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	E2 _e2;
public:
	assignment_expression(lvalue_expression_ptr e1, E2 e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable& as_lvalue(runtime_context& ctx) override{
		auto v = _e2->as_var(ctx);
		return _e1->as_lvalue(ctx) = v;
	}
};

NUMBER_BINARY_L_CPP(mul_assignment, mul_assign, *=)
NUMBER_BINARY_L_CPP(div_assignment, div_assign, /=)

class idiv_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	idiv_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable& as_lvalue(runtime_context& ctx) override{
		number n2 = _e2->as_number(ctx);
		variable& v1 = _e1->as_lvalue(ctx);
		number& n1 = v1.as_lnumber();
		n1 = integer(n1)/integer(n2);
		return v1;
	}
};

void mod_assign_full(variable& l, const variable& r, runtime_context& ctx);
inline void mod_assign(variable& l, const variable& r, runtime_context& ctx) {
	if(l.get_var_type() == var_type::number && r.get_var_type() == var_type::number){
		l.as_lnumber_unsafe() = fmod(l.as_number_unsafe(), r.as_number_unsafe());
	}
	mod_assign_full(l, r, ctx);
}
BIN_OPERATOR_L(mod_assignment, mod_assign)

NUMBER_BINARY_L_CPP(plus_assignment, plus_assign, +=)
NUMBER_BINARY_L_CPP(minus_assignment, minus_assign, -=)

INTEGER_BINARY_ASSIGN_CPP(shiftl_assignment, shiftl_assign, <<)
INTEGER_BINARY_ASSIGN_CPP(shiftr_assignment, shiftr_assign, >>)
INTEGER_BINARY_ASSIGN_CPP(and_assignment, bitwise_and_assign, &)
INTEGER_BINARY_ASSIGN_CPP(xor_assignment, bitwise_xor_assign, ^)
INTEGER_BINARY_ASSIGN_CPP(or_assignment, bitwise_or_assign, ^)


}//donkey

#endif /*__assignment_expressions_hpp__*/


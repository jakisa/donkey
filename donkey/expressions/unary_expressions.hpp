#ifndef __unary_expressions_hpp__
#define __unary_expressions_hpp__

#include "expressions.hpp"
#include "operators.hpp"

namespace donkey{

NUMBER_UNARY_L_CPP(pre_inc, pre_inc, ++)

POSTFIX_CPP(post_inc, post_inc, ++)

NUMBER_UNARY_L_CPP(pre_dec, pre_dec, --)

POSTFIX_CPP(post_dec, post_dec, --)

NUMBER_UNARY_CPP(unary_plus, u_plus, +)

NUMBER_UNARY_CPP(unary_minus, u_minus, -)

INTEGER_UNARY_CPP(bitwise_not, bitwise_not, ~)

class logical_not_expression final: public expression{
private:
	expression_ptr _e;
public:
	logical_not_expression(expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual number as_number(runtime_context& ctx) override{
		return as_bool(ctx) ? 1 : 0;
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_bool(ctx);
	}
	
	virtual bool as_bool(runtime_context& ctx) override{
		return !_e->as_bool(ctx);
	}
};

}//donkey

#endif /*__unary_expressions_hpp__*/

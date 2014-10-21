#ifndef __item_expressions_hpp__
#define __item_expressions_hpp__

#include "expressions.hpp"
#include "operators.hpp"
#include "assignment_expressions.hpp"
#include "unary_expressions.hpp"

namespace donkey{

class index_expression final: public item_expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	index_expression(expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}
	
	virtual item_handle as_item(runtime_context& ctx) override{
		return item_handle(_e1->as_param(ctx), _e2->as_param(ctx));
	}
};

ITEM_PRE_OPERATOR(item_pre_inc, pre_inc)
ITEM_POST_OPERATOR(item_post_inc, post_inc)
ITEM_PRE_OPERATOR(item_pre_dec, pre_dec)
ITEM_POST_OPERATOR(item_post_dec, post_dec)

class item_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		set_item(ctx, ret.first, ret.second, _e2->as_param(ctx));
		return ret;
	}
};

ITEM_ASSIGN_OPERATOR(item_mul_assignment, mul_assign)
ITEM_ASSIGN_OPERATOR(item_div_assignment, div_assign)

class item_idiv_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_idiv_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(integer(v.as_number()) / integer(_e2->as_number(ctx))));
		return ret;
	}
};

ITEM_ASSIGN_OPERATOR(item_mod_assignment, mod_assign)
ITEM_ASSIGN_OPERATOR(item_plus_assignment, plus_assign)
ITEM_ASSIGN_OPERATOR(item_minus_assignment, minus_assign)
ITEM_ASSIGN_OPERATOR(item_shiftl_assignment, shiftl_assign)
ITEM_ASSIGN_OPERATOR(item_shiftr_assignment, shiftr_assign)
ITEM_ASSIGN_OPERATOR(item_and_assignment, bitwise_and_assign)
ITEM_ASSIGN_OPERATOR(item_xor_assignment, bitwise_xor_assign)
ITEM_ASSIGN_OPERATOR(item_or_assignment, bitwise_or_assign)


}//donkey


#endif /* __item_expressions_hpp__ */

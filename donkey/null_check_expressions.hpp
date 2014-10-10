#ifndef __null_check_expressions_hpp__
#define __null_check_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class fallback_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	fallback_expression(expression_ptr e1, expression_ptr e2):
		expression(combined_expression_type(e1, e2)),
		_e1(e1),
		_e2(e2){
	}

	virtual number as_number(runtime_context& ctx) override{
		return as_param(ctx).as_number();
	}

	virtual variable as_param(runtime_context& ctx) override{
		variable v = _e1->as_param(ctx);
		if(v.get_data_type() == var_type::nothing){
			return _e2->as_param(ctx);
		}
		return v;
	}

	virtual void as_void(runtime_context& ctx) override{
		as_param(ctx);
	}
};


class fallback_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	fallback_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable& as_lvalue(runtime_context& ctx) override{
		variable& v = _e1->as_lvalue(ctx);
		if(v.get_data_type() == var_type::nothing){
			v = _e2->as_param(ctx);
		}
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

}//donkey

#endif /* __null_check_expressions_hpp__ */

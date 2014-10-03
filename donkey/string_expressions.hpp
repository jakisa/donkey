#ifndef __string_expressions_hpp__
#define __string_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class concat_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	concat_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::string),
		_e1(e1),
		_e2(e2){
	}

	virtual std::string as_string(runtime_context& ctx) override{
		return _e1->as_string(ctx) + _e2->as_string(ctx);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_string(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_string(ctx);
	}
};

class concat_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	concat_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}
	
	virtual variable& as_lvalue(runtime_context& ctx) override{
		variable& v = _e1->as_lvalue(ctx);
		v = variable(v.to_string() + _e2->as_string(ctx));
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

}//donkey

#endif /*__string_expressions_hpp__*/

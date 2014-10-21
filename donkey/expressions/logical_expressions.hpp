#ifndef __logical_expressions_hpp__
#define __logical_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class logical_and_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	logical_and_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
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
		return _e1->as_bool(ctx) && _e2->as_bool(ctx);
	}
};

class logical_or_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	logical_or_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
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
		return _e1->as_bool(ctx) || _e2->as_bool(ctx);
	}
};

}//donkey

#endif /*__logical_expressions_hpp__*/
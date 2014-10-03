#ifndef __relation_expressions_hpp__
#define __relation_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class less_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	less_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_param(ctx) < _e2->as_param(ctx);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class greater_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	greater_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_param(ctx) > _e2->as_param(ctx);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class less_equal_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	less_equal_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_param(ctx) <= _e2->as_param(ctx);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class greater_equal_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	greater_equal_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_param(ctx) >= _e2->as_param(ctx);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class equal_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	equal_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_param(ctx) == _e2->as_param(ctx);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class unequal_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	unequal_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_param(ctx) != _e2->as_param(ctx);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};


}//donkey

#endif /*__relation_expressions_hpp__*/

#ifndef __arithmetic_expressions_hpp__
#define __arithmetic_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class mul_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	mul_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) * _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class div_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	div_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) / _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

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

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) / (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class mod_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	mod_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return fmod(_e1->as_number(ctx),_e2->as_number(ctx));
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class plus_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	plus_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) + _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class minus_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	minus_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) - _e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class shiftl_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	shiftl_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) << (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class shiftr_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	shiftr_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) >> (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class bitwise_and_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	bitwise_and_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) & (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class bitwise_xor_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	bitwise_xor_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) ^ (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class bitwise_or_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	bitwise_or_expression(expression_ptr e1, expression_ptr e2):
		expression(expression_type::number),
		_e1(e1),
		_e2(e2){
	}

	virtual double as_number(runtime_context& ctx) override{
		return (int)_e1->as_number(ctx) | (int)_e2->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};


}//donkey;

#endif /*__arithmetic_expressions_hpp__*/

#ifndef __unary_expressions_hpp__
#define __unary_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class pre_inc_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e;
public:
	pre_inc_expression(lvalue_expression_ptr e):
		_e(e){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e->as_lvalue(ctx);
		++get_lnumber(v);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class post_inc_expression final: public expression{
private:
	lvalue_expression_ptr _e;
public:
	post_inc_expression(lvalue_expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override final{
		return get_lnumber(_e->as_lvalue(ctx))++;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override final{
		return std::make_shared<number_variable>(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override final{
		as_number(ctx);
	}
};

class pre_dec_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e;
public:
	pre_dec_expression(lvalue_expression_ptr e):
		_e(e){
	}

	virtual variable_ptr& as_lvalue(runtime_context& ctx) override{
		variable_ptr& v = _e->as_lvalue(ctx);
		--get_lnumber(v);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class post_dec_expression final: public expression{
private:
	lvalue_expression_ptr _e;
public:
	post_dec_expression(lvalue_expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override final{
		return get_lnumber(_e->as_lvalue(ctx))--;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override final{
		return std::make_shared<number_variable>(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override final{
		as_number(ctx);
	}
};

class unary_plus_expression final: public expression{
private:
	expression_ptr _e;
public:
	unary_plus_expression(expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return std::make_shared<number_variable>(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class unary_minus_expression final: public expression{
private:
	expression_ptr _e;
public:
	unary_minus_expression(expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override{
		return -_e->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return std::make_shared<number_variable>(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class bitwise_not_expression final: public expression{
private:
	expression_ptr _e;
public:
	bitwise_not_expression(expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override{
		return ~int(_e->as_number(ctx));
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return std::make_shared<number_variable>(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class logical_not_expression final: public expression{
private:
	expression_ptr _e;
public:
	logical_not_expression(expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual double as_number(runtime_context& ctx) override{
		return !_e->as_number(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return std::make_shared<number_variable>(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

}//donkey

#endif /*__unary_expressions_hpp__*/

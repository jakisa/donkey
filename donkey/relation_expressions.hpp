#ifndef __relation_expressions_hpp__
#define __relation_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

inline bool less_than(expression_ptr e1, expression_ptr e2, runtime_context& ctx){
	if(e1->get_type() == expression_type::function || e2->get_type() == expression_type::function){
		runtime_error("cannot compare const functions");
	}
	if(is_variant_expression(e1)){
		if(is_variant_expression(e2)){
			return variable::less(get_variable(e1, ctx), get_variable(e2, ctx));
		}
		if(e2->get_type() == expression_type::number){
			return variable::less(get_variable(e1, ctx), e2->as_number(ctx));
		}
		return variable::less(get_variable(e1, ctx), e2->as_string(ctx));
	}
	if(is_variant_expression(e2)){
		if(e1->get_type() == expression_type::number){
			return variable::greater(get_variable(e2, ctx), e1->as_number(ctx));
		}
		return variable::greater(get_variable(e2, ctx), e1->as_string(ctx));
	}
	if(e1->get_type() == expression_type::number){
		if(e2->get_type() == expression_type::number){
			return e1->as_number(ctx) < e2->as_number(ctx);
		}

		runtime_error("cannot compare number and string");
		return false;
	}

	if(e2->get_type() == expression_type::string){
		return e1->as_string(ctx) < e2->as_string(ctx);
	}
	runtime_error("cannot compare string and number");
	return false;
}

inline bool equal_to(expression_ptr e1, expression_ptr e2, runtime_context& ctx){
	if(e1->get_type() == expression_type::function || e2->get_type() == expression_type::function){
		runtime_error("cannot compare const functions");
	}
	if(is_variant_expression(e1)){
		if(is_variant_expression(e2)){
			return variable::equals(get_variable(e1, ctx), get_variable(e2, ctx));
		}
		if(e2->get_type() == expression_type::number){
			return variable::equals(get_variable(e1, ctx), e2->as_number(ctx));
		}
		return variable::equals(get_variable(e1, ctx), e2->as_string(ctx));
	}
	if(is_variant_expression(e2)){
		if(e1->get_type() == expression_type::number){
			return variable::equals(get_variable(e2, ctx), e1->as_number(ctx));
		}
		return variable::equals(get_variable(e2, ctx), e1->as_string(ctx));
	}
	if(e1->get_type() == expression_type::number){
		if(e2->get_type() == expression_type::number){
			return e1->as_number(ctx) == e2->as_number(ctx);
		}
		return false;
	}

	if(e2->get_type() == expression_type::string){
		return e1->as_string(ctx) == e2->as_string(ctx);
	}
	return false;
}


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
		return less_than(_e1, _e2, ctx) ? 1 : 0;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
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
		return less_than(_e2, _e1, ctx) ? 1 : 0;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
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
		return less_than(_e2, _e1, ctx) ? 0 : 1;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
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
		return less_than(_e1, _e2, ctx) ? 0 : 1;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
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
		return equal_to(_e1, _e2, ctx) ? 1 : 0;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
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
		return equal_to(_e1, _e2, ctx) ? 0 : 1;
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new number_variable(as_number(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};


}//donkey

#endif /*__relation_expressions_hpp__*/

#ifndef __ternary_expressions_hpp__
#define __ternary_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

inline expression_type combined_expression_type(expression_ptr e1, expression_ptr e2){
	if(is_variant_expression(e1) || is_variant_expression(e2)){
		return expression_type::variant;
	}
	if(e1->get_type() == expression_type::function && e2->get_type() == expression_type::function){
		return expression_type::function;
	}
	if(e1->get_type() == expression_type::number && e2->get_type() == expression_type::number){
		return expression_type::number;
	}
	return expression_type::string;
}

class conditional_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
	expression_ptr _e3;
public:
	conditional_expression(expression_ptr e1, expression_ptr e2, expression_ptr e3):
		expression(combined_expression_type(e2, e3)),
		_e1(e1),
		_e2(e2),
		_e3(e3){
	}

	virtual double as_number(runtime_context& ctx) override{
		return _e1->as_number(ctx) ? _e2->as_number(ctx) : _e3->as_number(ctx);
	}
	
	virtual std::string as_string(runtime_context &ctx) override{
		return _e1->as_number(ctx) ? _e2->as_string(ctx) : _e3->as_string(ctx);
	}
	
	virtual function as_function(runtime_context &ctx) override{
		return _e1->as_number(ctx) ? _e2->as_function(ctx) : _e3->as_function(ctx);
	}

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return _e1->as_number(ctx) ? _e2->as_param(ctx) : _e3->as_param(ctx);
	}

	virtual void as_void(runtime_context& ctx) override{
		_e1->as_number(ctx) ? _e2->as_void(ctx) : _e3->as_void(ctx);
	}
};

}//donkey

#endif /*__ternary_expressions_hpp__*/

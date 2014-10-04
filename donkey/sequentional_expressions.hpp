#ifndef __sequentional_expressions_hpp__
#define __sequentional_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class comma_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	comma_expression(expression_ptr e1, expression_ptr e2):
		expression((e2->get_type() == expression_type::lvalue) ? expression_type::variant : e2->get_type()),
		_e1(e1),
		_e2(e2){
	}

	virtual number as_number(runtime_context& ctx) override{
		return _e1->as_void(ctx), _e2->as_number(ctx);
	}
	
	virtual std::string as_string(runtime_context &ctx) override{
		return _e1->as_void(ctx), _e2->as_string(ctx);
	}
	
	virtual code_address as_function(runtime_context &ctx) override{
		return _e1->as_void(ctx), _e2->as_function(ctx);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return _e1->as_void(ctx), _e2->as_param(ctx);
	}

	virtual void as_void(runtime_context& ctx) override{
		_e1->as_void(ctx), _e2->as_void(ctx);
	}
};

}//donkey

#endif /*__sequentional_expressions_hpp__*/

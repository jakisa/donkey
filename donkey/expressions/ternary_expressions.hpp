#ifndef __ternary_expressions_hpp__
#define __ternary_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

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

	virtual number as_number(runtime_context& ctx) override{
		return _e1->as_bool(ctx) ? _e2->as_number(ctx) : _e3->as_number(ctx);
	}
	
	virtual std::string as_string(runtime_context &ctx) override{
		return _e1->as_bool(ctx) ? _e2->as_string(ctx) : _e3->as_string(ctx);
	}
	
	virtual variable call(runtime_context& ctx, size_t params_size) override{
		return _e1->as_bool(ctx) ? _e2->call(ctx, params_size) : _e3->call(ctx, params_size);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return _e1->as_bool(ctx) ? _e2->as_param(ctx) : _e3->as_param(ctx);
	}

	virtual void as_void(runtime_context& ctx) override{
		_e1->as_bool(ctx) ? _e2->as_void(ctx) : _e3->as_void(ctx);
	}
	
	virtual bool as_bool(runtime_context& ctx) override{
		return _e1->as_bool(ctx) ? _e2->as_bool(ctx) : _e3->as_bool(ctx);
	}
};

}//donkey

#endif /*__ternary_expressions_hpp__*/

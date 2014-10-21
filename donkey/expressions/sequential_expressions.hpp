#ifndef __sequential_expressions_hpp__
#define __sequential_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class comma_expression final: public expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	comma_expression(expression_ptr e1, expression_ptr e2):
		expression(is_variant_expression(e2) ? expression_type::variant : e2->get_type()),
		_e1(e1),
		_e2(e2){
	}

	virtual number as_number(runtime_context& ctx) override{
		return _e1->as_void(ctx), _e2->as_number(ctx);
	}
	
	virtual std::string as_string(runtime_context &ctx) override{
		return _e1->as_void(ctx), _e2->as_string(ctx);
	}
	
	virtual variable call(runtime_context& ctx, size_t params_size) override{
		_e1->as_void(ctx);
		return _e2->call(ctx, params_size);
	}

	virtual variable as_param(runtime_context& ctx) override{
		return _e1->as_void(ctx), _e2->as_param(ctx);
	}

	virtual void as_void(runtime_context& ctx) override{
		_e1->as_void(ctx), _e2->as_void(ctx);
	}
	
	virtual bool as_bool(runtime_context& ctx) override{
		return _e1->as_void(ctx), _e2->as_bool(ctx); 
	}
};

}//donkey

#endif /*__sequential_expressions_hpp__*/

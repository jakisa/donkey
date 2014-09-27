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

	virtual variable_ptr as_param(runtime_context& ctx) override{
		return variable_ptr(new string_variable(as_string(ctx)));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_string(ctx);
	}
};

}//donkey

#endif /*__string_expressions_hpp__*/

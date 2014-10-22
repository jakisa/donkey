#ifndef __null_check_expressions_hpp__
#define __null_check_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

template<class E1, class E2>
class fallback_expression final: public expression{
private:
	E1 _e1;
	E2 _e2;
public:
	fallback_expression(E1 e1, E2 e2):
		expression(combined_expression_type(e1, e2)),
		_e1(e1),
		_e2(e2){
	}

	virtual number as_number(runtime_context& ctx) override{
		return as_param(ctx).as_number();
	}

	virtual variable as_param(runtime_context& ctx) override{
		auto v = _e1->as_var(ctx);
		if(v.get_data_type() == var_type::nothing){
			return _e2->as_var(ctx);
		}
		return v;
	}

	virtual void as_void(runtime_context& ctx) override{
		as_param(ctx);
	}
	
	virtual bool as_bool(runtime_context& ctx) override{
		return as_param(ctx).to_bool(ctx);
	}
};

template<class E2>
class fallback_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	E2 _e2;
public:
	fallback_assignment_expression(lvalue_expression_ptr e1, E2 e2):
		_e1(e1),
		_e2(e2){
	}

	virtual variable& as_lvalue(runtime_context& ctx) override{
		auto v2 = _e2->as_param(ctx);
		
		variable& v1 = _e1->as_lvalue(ctx);
		if(v1.get_data_type() == var_type::nothing){
			v1 = v2;
		}
		return v1;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

template<class E>
class item_fallback_assignment_expression final: public item_expression<typename handle_version<E>::type>{
private:
	typedef typename handle_version<E>::type handle;
	E _e1;
	expression_ptr _e2;
public:
	item_fallback_assignment_expression(E e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual handle as_item(runtime_context& ctx) override{
		handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.that, variable(ret.index));
		if(v.get_data_type() == var_type::nothing){
			set_item(ctx, ret.that, variable(ret.index), _e2->as_param(ctx));
		}
		return ret;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.that, variable(ret.index));
		if(v.get_data_type() == var_type::nothing){
			set_item(ctx, ret.that, std::move(ret.index), _e2->as_param(ctx));
		}
	}
};

}//donkey

#endif /* __null_check_expressions_hpp__ */

#ifndef __item_expressions_hpp__
#define __item_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class index_expression final: public item_expression{
private:
	expression_ptr _e1;
	expression_ptr _e2;
public:
	index_expression(expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}
	
	virtual item_handle as_item(runtime_context& ctx) override{
		return item_handle(_e1->as_param(ctx), _e2->as_param(ctx));
	}	
};

class item_pre_inc_expression final: public item_expression{
private:
	item_expression_ptr _e;
public:
	item_pre_inc_expression(item_expression_ptr e):
		_e(e){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(v.as_number()+1));
		return ret;
	}
};

class item_post_inc_expression final: public expression{
private:
	item_expression_ptr _e;
public:
	item_post_inc_expression(item_expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual number as_number(runtime_context& ctx) override{
		item_handle ret = _e->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(v.as_number()+1));
		return v.as_number();
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class item_pre_dec_expression final: public item_expression{
private:
	item_expression_ptr _e;
public:
	item_pre_dec_expression(item_expression_ptr e):
		_e(e){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(v.as_number()-1));
		return ret;
	}
};

class item_post_dec_expression final: public expression{
private:
	item_expression_ptr _e;
public:
	item_post_dec_expression(item_expression_ptr e):
		expression(expression_type::number),
		_e(e){
	}

	virtual number as_number(runtime_context& ctx) override{
		item_handle ret = _e->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(v.as_number()-1));
		return v.as_number();
	}

	virtual variable as_param(runtime_context& ctx) override{
		return variable(as_number(ctx));
	}

	virtual void as_void(runtime_context& ctx) override{
		as_number(ctx);
	}
};

class item_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		set_item(ctx, ret.first, ret.second, _e2->as_param(ctx));
		return ret;
	}
};

class item_mul_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_mul_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(v.as_number() * _e2->as_number(ctx)));
		return ret;
	}
};

class item_div_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_div_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(v.as_number() / _e2->as_number(ctx)));
		return ret;
	}
};

class item_idiv_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_idiv_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(integer(v.as_number()) / integer(_e2->as_number(ctx))));
		return ret;
	}
};

class item_mod_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_mod_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(fmod(v.as_number(), _e2->as_number(ctx))));
		return ret;
	}
};

class item_plus_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_plus_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(v.as_number() + _e2->as_number(ctx)));
		return ret;
	}
};

class item_minus_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_minus_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(v.as_number() - _e2->as_number(ctx)));
		return ret;
	}
};

class item_shiftl_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_shiftl_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(integer(v.as_number()) << integer(_e2->as_number(ctx))));
		return ret;
	}
};

class item_shiftr_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_shiftr_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(integer(v.as_number()) >> integer(_e2->as_number(ctx))));
		return ret;
	}
};

class item_and_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_and_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(integer(v.as_number()) & integer(_e2->as_number(ctx))));
		return ret;
	}
};

class item_xor_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_xor_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(integer(v.as_number()) ^ integer(_e2->as_number(ctx))));
		return ret;
	}
};

class item_or_assignment_expression final: public item_expression{
private:
	item_expression_ptr _e1;
	expression_ptr _e2;
public:
	item_or_assignment_expression(item_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual item_handle as_item(runtime_context& ctx) override{
		item_handle ret = _e1->as_item(ctx);
		variable v = get_item(ctx, ret.first, ret.second);
		set_item(ctx, ret.first, ret.second, variable(integer(v.as_number()) | integer(_e2->as_number(ctx))));
		return ret;
	}
};


}//donkey


#endif /* __item_expressions_hpp__ */

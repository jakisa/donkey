#ifndef __operators_hpp__
#define __operators_hpp__

#include "variables.hpp"
#include "expressions.hpp"

namespace donkey{

#define UN_OPERATOR(name, op)\
template<class E>\
class name##_expression final: public expression{\
private:\
	E _e;\
public:\
	name##_expression(E e):\
		expression(expression_type::variant),\
		_e(e){\
	}\
\
	virtual number as_number(runtime_context& ctx) override{\
		return as_param(ctx).as_number();\
	}\
\
	virtual variable as_param(runtime_context& ctx) override{\
		return op(_e->as_var(ctx), ctx);\
	}\
\
	virtual void as_void(runtime_context& ctx) override{\
		as_param(ctx);\
	}\
	virtual bool as_bool(runtime_context& ctx) override{\
		return as_param(ctx).to_bool(ctx);\
	}\
\
};

#define UN_OPERATOR_L(name, op)\
class name##_expression final: public lvalue_expression{\
private:\
	lvalue_expression_ptr _e;\
public:\
	name##_expression(lvalue_expression_ptr e):\
		_e(e){\
	}\
\
	virtual variable& as_lvalue(runtime_context& ctx) override{\
		variable& v = _e->as_lvalue(ctx);\
		op(v, ctx);\
		return v;\
	}\
};

#define POST_OPERATOR(name, op)\
class name##_expression final: public expression{\
private:\
	lvalue_expression_ptr _e;\
public:\
	name##_expression(lvalue_expression_ptr e):\
		expression(expression_type::variant),\
		_e(e){\
	}\
\
	virtual number as_number(runtime_context& ctx) override{\
		return as_param(ctx).as_number();\
	}\
\
	virtual variable as_param(runtime_context& ctx) override{\
		return op(_e->as_lvalue(ctx), ctx);\
	}\
\
	virtual void as_void(runtime_context& ctx) override{\
		as_param(ctx);\
	}\
	virtual bool as_bool(runtime_context& ctx) override{\
		return as_param(ctx).to_bool(ctx);\
	}\
\
};

#define BIN_OPERATOR(name, op)\
template<class E1, class E2>\
class name##_expression final: public expression{\
private:\
	E1 _e1;\
	E2 _e2;\
public:\
	name##_expression(E1 e1, E2 e2):\
		expression(expression_type::variant),\
		_e1(e1),\
		_e2(e2){\
	}\
\
	virtual number as_number(runtime_context& ctx) override{\
		return as_param(ctx).as_number();\
	}\
\
	virtual variable as_param(runtime_context& ctx) override{\
		return op(_e1->as_var(ctx), _e2->as_var(ctx), ctx);\
	}\
\
	virtual void as_void(runtime_context& ctx) override{\
		as_param(ctx);\
	}\
	virtual bool as_bool(runtime_context& ctx) override{\
		return as_param(ctx).to_bool(ctx);\
	}\
};

#define BIN_OPERATOR_L(name, op)\
template<class E2>\
class name##_expression final: public lvalue_expression{\
private:\
	lvalue_expression_ptr _e1;\
	E2 _e2;\
public:\
	name##_expression(lvalue_expression_ptr e1, E2 e2):\
		_e1(e1),\
		_e2(e2){\
	}\
\
	virtual variable& as_lvalue(runtime_context& ctx) override{\
		auto v2 = _e2->as_var(ctx);\
		variable& v1 = _e1->as_lvalue(ctx);\
		op(v1, v2, ctx);\
		return v1;\
	}\
};

#define ITEM_PRE_OPERATOR(name, op)\
class name##_expression final: public item_expression{\
private:\
	item_expression_ptr _e;\
public:\
	name##_expression(item_expression_ptr e):\
		_e(e){\
	}\
\
	virtual item_handle as_item(runtime_context& ctx) override{\
		item_handle ret = _e->as_item(ctx);\
		variable v = get_item(ctx, ret.first, ret.second);\
		op(v, ctx);\
		set_item(ctx, ret.first, ret.second, std::move(v));\
		return ret;\
	}\
};

#define ITEM_POST_OPERATOR(name, op)\
class name##_expression final: public expression{\
private:\
	item_expression_ptr _e;\
public:\
	name##_expression(item_expression_ptr e):\
		expression(expression_type::variant),\
		_e(e){\
	}\
\
	virtual number as_number(runtime_context& ctx) override{\
		return as_param(ctx).as_number();\
	}\
\
	virtual variable as_param(runtime_context& ctx) override{\
		item_handle handle = _e->as_item(ctx);\
		variable v = get_item(ctx, handle.first, handle.second);\
		variable ret = op(v, ctx);\
		set_item(ctx, handle.first, handle.second, std::move(v));\
		return ret;\
	}\
\
	virtual void as_void(runtime_context& ctx) override{\
		as_param(ctx);\
	}\
\
	virtual bool as_bool(runtime_context& ctx) override{\
		return as_param(ctx).to_bool(ctx);\
	}\
};

#define ITEM_ASSIGN_OPERATOR(name, op)\
template<class E2>\
class name##_expression final: public item_expression{\
private:\
	item_expression_ptr _e1;\
	E2 _e2;\
public:\
	name##_expression(item_expression_ptr e1, E2 e2):\
		_e1(e1),\
		_e2(e2){\
	}\
\
	virtual item_handle as_item(runtime_context& ctx) override{\
		item_handle ret = _e1->as_item(ctx);\
		variable v = get_item(ctx, ret.first, ret.second);\
		op(v, _e2->as_var(ctx), ctx);\
\
		set_item(ctx, ret.first, ret.second, std::move(v));\
		return ret;\
	}\
};

#define NUMBER_UNARY_CPP(name, op, cpp)\
\
variable op##_full(const variable& l, runtime_context& ctx);\
\
inline variable op(const variable& l, runtime_context& ctx){\
	if(l.get_var_type() == var_type::number){\
		return variable(cpp l.as_number_unsafe());\
	}\
	return op##_full(l, ctx);\
}\
UN_OPERATOR(name, op)

#define INTEGER_UNARY_CPP(name, op, cpp)\
\
variable op##_full(const variable& l, runtime_context& ctx);\
\
inline variable op(const variable& l, runtime_context& ctx){\
	if(l.get_var_type() == var_type::number){\
		return variable(cpp l.as_integer_unsafe());\
	}\
	return op##_full(l, ctx);\
}\
UN_OPERATOR(name, op)

#define NUMBER_UNARY_L_CPP(name, op, cpp)\
\
void op##_full(variable& l, runtime_context& ctx);\
\
inline void op(variable& l, runtime_context& ctx){\
	if(l.get_var_type() == var_type::number){\
		cpp l.as_lnumber_unsafe();\
	}else{\
		op##_full(l, ctx);\
	}\
}\
UN_OPERATOR_L(name, op)

#define POSTFIX_CPP(name, op, cpp)\
\
variable op##_full(const variable& l, runtime_context& ctx);\
\
inline variable op(variable& l, runtime_context& ctx){\
	if(l.get_var_type() == var_type::number){\
		return variable(l.as_lnumber_unsafe() cpp);\
	}\
	return op##_full(l, ctx);\
}\
POST_OPERATOR(name, op)

#define NUMBER_BINARY_CPP(name, op, cpp)\
\
variable op##_full(const variable& l, const variable& r, runtime_context& ctx);\
\
inline variable op(const variable& l, const variable& r, runtime_context& ctx) {\
	if(l.get_var_type() == var_type::number && r.get_var_type() == var_type::number){\
		return variable(l.as_number_unsafe() cpp r.as_number_unsafe());\
	}\
	return op##_full(l, r, ctx);\
}\
BIN_OPERATOR(name, op)\

#define INTEGER_BINARY_CPP(name, op, cpp)\
\
variable op##_full(const variable& l, const variable& r, runtime_context& ctx);\
\
inline variable op(const variable& l, const variable& r, runtime_context& ctx) {\
	if(l.get_var_type() == var_type::number && r.get_var_type() == var_type::number){\
		return variable(l.as_integer_unsafe() cpp r.as_integer_unsafe());\
	}\
	return op##_full(l, r, ctx);\
}\
BIN_OPERATOR(name, op)

#define NUMBER_BINARY_L_CPP(name, op, cpp)\
\
void op##_full(variable& l, const variable& r, runtime_context& ctx);\
\
inline void op(variable& l, const variable& r, runtime_context& ctx) {\
	if(l.get_var_type() == var_type::number && r.get_var_type() == var_type::number){\
		l.as_lnumber_unsafe() cpp r.as_number_unsafe();\
	}else{\
		op##_full(l, r, ctx);\
	}\
}\
BIN_OPERATOR_L(name, op)\

#define INTEGER_BINARY_ASSIGN_CPP(name, op, cpp)\
\
void op##_full(variable& l, const variable& r, runtime_context& ctx);\
\
inline void op(variable& l, const variable& r, runtime_context& ctx) {\
	if(l.get_var_type() == var_type::number && r.get_var_type() == var_type::number){\
		l.as_lnumber_unsafe() = l.as_integer_unsafe() cpp r.as_integer_unsafe();\
	}else{\
		op##_full(l, r, ctx);\
	}\
}\
BIN_OPERATOR_L(name, op)\



}//donkey


#endif /* __operators_hpp__ */

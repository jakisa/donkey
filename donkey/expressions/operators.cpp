#include "operators.hpp"

#include "variables.hpp"

namespace donkey{

static void not_defined_error(const std::string& op, const std::string& type){
	return runtime_error(op + " is not defined for " + type);
}

#define CHECK_DEFINED(tbl, o)\
if(!tbl->op##o){\
	not_defined_error("op"#o, tbl->get_full_name());\
}

#define INVOKE_OPERATOR(tbl, o, that, cnt)\
(*tbl->op##o)(that, ctx, cnt)

#define UNARY_SIMPLE(name, o)\
variable name##_full(const variable& l, runtime_context& ctx){\
	vtable* vt = l.get_vtable();\
	CHECK_DEFINED(vt, o)\
	return INVOKE_OPERATOR(vt, o, l, 0);\
}

#define UNARY_LVALUE(name, o)\
void name##_full(variable& l, runtime_context& ctx){\
	vtable* vt = l.get_vtable();\
	CHECK_DEFINED(vt, o)\
	INVOKE_OPERATOR(vt, o, l, 0);\
}

#define BINARY_SIMPLE(name, o)\
variable name##_full(const variable& l, const variable& r, runtime_context& ctx){\
	vtable* vt = l.get_vtable();\
	CHECK_DEFINED(vt, o)\
	stack_pusher pusher(ctx, 1);\
	pusher.push(variable(r));\
	return INVOKE_OPERATOR(vt, o, l, 1);\
}

#define BINARY_DOUBLE(name, o)\
variable name##_full(const variable& l, const variable& r, runtime_context& ctx){\
	vtable* lvt = l.get_vtable();\
	if(!lvt->op##o){\
		vtable* rvt = r.get_vtable();\
		if(!rvt->op##o##Inv){\
			not_defined_error("op"#o, lvt->get_full_name());\
		}\
		stack_pusher pusher(ctx, 1);\
		pusher.push(variable(l));\
		return INVOKE_OPERATOR(rvt, o##Inv, r, 1);\
	}else{\
		stack_pusher pusher(ctx, 1);\
		pusher.push(variable(r));\
		return INVOKE_OPERATOR(lvt, o, l, 1);\
	}\
}

#define BINARY_LVALUE(name, o)\
void name##_full(variable& l, const variable& r, runtime_context& ctx){\
	vtable* vt = l.get_vtable();\
	CHECK_DEFINED(vt, o)\
	stack_pusher pusher(ctx, 1);\
	pusher.push(variable(r));\
	INVOKE_OPERATOR(vt, o, l, 1);\
}


BINARY_SIMPLE(eq, EQ)
BINARY_SIMPLE(ne, NE)
BINARY_DOUBLE(lt, LT)
BINARY_DOUBLE(gt, GT)
BINARY_DOUBLE(le, LE)
BINARY_DOUBLE(ge, GE)

UNARY_SIMPLE(post_inc, PostInc)
UNARY_SIMPLE(post_dec, PostDec)
UNARY_SIMPLE(u_plus, Pos)
UNARY_SIMPLE(u_minus, Neg)
UNARY_SIMPLE(bitwise_not, Not)

UNARY_LVALUE(pre_inc, PreInc)
UNARY_LVALUE(pre_dec, PreDec)

BINARY_DOUBLE(mul, Mul)
BINARY_DOUBLE(div, Div)
BINARY_DOUBLE(mod, Mod)
BINARY_DOUBLE(plus, Add)
BINARY_DOUBLE(minus, Sub)
BINARY_DOUBLE(shiftl, SL)
BINARY_DOUBLE(shiftr, SR)
BINARY_DOUBLE(bitwise_and, And)
BINARY_DOUBLE(bitwise_or, Or)
BINARY_DOUBLE(bitwise_xor, Xor)

BINARY_LVALUE(mul_assign, MulSet)
BINARY_LVALUE(div_assign, DivSet)
BINARY_LVALUE(mod_assign, ModSet)
BINARY_LVALUE(plus_assign, AddSet)
BINARY_LVALUE(minus_assign, SubSet)
BINARY_LVALUE(shiftl_assign, SLSet)
BINARY_LVALUE(shiftr_assign, SRSet)
BINARY_LVALUE(bitwise_and_assign, AndSet)
BINARY_LVALUE(bitwise_or_assign, OrSet)
BINARY_LVALUE(bitwise_xor_assign, XorSet)


}//donkey


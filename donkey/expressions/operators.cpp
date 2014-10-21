#include "operators.hpp"

#include "variables.hpp"

namespace donkey{

#define UNARY_SIMPLE(name, op)\
variable name##_full(const variable& l, runtime_context& ctx){\
	return l.get_vtable()->call_member(l, ctx, 1, "op"#op);\
}

#define UNARY_LVALUE(name, op)\
void name##_full(variable& l, runtime_context& ctx){\
	l.get_vtable()->call_member(l, ctx, 1, "op"#op);\
}

#define BINARY_SIMPLE(name, op)\
variable name##_full(const variable& l, const variable& r, runtime_context& ctx){\
	stack_pusher pusher(ctx);\
	pusher.push(variable(r));\
\
	return l.get_vtable()->call_member(l, ctx, 1, "op"#op);\
}

#define BINARY_DOUBLE(name, op)\
variable name##_full(const variable& l, const variable& r, runtime_context& ctx){\
	variable ret;\
	{\
		stack_pusher pusher(ctx);\
		pusher.push(variable(r));\
		if(l.get_vtable()->try_call_member(l, ctx, 1, "op"#op, ret)){\
			return ret;\
		}\
	}\
	{\
		stack_pusher pusher(ctx);\
		pusher.push(variable(l));\
		if(r.get_vtable()->try_call_member(r, ctx, 1, "op"#op"Inv", ret)){\
			return ret;\
		}\
	}\
\
	runtime_error(std::string("op"#op) + " is not defined for " + l.get_full_type_name());\
	return variable();\
}

#define BINARY_LVALUE(name, op)\
void name##_full(variable& l, const variable& r, runtime_context& ctx){\
	stack_pusher pusher(ctx);\
	pusher.push(variable(r));\
\
	l.get_vtable()->call_member(l, ctx, 1, "op"#op);\
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


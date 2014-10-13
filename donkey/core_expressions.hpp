#ifndef __core_expressions_hpp__
#define __core_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class null_expression final: public expression{
public:
	null_expression():
		expression(expression_type::variant){
	}
	virtual std::string as_string(runtime_context& ctx) override{
		return as_param(ctx).to_string();
	}
	virtual variable as_param(runtime_context&) override{
		return variable();
	}
	virtual void as_void(runtime_context&) override{
	}
};

class this_expression final: public expression{
public:
	this_expression():
		expression(expression_type::variant){
	}
	virtual std::string as_string(runtime_context& ctx) override{
		return as_param(ctx).to_string();
	}
	virtual variable as_param(runtime_context& ctx) override{
		return *ctx.that();
	}
	virtual void as_void(runtime_context&) override{
	}
};


class const_number_expression final: public expression{
private:
	variable _d;
public:
	const_number_expression(number d):
		expression(expression_type::number),
		_d(d){
	}
	virtual number as_number(runtime_context&) override{
		return _d.as_number_unsafe();
	}
	virtual variable as_param(runtime_context&) override{
		return _d;
	}

	virtual void as_void(runtime_context&) override{
	}
};

class const_string_expression final: public expression{
private:
	variable _s;
public:
	const_string_expression(std::string s):
		expression(expression_type::string),
		_s(s){
	}
	virtual std::string as_string(runtime_context&) override{
		return _s.to_string();
	}
	virtual variable as_param(runtime_context&) override{
		return _s;
	}

	virtual void as_void(runtime_context&) override{
	}
};

class const_function_expression final: public expression{
private:
	code_address _f;
public:
	const_function_expression(code_address f):
		expression(expression_type::function),
		_f(f){
	}
	virtual variable call(runtime_context& ctx, variable* params, size_t params_size) override{
		return call_function_by_address(_f, ctx, params, params_size);
	}
	virtual std::string as_string(runtime_context&) override{
		return "function";
	}
	virtual variable as_param(runtime_context&) override{
		return variable(_f);
	}

	virtual void as_void(runtime_context&) override{
	}
};


class local_variable_expression final: public lvalue_expression{
private:
	int _idx;
public:
	local_variable_expression(int idx):
		_idx(idx){
	}

	virtual variable& as_lvalue(runtime_context& ctx) override{
		return ctx.local(_idx);
	}

	virtual void as_void(runtime_context&) override{
	}
};

class parameter_expression final: public lvalue_expression{
private:
	int _idx;
public:
	parameter_expression(int idx):
		_idx(idx){
	}

	virtual variable& as_lvalue(runtime_context& ctx) override{
		return ctx.param(_idx);
	}

	virtual void as_void(runtime_context&) override{
	}
};

class global_variable_expression final: public lvalue_expression{
private:
	int _idx;
public:
	global_variable_expression(int idx):
		_idx(idx){
	}
	virtual variable& as_lvalue(runtime_context & ctx) override{
		return ctx.global(_idx);
	}

	virtual void as_void(runtime_context&) override{
	}
};

class member_expression final: public lvalue_expression{
private:
	std::string _name;
	expression_ptr _that;
public:
	member_expression(expression_ptr that, std::string name):
		_name(name),
		_that(that){
	}
	
	virtual variable& as_lvalue(runtime_context& ctx) override{
		variable that = _that->as_param(ctx);
		return get_vtable(ctx, that)->get_field(that, _name);
	}
	
	virtual variable call(runtime_context &ctx, variable* params, size_t params_size) override{
		variable that = _that->as_param(ctx);
		return get_vtable(ctx, that)->call_member(that, ctx, params, params_size, _name);
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_param(ctx);
	}
	
	virtual number as_number(runtime_context& ctx) override{
		return as_lvalue(ctx).as_number();
	}
	
	virtual std::string as_string(runtime_context& ctx) override{
		return as_lvalue(ctx).to_string();
	}
	
	virtual variable as_param(runtime_context& ctx) override{
		return as_lvalue(ctx);
	}
};

class field_expression final: public lvalue_expression{
private:
	expression_ptr _that;
	std::string _type;
	int _idx;
public:
	field_expression(expression_ptr that, std::string type, int idx):
		_that(that),
		_type(type),
		_idx(idx){
	}

	virtual variable& as_lvalue(runtime_context& ctx) override{
		variable that = _that->as_param(ctx);
		return get_vtable(ctx, that)->get_field(that, _type, _idx);
	}

	virtual void as_void(runtime_context&) override{
	}
};

class method_expression final: public expression{
private:
	expression_ptr _that;
	std::string _type;
	method& _m;
public:
	method_expression(expression_ptr that, const std::string& type, method& m):
		expression(expression_type::function),
		_that(that),
		_type(type),
		_m(m){
	}
	virtual variable call(runtime_context& ctx, variable* params, size_t params_size) override{
		variable that = _that->as_param(ctx);
		return get_vtable(ctx, that)->call_method(that, ctx, params, params_size, _type, _m);
	}
	virtual std::string as_string(runtime_context&) override{
		runtime_error("methods cannot be used as objects");
		return "";
	}
	virtual variable as_param(runtime_context&) override{
		runtime_error("methods cannot be used as objects");
		return variable();
	}

	virtual void as_void(runtime_context&) override{
		runtime_error("methods cannot be used as objects");
	}
};
}//donkey

#endif /*__core_expressions_hpp__*/

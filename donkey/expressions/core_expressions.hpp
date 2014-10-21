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
	
	virtual bool as_bool(runtime_context& ctx) override{
		return as_param(ctx).to_bool(ctx);
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
	
	virtual bool as_bool(runtime_context& ctx) override{
		return ctx.that()->to_bool(ctx);
	}
};

class deref_expression final: public item_expression{
private:
	expression_ptr _e;
public:
	deref_expression(expression_ptr e):
		_e(e){
	}
	
	virtual item_handle as_item(runtime_context& ctx) override{
		return item_handle(_e->as_param(ctx), variable(number(0)));
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
	
	virtual bool as_bool(runtime_context& ctx) override{
		return as_number(ctx) != 0;
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
	
	virtual bool as_bool(runtime_context& ctx) override{
		return _s.to_bool(ctx);
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
	virtual variable call(runtime_context& ctx, size_t params_size) override{
		return call_function_by_address(_f, ctx, params_size);
	}
	virtual std::string as_string(runtime_context&) override{
		return "function";
	}
	virtual variable as_param(runtime_context&) override{
		return variable(_f);
	}

	virtual void as_void(runtime_context&) override{
	}
	
	virtual bool as_bool(runtime_context& ctx) override{
		return as_param(ctx).to_bool(ctx);
	}
};


class local_variable_expression final: public lvalue_expression{
private:
	size_t _idx;
public:
	local_variable_expression(size_t idx):
		_idx(idx){
	}

	virtual variable& as_lvalue(runtime_context& ctx) override{
		return ctx.local(_idx);
	}
};

class global_variable_expression final: public lvalue_expression{
private:
	uint32_t _module_idx;
	uint32_t _var_idx;
public:
	global_variable_expression(uint32_t module_idx, uint32_t var_idx):
		_module_idx(module_idx),
		_var_idx(var_idx){
	}
	virtual variable& as_lvalue(runtime_context & ctx) override{
		return global_variable(ctx, _module_idx, _var_idx);
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
	
	virtual variable call(runtime_context &ctx, size_t params_size) override{
		variable that = _that->as_param(ctx);
		return get_vtable(ctx, that)->call_member(that, ctx, params_size, _name);
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
	virtual variable call(runtime_context& ctx, size_t params_size) override{
		variable that = _that->as_param(ctx);
		return get_vtable(ctx, that)->call_method(that, ctx, params_size, _type, _m);
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
	
	virtual bool as_bool(runtime_context&) override{
		runtime_error("methods cannot be used as objects");
		return false;
	}
};
}//donkey

#endif /*__core_expressions_hpp__*/

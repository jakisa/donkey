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

template<class E>
class deref_expression final: public item_expression<typename handle_version<E>::type>{
private:
	typedef typename handle_version<E>::type handle;
	E _e;
public:
	deref_expression(E e):
		_e(e){
	}
	
	virtual handle as_item(runtime_context& ctx) override{
		return handle(_e->as_var(ctx), variable(number(0)));
	}
	
	virtual void as_void(runtime_context& ctx) override{
		_e->as_void(ctx);
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
	vtable* _vt;
	method* _m;
	size_t _f;
	
	void update_member(const variable& that){
		vtable* vt = that.get_vtable();
		
		if(vt != _vt){
			if(vt->has_field(_name)){
				_f = vt->get_field_index(_name);
				_m = nullptr;
			}else if(vt->has_method(_name)){
				_f = size_t(-1);
				_m = vt->get_method(_name).get();
			}
			_vt = vt;
		}
	}
	
public:
	member_expression(expression_ptr that, std::string name):
		_name(name),
		_that(that),
		_vt(nullptr),
		_m(nullptr),
		_f(size_t(-1)){
	}
	
	virtual variable& as_lvalue(runtime_context& ctx) override{
		variable that = _that->as_param(ctx);
		update_member(that);
		if(_f == size_t(-1)){
			runtime_error("field " + _name + " is not defined for " + that.get_full_type_name());
		}
		return that.nth_field(_f);
	}
	
	virtual variable call(runtime_context &ctx, size_t params_size) override{
		variable that = _that->as_param(ctx);
		update_member(that);
		
		if(_m){
			return (*_m)(that, ctx, params_size);
		}else if(_f != size_t(-1)){
			return that.nth_field(_f).call(ctx, params_size);
		}else{
			runtime_error("member " + _name + " is not defined for " + that.get_full_type_name());
			return variable();
		}
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
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
		return that.get_vtable()->get_field(that, _type, _idx);
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
		return that.get_vtable()->call_method(that, ctx, params_size, _type, _m);
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

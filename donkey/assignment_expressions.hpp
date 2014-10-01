#ifndef __assignment_expressions_hpp__
#define __assignment_expressions_hpp__

#include "expressions.hpp"

namespace donkey{

class assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		return _e1->as_lvalue(ctx) = _e2->as_param(ctx);
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class mul_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	mul_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		v.as_lnumber() *= _e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class div_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	div_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		v.as_lnumber() /= _e2->as_number(ctx);
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class idiv_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	idiv_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		double& n = v.as_lnumber();
		n = int(n) / int(_e2->as_number(ctx));
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class mod_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	mod_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		double& n = v.as_lnumber();
		n = fmod(n, _e2->as_number(ctx));
		return v;
	}
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class plus_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	plus_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		v.as_lnumber() += _e2->as_number(ctx);
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class minus_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	minus_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		v.as_lnumber() -= _e2->as_number(ctx);
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};


class shiftl_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	shiftl_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		double& n = v.as_lnumber();
		n = int(n) << int(_e2->as_number(ctx));
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class shiftr_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	shiftr_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		double& n = v.as_lnumber();
		n = int(n) >> int(_e2->as_number(ctx));
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class and_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	and_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		double& n = v.as_lnumber();
		n = int(n) & int(_e2->as_number(ctx));
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class xor_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	xor_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		double& n = v.as_lnumber();
		n = int(n) ^ int(_e2->as_number(ctx));
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

class or_assignment_expression final: public lvalue_expression{
private:
	lvalue_expression_ptr _e1;
	expression_ptr _e2;
public:
	or_assignment_expression(lvalue_expression_ptr e1, expression_ptr e2):
		_e1(e1),
		_e2(e2){
	}

	virtual stack_var& as_lvalue(runtime_context& ctx) override{
		stack_var& v = _e1->as_lvalue(ctx);
		double& n = v.as_lnumber();
		n = int(n) | int(_e2->as_number(ctx));
		return v;
	}
	
	virtual void as_void(runtime_context& ctx) override{
		as_lvalue(ctx);
	}
};

}//donkey

#endif /*__assignment_expressions_hpp__*/


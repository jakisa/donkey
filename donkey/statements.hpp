#ifndef __statements_hpp__
#define __statements_hpp__

#include "runtime_context.hpp"
#include "expressions.hpp"
#include <vector>
#include <unordered_map>

namespace donkey{

enum class statement_retval{
	nxt,
	brk,
	cnt,
	ret,
};

typedef std::function<statement_retval(runtime_context&)> statement;

inline statement_retval empty_statement(runtime_context&){
	return statement_retval::nxt;
}

class expression_statement{
private:
	expression_ptr _e;
public:
	expression_statement(expression_ptr e):
		_e(e){
	}
	expression_statement(expression_statement&& orig):
		_e(orig._e){
	}
	expression_statement(const expression_statement& orig):
		_e(orig._e){
	}
	statement_retval operator()(runtime_context& ctx) const{
		_e->as_void(ctx);
		return statement_retval::nxt;
	}
};

class vars_block_statement{
private:
	std::vector<statement> _ss;
	size_t _locals_count;
public:
	vars_block_statement(vars_block_statement&& orig):
		_ss(std::move(orig._ss)),
		_locals_count(orig._locals_count){
	}
	
	vars_block_statement(const vars_block_statement& orig):
		_ss(orig._ss),
		_locals_count(orig._locals_count){
	}
	
	vars_block_statement(std::vector<statement>&& ss, int locals_count):
		_ss(ss),
		_locals_count(locals_count){
	}
	statement_retval operator()(runtime_context& ctx) const{
		stack_pusher pusher(ctx, _locals_count);
		
		pusher.push_default(_locals_count);
		
		for(const statement& s: _ss){
			statement_retval r = s(ctx);
			if(r != statement_retval::nxt){
				return r;
			}
		}
		return statement_retval::nxt;
	}
};

class block_statement{
private:
	std::vector<statement> _ss;
public:
	block_statement(block_statement&& orig):
		_ss(std::move(orig._ss)){
	}
	
	block_statement(const block_statement& orig):
		_ss(orig._ss){
	}
	
	block_statement(std::vector<statement>&& ss):
		_ss(ss){
	}
	statement_retval operator()(runtime_context& ctx) const{
		for(const statement& s: _ss){
			statement_retval r = s(ctx);
			if(r != statement_retval::nxt){
				return r;
			}
		}
		return statement_retval::nxt;
	}
};

class for_statement{
private:
	expression_ptr _e1;
	expression_ptr _e2;
	expression_ptr _e3;
	statement _s;
public:
	for_statement(for_statement&& orig):
		_e1(orig._e1),
		_e2(orig._e2),
		_e3(orig._e3),
		_s(std::move(orig._s)){
	}
	for_statement(const for_statement& orig):
		_e1(orig._e1),
		_e2(orig._e2),
		_e3(orig._e3),
		_s(orig._s){
	}
	for_statement(expression_ptr e1, expression_ptr e2, expression_ptr e3, statement&& s):
		_e1(e1),
		_e2(e2),
		_e3(e3),
		_s(s){
	}
	statement_retval operator()(runtime_context& ctx) const{
		for(_e1->as_void(ctx); _e2->as_bool(ctx); _e3->as_void(ctx)){
			switch(_s(ctx)){
				case statement_retval::brk:
					return statement_retval::nxt;
				case statement_retval::ret:
					return statement_retval::ret;
				default:
					break;
			}
		}
		return statement_retval::nxt;
	}
};

class while_statement{
private:
	expression_ptr _e;
	statement _s;
public:
	while_statement(while_statement&& orig):
		_e(orig._e),
		_s(std::move(orig._s)){
	}
	while_statement(const while_statement& orig):
		_e(orig._e),
		_s(orig._s){
	}
	while_statement(expression_ptr e, statement&& s):
		_e(e),
		_s(s){
	}
	statement_retval operator()(runtime_context& ctx) const{
		while(_e->as_bool(ctx)){
			switch(_s(ctx)){
				case statement_retval::brk:
					return statement_retval::nxt;
				case statement_retval::ret:
					return statement_retval::ret;
				default:
					break;
			}
		}
		return statement_retval::nxt;
	}
};

class do_statement{
private:
	expression_ptr _e;
	statement _s;
public:
	do_statement(do_statement&& orig):
		_e(orig._e),
		_s(std::move(orig._s)){
	}
	do_statement(const do_statement& orig):
		_e(orig._e),
		_s(orig._s){
	}
	do_statement(expression_ptr e, statement&& s):
		_e(e),
		_s(s){
	}
	statement_retval operator()(runtime_context& ctx) const{
		do{
			switch(_s(ctx)){
				case statement_retval::brk:
					return statement_retval::nxt;
				case statement_retval::ret:
					return statement_retval::ret;
				default:
					break;
			}
		}while(_e->as_bool(ctx));
		return statement_retval::nxt;
	}
};

class if_statement{
private:
	std::vector<expression_ptr> _es;
	std::vector<statement> _ss;
public:
	if_statement(if_statement&& orig):
		_es(std::move(orig._es)),
		_ss(std::move(orig._ss)){
	}
	if_statement(const if_statement& orig):
		_es(orig._es),
		_ss(orig._ss){
	}
	if_statement(std::vector<expression_ptr>&& es, std::vector<statement>&& ss):
		_es(es),
		_ss(ss){
	}
	
	statement_retval operator()(runtime_context& ctx) const{
		for(size_t i = 0; i < _es.size(); ++i){
			if(_es[i]->as_bool(ctx)){
				return _ss[i](ctx);
			}
		}
		return _ss.back()(ctx);
	}
};

class simple_if_statement{
private:
	expression_ptr _e;
	statement _s;
public:
	simple_if_statement(simple_if_statement&& orig):
		_e(orig._e),
		_s(std::move(orig._s)){
	}
	simple_if_statement(const simple_if_statement& orig):
		_e(orig._e),
		_s(orig._s){
	}
	simple_if_statement(expression_ptr e, statement&& s):
		_e(e),
		_s(std::move(s)){
	}
	
	statement_retval operator()(runtime_context& ctx) const{
		if(_e->as_bool(ctx)){
			return _s(ctx);
		}
		return statement_retval::nxt;
	}
};

class if_else_statement{
private:
	expression_ptr _e;
	statement _s;
	statement _else;
public:
	if_else_statement(if_else_statement&& orig):
		_e(orig._e),
		_s(std::move(orig._s)),
		_else(std::move(orig._else)){
	}
	if_else_statement(const if_else_statement& orig):
		_e(orig._e),
		_s(orig._s),
		_else(orig._else){
	}
	if_else_statement(expression_ptr e, statement&& s, statement&& els):
		_e(e),
		_s(std::move(s)),
		_else(std::move(els)){
	}
	
	statement_retval operator()(runtime_context& ctx) const{
		if(_e->as_bool(ctx)){
			return _s(ctx);
		}
		_else(ctx);
		return statement_retval::nxt;
	}
};

class switch_statement{
private:
	expression_ptr _e;
	std::vector<statement> _ss;
	std::unordered_map<number, size_t> _cases;
	size_t _dflt;
public:
	switch_statement(switch_statement&& orig):
		_e(orig._e),
		_ss(std::move(orig._ss)),
		_cases(std::move(orig._cases)),
		_dflt(orig._dflt){
	}
	switch_statement(const switch_statement& orig):
		_e(orig._e),
		_ss(orig._ss),
		_cases(orig._cases),
		_dflt(orig._dflt){
	}
	switch_statement(expression_ptr e, std::vector<statement>&& ss, std::unordered_map<number, size_t>&& cases, size_t dflt):
		_e(e),
		_ss(ss),
		_cases(cases),
		_dflt(dflt){
	}
	
	statement_retval operator()(runtime_context& ctx) const{
		size_t idx = _ss.size();
		auto it = _cases.find(_e->as_number(ctx));
		if(it != _cases.end()){
			idx = it->second;
		}else{
			idx = _dflt;
		}
		
		for(; idx < _ss.size(); ++idx){
			switch(_ss[idx](ctx)){
				case statement_retval::brk:
					return statement_retval::nxt;
				case statement_retval::ret:
					return statement_retval::ret;
				default:
					break;
			}
		}
		return statement_retval::nxt;
	}
};

inline statement_retval break_statement(runtime_context&){
	return statement_retval::brk;
}

inline statement_retval continue_statement(runtime_context&){
	return statement_retval::cnt;
}

class return_statement{
private:
	expression_ptr _e;
public:
	return_statement(expression_ptr e):
		_e(e){
	}
	return_statement(return_statement&& orig):
		_e(orig._e){
	}
	return_statement(const return_statement& orig):
		_e(orig._e){
	}
	statement_retval operator()(runtime_context& ctx) const{
		ctx.set_retval(_e->as_param(ctx));
		return statement_retval::ret;
	}
};

class base_constructor_statement{
private:
	vtable* _vt;
	std::vector<expression_ptr> _params;
public:
	base_constructor_statement(vtable* vt, std::vector<expression_ptr>&& params):
		_vt(vt),
		_params(std::move(params)){
	}
	base_constructor_statement(base_constructor_statement&& orig):
		_vt(orig._vt),
		_params(std::move(orig._params)){
	}
	base_constructor_statement(const base_constructor_statement& orig):
		_vt(orig._vt),
		_params(orig._params){
	}
	statement_retval operator()(runtime_context& ctx) const{
		stack_pusher pusher(ctx, _params.size());
		for(size_t i = 0; i != _params.size(); ++i){
			pusher.push(_params[i]->as_param(ctx));
		}
		_vt->call_base_constructor(*ctx.that(), ctx, _params.size());
		
		return statement_retval::nxt;
	}
};

class base_default_constructor_statement{
private:
	vtable* _vt;
public:
	base_default_constructor_statement(vtable* vt):
		_vt(vt){
	}
	base_default_constructor_statement(base_default_constructor_statement&& orig):
		_vt(orig._vt){
	}
	base_default_constructor_statement(const base_default_constructor_statement& orig):
		_vt(orig._vt){
	}
	statement_retval operator()(runtime_context& ctx) const{
		_vt->call_base_constructor(*ctx.that(), ctx, 0);
		
		return statement_retval::nxt;
	}
};

class base_destructor_statement{
private:
	vtable* _vt;
public:
	base_destructor_statement(vtable* vt):
		_vt(vt){
	}
	base_destructor_statement(base_destructor_statement&& orig):
		_vt(orig._vt){
	}
	base_destructor_statement(const base_destructor_statement& orig):
		_vt(orig._vt){
	}
	statement_retval operator()(runtime_context& ctx) const{
		_vt->call_base_destructor(*ctx.that(), ctx);
		
		return statement_retval::nxt;
	}
};

}//donkey

#endif /*__statements_hpp__*/

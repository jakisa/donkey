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
	statement_retval operator()(runtime_context& ctx) const{
		_e->as_void(ctx);
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
	block_statement(std::vector<statement>& ss):
		_ss(std::move(ss)){
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
	for_statement(expression_ptr e1, expression_ptr e2, expression_ptr e3, statement& s):
		_e1(e1),
		_e2(e2),
		_e3(e3),
		_s(std::move(s)){
	}
	statement_retval operator()(runtime_context& ctx) const{
		for(_e1->as_void(ctx); _e2->as_number(ctx); _e3->as_void(ctx)){
			switch(_s(ctx)){
				case statement_retval::brk:
					return statement_retval::nxt;
				case statement_retval::ret:
					return statement_retval::ret;
				default:
					break;
			}
		}
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
	while_statement(expression_ptr e, statement& s):
		_e(e),
		_s(std::move(s)){
	}
	statement_retval operator()(runtime_context& ctx) const{
		while(_e->as_number(ctx)){
			switch(_s(ctx)){
				case statement_retval::brk:
					return statement_retval::nxt;
				case statement_retval::ret:
					return statement_retval::ret;
				default:
					break;
			}
		}
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
	do_statement(expression_ptr e, statement& s):
		_e(e),
		_s(std::move(s)){
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
		}while(_e->as_number(ctx));
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
	if_statement(std::vector<expression_ptr>& es, std::vector<statement>& ss):
		_es(std::move(es)),
		_ss(std::move(ss)){
	}
	
	statement_retval operator()(runtime_context& ctx) const{
		for(size_t i = 0; i < _es.size(); ++i){
			if(_es[i]->as_number(ctx)){
				return _ss[i](ctx);
			}
		}
		if(_ss.size() > _es.size()){
			return _ss.back()(ctx);
		}
		return statement_retval::nxt;
	}
};

class switch_statement{
private:
	expression_ptr _e;
	std::vector<statement> _ss;
	std::unordered_map<double, size_t> _cases;
	size_t _dflt;
public:
	switch_statement(switch_statement&& orig):
		_e(orig._e),
		_ss(std::move(orig._ss)),
		_cases(std::move(orig._cases)),
		_dflt(orig._dflt){
	}
	switch_statement(expression_ptr e, std::vector<statement>& ss, std::unordered_map<double, size_t>& cases, size_t dflt):
		_e(e),
		_ss(std::move(ss)),
		_cases(std::move(cases)),
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
	statement_retval operator()(runtime_context& ctx) const{
		if(_e){
			ctx.top() = _e->as_param(ctx);
		}
		return statement_retval::ret;
	}
};

}//donkey

#endif /*__statements_hpp__*/

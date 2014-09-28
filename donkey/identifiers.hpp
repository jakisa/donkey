#ifndef __identifiers_hpp__
#define __identifiers_hpp__

#include "runtime_context.hpp"

namespace donkey{

enum class identifier_type{
	global_variable,
	local_variable,
	function
};

class identifier{
	identifier(const identifier&) = delete;
	void operator=(const identifier&) = delete;
private:
	identifier_type _t;
protected:
	identifier(identifier_type t):
		_t(t){
	}
public:
	identifier_type get_type() const{
		return _t;
	}
	
	virtual ~identifier(){
	}
};

typedef std::shared_ptr<identifier> identifier_ptr;

class global_variable_identifier: public identifier{
	int _idx;
public:
	global_variable_identifier(int idx):
		identifier(identifier_type::global_variable),
		_idx(idx){
	}
	
	int get_index() const{
		return _idx;
	}
};

class local_variable_identifier: public identifier{
	int _idx;
public:
	local_variable_identifier(int idx):
		identifier(identifier_type::local_variable),
		_idx(idx){
	}
	
	int get_index() const{
		return _idx;
	}
};

class function_identifier: public identifier{
	function _f;
public:
	function_identifier(function f = function()):
		identifier(identifier_type::function),
		_f(std::move(f)){
	}
	
	function get_function() const{
		return _f;
	}
	
	void set_function(function f){
		_f = std::move(f);
	}
	
	bool is_empty() const{
		return (bool)_f;
	}
};

typedef std::shared_ptr<function_identifier> function_identifier_ptr;

class identifier_lookup{
	identifier_lookup(const identifier_lookup&) = delete;
	void operator=(const identifier_lookup&) = delete;
protected:
	identifier_lookup(){
	}
public:
	virtual identifier_ptr get_identifier(std::string name) const = 0;
	
	virtual ~identifier_lookup(){
	}
};


}//namespace donkey

#endif /* __identifiers_hpp__*/

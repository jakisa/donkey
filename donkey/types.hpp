#ifndef __types_hpp__
#define __types_hpp__

#include <memory>
#include <vector>
#include <string>
#include <algorithm>

namespace donkey{

class type{
	type(const type&) = delete;
	void operator=(const type&) = delete;
protected:
	type(){
	}
public:
	virtual bool is_void() const{
		return false;
	}
	virtual bool is_simple() const{
		return false;
	}
	virtual bool is_array() const{
		return false;
	}
	virtual bool is_map() const{
		return false;
	}
	virtual bool is_function() const{
		return false;
	}
	virtual bool is_class() const{
		return false;
	}
	virtual ~type(){
	}
};

class type_void: public type{
public:
	type_void(){
	}
	virtual bool is_void() const override{
		return true;
	}
};

enum struct simple_type{
	number,
	string
};

class type_simple: public type{
private:
	simple_type _st;
public:
	type_simple(simple_type st):
		_st(st){
	}
	virtual bool is_simple() const override{
		return true;
	}
	simple_type get_simple_type() const{
		return _st;
	}
};

class type_array: public type{
private:
	const type* _et;
public:
	type_array(const type* et):
		_et(et){
	}
	virtual bool is_simple() const override{
		return true;
	}
	const type* get_element_type() const{
		return _et;
	}
};

class type_map: public type{
private:
	const type* _et;
public:
	type_map(const type* et):
		_et(et){
	}
	virtual bool is_map() const override{
		return true;
	}
	const type* get_element_type() const{
		return _et;
	}
};

enum struct ref_type{
	value,
	ref,
	cref,
	fun
};

class var_type{
private:
	ref_type _rt;
	const type* _t;
	std::string _name;
public:
	var_type(ref_type rt, const type* t, const char* name = ""):
		_rt(rt),
		_t(t),
		_name(name){
	}

	ref_type get_ref_type() const{
		return _rt;
	}
	const type* get_type() const{
		return _t;
	}
	const std::string& get_name() const{
		return _name;
	}
};

class var_type_comparator{
public:
	bool operator()(const var_type& l, const var_type& r) const{
		return l.get_name() < r.get_name();
	}

	bool operator()(const var_type& l, const char* r) const{
		return l.get_name() < r;
	}

	bool operator()(const char* l, const var_type& r) const{
		return l < r.get_name();
	}
};


class type_function: public type{
private:
	var_type _ret;
	std::vector<var_type> _params;
public:
	type_function(var_type ret, std::initializer_list<var_type> params):
		_ret(ret),
		_params(params){
	}

	virtual bool is_function() const override{
		return true;
	}

	const var_type& get_ret() const{
		return _ret;
	}

	int get_param_count() const{
		return _params.size();
	}

	const var_type& get_param(int idx) const{
		return _params[idx];
	}
};

class type_class: public type{
private:
	std::vector<var_type> _fields;
public:
	type_class(std::initializer_list<var_type> fields):
		_fields(fields){
		std::sort(_fields.begin(), _fields.end(), var_type_comparator());
	}

	virtual bool is_class() const override{
		return true;
	}

	bool has_field(const char* name) const{
		return std::binary_search(_fields.begin(), _fields.end(), name, var_type_comparator());
	}

	const var_type& get_field(const char* name) const{
		return *std::lower_bound(_fields.begin(), _fields.end(), name, var_type_comparator());
	}
};


}//namespace donkey


#endif /* __types_hpp__ */

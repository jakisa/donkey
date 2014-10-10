#ifndef __variables_hpp__
#define __variables_hpp__


#include "config.hpp"
#include "errors.hpp"
#include "helpers.hpp"
#include "function.hpp"
#include "donkey_object.hpp"

#include <cstring>
#include <cstdint>
#include <vector>


namespace donkey{

struct code_address{
	uint64_t value;
	
	bool operator==(const code_address& oth) const{
		return value == oth.value;
	}
};

/*
enum struct data_type: char{
	nothing      = 0x00,
	number       = 0x01,
	code_address = 0x02,
	string       = 0x03,
	function     = 0x04,
	object       = 0x05,
};

enum struct mem_type: char{
	nothing        = 0x00,
	shared_pointer = 0x10,
	value          = 0x20,
	weak_pointer   = 0x30,
};

enum{
	mem_type_smartptr_mask = 0x10
};*/

enum struct var_type: char{
	nothing        = 0x00,
	number         = 0x01,
	code_address   = 0x02,
	
	string         = 0x13,
	function       = 0x14,
	object         = 0x15,
	
	weak_string    = 0x23,
	weak_function  = 0x24,
	weak_object    = 0x25,
};

enum{
	sharedptr_mask = 0x10,
	weakptr_mask   = 0x20,
	smartptr_mask  = (sharedptr_mask | weakptr_mask),
};

inline bool is_shared(var_type vt){
	return sharedptr_mask & char(vt);
}

inline bool is_weak(var_type vt){
	return weakptr_mask & char(vt);
}

inline bool is_smart(var_type vt){
	return smartptr_mask & char(vt);
}

inline bool is_string(var_type vt){
	return char(var_type::string) & char(vt);
}

inline bool is_function(var_type vt){
	return char(var_type::function) & char(vt);
}

inline bool is_object(var_type vt){
	return char(var_type::object) & char(vt);
}

inline var_type shared_version(var_type vt){
	return var_type((char(vt) & (~sharedptr_mask)) | weakptr_mask);
}

inline var_type weak_version(var_type vt){
	return var_type((char(vt) & (~weakptr_mask)) | sharedptr_mask);
}


class variable;

class vtable;

class heap_header{
	heap_header(const heap_header&) = delete;
	void operator=(const heap_header&) = delete;
	friend class variable;
private:
	size_t _s_count;
	size_t _u_count;
	void* _p;
	heap_header(void* p):
		_s_count(1),
		_u_count(1),
		_p(p){
	}
public:
	void add_shared(){
		++_s_count;
		++_u_count;
	}
	
	template<class T>
	void remove_shared(){
		--_s_count;
		if(!_s_count){
			delete static_cast<T*>(_p);
			_p = nullptr;
		}
		--_u_count;
		if(!_u_count){
			delete this;
		}
	}
	
	template<class T>
	void remove_shared_array(){
		--_s_count;
		if(!_s_count){
			delete[] static_cast<T*>(_p);
			_p = nullptr;
		}
		--_u_count;
		if(!_u_count){
			delete this;
		}
	}
	
	void add_weak(){
		++_u_count;
	}
	void remove_weak(){
		--_u_count;
	}
	
	bool expired(){
		return _s_count == 0;
	}
	
	template<class T>
	T* as_t(){
		if(_p == nullptr){
			runtime_error("null reference exception");
		}
		return static_cast<T*>(_p);
	}
};

inline constexpr size_t max(size_t x, size_t y){
	return x > y ? x : y;
}


inline constexpr size_t stack_var_union_size(){
	return max(
		max(sizeof(number), sizeof(code_address)),
		sizeof(heap_header*)
	);
}

variable call_function_by_address(code_address addr, runtime_context& ctx, size_t params_size); //runtime_context.cpp

class variable final{
private:
	union{
		number _n;
		code_address _f;
		heap_header* _h_ptr;
		std::array<char, stack_var_union_size()> _;
	};
	
	var_type _vt;
	
	void _runtime_error(std::string msg) const;
	
	void _inc_counts_impl() const;
	
	void _inc_counts() const{
		if(is_smart(_vt)){
			_inc_counts_impl();
		}
	}
	
	void _dec_counts_impl() const;
	
	void _dec_counts() const{
		if(is_smart(_vt)){
			_dec_counts_impl();
		}
	}
public:
	variable():
		_vt(var_type::nothing){
	}
	
	void reset(){
		_dec_counts();
		_vt = var_type::nothing;
	}
	
	explicit variable(number n):
		_n(n),
		_vt(var_type::number){
	}
	
	explicit variable(code_address f):
		_f(f),
		_vt(var_type::code_address){
	}
	
	explicit variable(const std::string& s){
		char* p = new char[s.size() + 1];
		if(!p){
			runtime_error("out of memory");
		}
		_h_ptr = new heap_header(p);
		if(!_h_ptr){
			delete[] p;
			runtime_error("out of memory");
		}
		memcpy(p, s.c_str(), s.size()+1);
		_vt = var_type::string;
	}
	
	explicit variable(const char* s){
		if(!s){
			s = "";
		}
		size_t sz = strlen(s);
		char* p = new char[sz + 1];
		if(!p){
			runtime_error("out of memory");
		}
		_h_ptr = new heap_header(p);
		if(!_h_ptr){
			runtime_error("out of memory");
		}
		memcpy(p, s, sz + 1);
		_vt = var_type::string;
	}
	
	explicit variable(function&& f){
		function* p = new function(std::move(f));
		if(!p){
			runtime_error("out of memory");
		}
		_h_ptr = new heap_header(p);
		if(!_h_ptr){
			delete p;
			runtime_error("out of memory");
		}
		_vt = var_type::function;
	}
	
	explicit variable(vtable* vt){
		donkey_object* p = new donkey_object(vt);
		if(!p){
			runtime_error("out of memory");
		}
		_h_ptr = new heap_header(p);
		if(!_h_ptr){
			delete p;
			runtime_error("out of memory");
		}
		_vt = var_type::object;
	}
		
	
	variable(const variable& orig):
		_(orig._),
		_vt(orig._vt){
		_inc_counts();
	}
	
	variable& operator=(const variable& orig){
		if(&orig == this){
			return *this;
		}
		orig._inc_counts();
		_dec_counts();
		
		_ = orig._;
		_vt = orig._vt;
		
		return *this;
	}
	
	variable(variable&& orig) noexcept:
		_(orig._),
		_vt(orig._vt){
		orig._vt = var_type::nothing;
	}
	
	variable& operator=(variable&& orig) noexcept{
		if(&orig == this){
			return *this;
		}
		
		char tmp[sizeof(variable)];
		memcpy(tmp, this, sizeof(variable));
		memcpy(this, &orig, sizeof(variable));
		memcpy(&orig, tmp, sizeof(variable));
		
		return *this;
	}
	
	variable non_shared() const{
		if(is_shared(_vt)){
			variable ret;
			ret._vt = weak_version(_vt);
			ret._h_ptr = _h_ptr;
			ret._inc_counts();
			return ret;
		}
		return *this;
	}
	
	variable non_weak() const{
		if(is_weak(_vt)){
			if(_h_ptr->_s_count == 0){
				return variable();
			}
			variable ret;
			ret._vt = shared_version(_vt);
			ret._h_ptr = _h_ptr;
			ret._inc_counts();
			return ret;
		}
		return *this;
	}
	
	~variable(){
		_dec_counts();
	}
	
	var_type get_data_type() const{
		if(is_weak(_vt)){
			return _h_ptr->_p == nullptr ? var_type::nothing : shared_version(_vt);
		}
		return _vt;
	}
	
	bool is_callable() const{
		var_type vt = get_data_type();
		return vt == var_type::code_address || vt == var_type::function;
	}
	
	std::string get_type_name() const{
		switch(get_data_type()){
			case var_type::number:
				return "number";
			case var_type::code_address:
			case var_type::function:
				return "function";
			case var_type::string:
				return "string";
			case var_type::nothing:
				return "null";
			case var_type::object:
				return _h_ptr->as_t<donkey_object>()->get_type_name();
			default:
				return "";
		}
		return "";
	}
	
	number as_number_unsafe() const{
		return _n;
	}
	
	number as_number() const{
		if(_vt != var_type::number){
			_runtime_error("number expected");
		}
		return as_number_unsafe();
	}
	
	integer as_integer() const{
		return (integer)as_number();
	}
	
	number& as_lnumber_unsafe(){
		return _n;
	}
	
	number& as_lnumber(){
		if(_vt != var_type::number){
			_runtime_error("number expected");
		}
		return as_lnumber_unsafe();
	}
	
	code_address as_code_address_unsafe() const{
		return _f;
	}
	
	variable call(runtime_context& ctx, size_t params_size) const{
		switch(get_data_type()){
			case var_type::code_address:
				return call_function_by_address(as_code_address_unsafe(), ctx, params_size);
			case var_type::function:
				return (*_h_ptr->as_t<function>())(ctx, params_size);
			default:
				_runtime_error("function expected");
		}
		return variable();
	}
	
	const char* as_string_unsafe() const{
		return _h_ptr->as_t<char>();
	}
	
	const char* as_string() const{
		if(get_data_type() != var_type::string){
			_runtime_error("string expected");
		}
		return as_string_unsafe();
	}
	
	std::string to_string() const{
		switch(get_data_type()){
			case var_type::string:
				return std::string(as_string_unsafe());
			case var_type::number:
				return donkey::to_string(as_number());
			case var_type::code_address:
			case var_type::function:
				return std::string("function");
			case var_type::object:
				return _h_ptr->as_t<donkey_object>()->to_string();
			default:
				return std::string("null");
		}
	}
	
	heap_header* as_reference_unsafe() const{
		return _h_ptr;
	}
	
	bool operator==(const variable& oth) const{
		var_type dt = get_data_type();
		
		if(dt != oth.get_data_type()){
			return false;
		}
		
		switch(dt){
			case var_type::number:
				return as_number_unsafe() == oth.as_number_unsafe();
			case var_type::code_address:
				return as_code_address_unsafe() == oth.as_code_address_unsafe();
			case var_type::string:
				return strcmp(as_string_unsafe(), oth.as_string_unsafe()) == 0;
			case var_type::nothing:
				return true;
			default:
				return as_reference_unsafe() == oth.as_reference_unsafe();
		}
	}
	
	bool operator!=(const variable& oth) const{
		return !(*this == oth);
	}
	
	bool operator<(const variable& oth) const{
		var_type dt = get_data_type();
		if(dt != oth.get_data_type()){
			runtime_error("cannot compare " + get_type_name() + " and " + oth.get_type_name());
		}
		switch(dt){
			case var_type::number:
				return as_number_unsafe() < oth.as_number_unsafe();
			case var_type::string:
				return strcmp(as_string_unsafe(), oth.as_string_unsafe()) < 0;
			default:
				runtime_error("cannot compare " + get_type_name() + " and " + oth.get_type_name());
				return false;
		}
	}
	
	bool operator>(const variable& oth) const{
		return oth < *this;
	}
	
	bool operator<=(const variable& oth) const{
		return !(oth < *this);
	}
	
	bool operator>=(const variable& oth) const{
		return !(*this < oth);
	}
	
	variable& nth_field(size_t n) const{
		if(get_data_type() == var_type::object){
			return _h_ptr->as_t<donkey_object>()->get_field(n);
		}
	
		_runtime_error("variable doesn't have " + std::to_string(n) + " fields");
		return *static_cast<variable*>(nullptr);
	}
	
	vtable* get_vtable() const{
		if(get_data_type() == var_type::object){
			return _h_ptr->as_t<donkey_object>()->get_vtable();
		}
	
		return nullptr;
	}
	
};

}//donkey

#endif /*__variables_hpp__*/

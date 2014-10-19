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

class code_address{
private:
	uint32_t _module_index;
	uint32_t _function_index;
public:
	code_address(uint32_t module_index, uint32_t function_index):
		_module_index(module_index),
		_function_index(function_index){
	}
	bool operator==(const code_address& oth) const{
		return _module_index == oth._module_index && _function_index == oth._function_index;
	}

	size_t get_module_index() const{
		return _module_index;
	}

	size_t get_function_index() const{
		return _function_index;
	}
};


enum struct var_type: char{
	nothing        = 0x00,
	number         = 0x01,
	code_address   = 0x02,
	
	string         = 0x13,
	function       = 0x14,
	object         = 0x15,
	native         = 0x16,
	
	weak_string    = 0x23,
	weak_function  = 0x24,
	weak_object    = 0x25,
	weak_native    = 0x26,
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

inline bool is_native(var_type vt){
	return char(var_type::native) & char(vt);
}

inline var_type shared_version(var_type vt){
	return var_type((char(vt) & (~sharedptr_mask)) | weakptr_mask);
}

inline var_type weak_version(var_type vt){
	return var_type((char(vt) & (~weakptr_mask)) | sharedptr_mask);
}


class variable;

class vtable;


typedef void(*deleter_type)(void*);

template<typename T>
void deleter(void* p){
	delete static_cast<T*>(p);
}

template<typename T>
void array_deleter(void* p){
	delete[] static_cast<T*>(p);
}



class heap_header{
	heap_header(const heap_header&) = delete;
	void operator=(const heap_header&) = delete;
	friend class variable;
private:
	deleter_type _deleter;
	size_t _s_count;
	size_t _u_count;
	void* _p;
	vtable* _vt;
	heap_header(vtable* vt, void* p, deleter_type del):
		_deleter(del),
		_s_count(1),
		_u_count(1),
		_p(p),
		_vt(vt){
	}
public:
	void add_shared(){
		++_s_count;
		++_u_count;
	}
	
	void remove_shared(){
		--_s_count;
		if(!_s_count){
			_deleter(_p);
			_p = nullptr;
		}
		--_u_count;
		if(!_u_count){
			delete this;
		}
	}
	
	void remove_shared_object(const variable& v){
		--_s_count;
		if(!_s_count){
			static_cast<donkey_object*>(_p)->dispose(v);
			_deleter(_p);
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
	
	vtable* get_vtable(){
		return _vt;
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

typedef std::shared_ptr<vtable> vtable_ptr;

vtable_ptr string_vtable(); //core_vtables.cpp
vtable_ptr function_vtable(); //core_vtables.cpp

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
		_h_ptr = new heap_header(string_vtable().get(), p, &array_deleter<char>);
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
		_h_ptr = new heap_header(string_vtable().get(), p, &array_deleter<char>);
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
		_h_ptr = new heap_header(function_vtable().get(), p, &deleter<function>);
		if(!_h_ptr){
			delete p;
			runtime_error("out of memory");
		}
		_vt = var_type::function;
	}
	
	variable(vtable* vt, runtime_context& ctx){
		donkey_object* p = new donkey_object(vt, &ctx);
		if(!p){
			runtime_error("out of memory");
		}
		_h_ptr = new heap_header(vt, p, &deleter<donkey_object>);
		if(!_h_ptr){
			delete p;
			runtime_error("out of memory");
		}
		_vt = var_type::object;
	}
	
	template<typename T>
	explicit variable(T* p){
		_h_ptr = new heap_header(p->get_vtable(), p, &deleter<T>);
		if(!_h_ptr){
			delete p;
			runtime_error("out of memory");
		}
		_vt = var_type::native;
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
	
	variable call_functor(runtime_context& ctx, size_t params_size) const;
	
	variable call(runtime_context& ctx, size_t params_size) const{
		switch(get_data_type()){
			case var_type::code_address:
				return call_function_by_address(as_code_address_unsafe(), ctx, params_size);
			case var_type::function:
				return (*_h_ptr->as_t<function>())(ctx, params_size);
			default:
				return call_functor(ctx, params_size);
		}
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
	
	std::string to_string() const;
	
	heap_header* as_reference_unsafe() const{
		return _h_ptr;
	}
	
	donkey_object* as_donkey_object_unsafe() const{
		return as_reference_unsafe()->as_t<donkey_object>();
	}
	
	std::string get_full_type_name() const;
	
	template<typename T>
	T* as_t_unsafe() const{
		return as_reference_unsafe()->as_t<T>();
	}
	
	template<typename T>
	T* as_t(const std::string& full_type_name) const{
		if(get_full_type_name() != full_type_name){
			_runtime_error(full_type_name + " expected");
		}
		return as_t_unsafe<T>();
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
			runtime_error("cannot compare " + get_full_type_name() + " and " + oth.get_full_type_name());
		}
		switch(dt){
			case var_type::number:
				return as_number_unsafe() < oth.as_number_unsafe();
			case var_type::string:
				return strcmp(as_string_unsafe(), oth.as_string_unsafe()) < 0;
			default:
				runtime_error("cannot compare " + get_full_type_name() + " and " + oth.get_full_type_name());
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
	
	vtable* get_vtable() const;
	
};

}//donkey

#endif /*__variables_hpp__*/

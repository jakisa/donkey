#ifndef __variables_hpp__
#define __variables_hpp__

#include "errors.hpp"
#include "helpers.hpp"

#include <cstring>


namespace donkey{

typedef u_int64_t code_address;

enum struct data_type: char{
	nothing  = 0x00,
	number   = 0x01,
	function = 0x02,
	string   = 0x03,
};

enum struct mem_type: char{
	nothing        = 0x00,
	shared_pointer = 0x10,
	value          = 0x20,
	weak_pointer   = 0x30,
	stack_pointer  = 0x40,
};

enum{
	mem_type_smartptr_mask = 0x10
};

class stack_var;

class heap_header{
	heap_header(const heap_header&) = delete;
	void operator=(const heap_header&) = delete;
	friend class stack_var;
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
	
	template<class T>
	T* as_t(){
		return static_cast<T*>(_p);
	}
};

inline constexpr size_t max(size_t x, size_t y){
	return x > y ? x : y;
}

inline constexpr size_t stack_var_union_size(){
	return max(
		max(sizeof(double), sizeof(code_address)),
		max(sizeof(stack_var*), sizeof(heap_header*))
	);
}


class stack_var{
private:
	union{
		double _n;
		code_address _f;
		stack_var* _s_ptr;
		heap_header* _h_ptr;
		std::array<char, stack_var_union_size()> _;
	};
	
	data_type _dt;
	mem_type _mt;
	
	void _inc_counts() const{
		if(!(char(_mt) & mem_type_smartptr_mask)){
			return;
		}
		switch(_mt){
			case mem_type::shared_pointer:
				_h_ptr->add_shared();
				break;
			case mem_type::weak_pointer:
				_h_ptr->add_weak();
				break;
			default:
				break;
		}
	}
	
	void _dec_counts() const{
		if(!(char(_mt) & mem_type_smartptr_mask)){
			return;
		}
		switch(_mt){
			case mem_type::shared_pointer:
				switch(_dt){
					case data_type::string:
						_h_ptr->remove_shared_array<char>();
						break;
					case data_type::number:
						_h_ptr->remove_shared<double>();
						break;
					case data_type::function:
						_h_ptr->remove_shared<code_address>();
						break;
					default:
						break;
				}
				break;
			case mem_type::weak_pointer:
				_h_ptr->remove_weak();
				break;
			default:
				break;
		}
	}
	
	struct by_ref_wrapper{
		stack_var& var;
		by_ref_wrapper(stack_var& var):
			var(var){
		}
	};
	
	struct by_val_wrapper{
		stack_var& var;
		by_val_wrapper(stack_var& var):
			var(var){
		}
	};
public:
	stack_var():
		_dt(data_type::nothing),
		_mt(mem_type::nothing){
	}
	
	stack_var& reset(){
		_dec_counts();
		_dt = data_type::nothing;
		_mt = mem_type::nothing;
		
		return *this;
	}
	
	explicit stack_var(double n):
		_n(n),
		_dt(data_type::number),
		_mt(mem_type::value){
	}
	
	explicit stack_var(code_address f):
		_f(f),
		_dt(data_type::function),
		_mt(mem_type::value){
	}
	
	explicit stack_var(const std::string& s){
		char* p = new char[s.size() + 1];
		if(!p){
			runtime_error("out of memory");
		}
		_h_ptr = new heap_header(p);
		if(!_h_ptr){
			runtime_error("out of memory");
		}
		memcpy(p, s.c_str(), s.size() + 1);
		_dt = data_type::string;
		_mt = mem_type::shared_pointer;
	}
	
	by_ref_wrapper by_ref(){
		return by_ref_wrapper(*this);
	}
	
	stack_var(by_ref_wrapper w):
		_s_ptr(w.var._mt == mem_type::stack_pointer ? w.var._s_ptr : &w.var),
		_dt(w.var._dt),
		_mt(mem_type::stack_pointer){
	}
	
	by_val_wrapper by_val(){
		return by_val_wrapper(*this);
	}
	
	stack_var(by_val_wrapper w):
		_(w.var._mt == mem_type::stack_pointer ? w.var._s_ptr->_ : w.var._),
		_dt(w.var._dt),
		_mt(w.var._mt == mem_type::stack_pointer ? w.var._s_ptr->_mt : w.var._mt){
		_inc_counts();
	}
		
	
	stack_var(const stack_var& orig):
		_(orig._),
		_dt(orig._dt),
		_mt(orig._mt){
		_inc_counts();
	}
	
	stack_var& operator=(const stack_var& orig){
		if(&orig == this){
			return *this;
		}
		if(orig._mt == mem_type::stack_pointer){
			return *this = *(orig._s_ptr);
		}
		if(_mt == mem_type::stack_pointer){
			return *(_s_ptr) = orig;
		}	
		orig._inc_counts();
		_dec_counts();
		
		_ = orig._;
		_dt = orig._dt;
		_mt = orig._mt;
		
		return *this;
	}
	
	stack_var(stack_var&& orig):
		_(orig._),
		_dt(orig._dt),
		_mt(orig._mt){
		orig._dt = data_type::nothing;
		orig._mt = mem_type::nothing;
	}
	
	stack_var& operator=(stack_var&& orig){
		if(&orig == this){
			return *this;
		}
		if(orig._mt == mem_type::stack_pointer){
			return *this = *(orig._s_ptr);
		}
		if(_mt == mem_type::stack_pointer){
			return *(_s_ptr) = orig;
		}
		orig._inc_counts();
		_dec_counts();
		
		_ = orig._;
		_dt = orig._dt;
		_mt = orig._mt;
		
		orig._dec_counts();
		
		orig._dt = data_type::nothing;
		orig._mt = mem_type::nothing;
		
		return *this;
	}
	
	stack_var non_shared() const{
		if(_mt == mem_type::shared_pointer){
			stack_var ret;
			ret._mt = mem_type::weak_pointer;
			ret._h_ptr = _h_ptr;
			ret._inc_counts();
			return ret;
		}
		return *this;
	}
	
	stack_var non_weak() const{
		if(_mt == mem_type::weak_pointer){
			if(_h_ptr->_s_count == 0){
				return stack_var();
			}
			stack_var ret;
			ret._mt = mem_type::shared_pointer;
			ret._h_ptr = _h_ptr;
			ret._inc_counts();
			return ret;
		}
		return *this;
	}
	
	~stack_var(){
		_dec_counts();
	}
	
	data_type get_data_type() const{
		return (_mt == mem_type::weak_pointer && _h_ptr->_p == nullptr) ? data_type::nothing : _dt;
	}
	
	std::string get_type_name() const{
		switch(get_data_type()){
			case data_type::number:
				return "number";
			case data_type::function:
				return "function";
			case data_type::string:
				return "string";
			default:
				return "null";
		}
	}
	
	double as_stack_number_unsafe() const{
		return _n;
	}
	
	double as_number_unsafe() const{
		switch(_mt){
			case mem_type::shared_pointer:
			case mem_type::weak_pointer:
				return *_h_ptr->as_t<double>();
			case mem_type::stack_pointer:
				return _s_ptr->as_number_unsafe();
			default:
				return _n;
		}
	}
	
	double as_number() const{
		if(get_data_type() != data_type::number){
			runtime_error("number expected");
		}
		return as_number_unsafe();
	}
	
	double& as_lnumber_unsafe(){
		switch(_mt){
			case mem_type::shared_pointer:
			case mem_type::weak_pointer:
				return *_h_ptr->as_t<double>();
			case mem_type::stack_pointer:
				return _s_ptr->as_lnumber_unsafe();
			default:
				return _n;
		}
	}
	
	double& as_lnumber(){
		if(get_data_type() != data_type::number){
			runtime_error("number expected");
		}
		return as_lnumber_unsafe();
	}
	
	code_address as_function_unsafe() const{
		switch(_mt){
			case mem_type::shared_pointer:
			case mem_type::weak_pointer:
				return *_h_ptr->as_t<code_address>();
			case mem_type::stack_pointer:
				return _s_ptr->as_function_unsafe();
			default:
				return _f;
		}
	}
	
	code_address as_function() const{
		if(get_data_type() != data_type::function){
			runtime_error("function expected");
		}
		return as_function_unsafe();
	}
	
	const char* as_string_unsafe() const{
		switch(_mt){
			case mem_type::stack_pointer:
				return _s_ptr->as_string_unsafe();
			default:
				return _h_ptr->as_t<char>();
		}
	}
	
	std::string to_string() const{
		switch(_dt){
			case data_type::string:
				return std::string(as_string_unsafe());
			case data_type::number:
				return donkey::to_string(as_number());
			case data_type::function:
				return std::string("function");
			default:
				return std::string("null");
		}
	}
	
	heap_header* as_reference_unsafe() const{
		switch(_mt){
			case mem_type::stack_pointer:
				return _s_ptr->as_reference_unsafe();
			default:
				return _h_ptr;
		}
	}
	
	bool operator==(const stack_var& oth) const{
		data_type dt = get_data_type();
		
		if(dt != oth.get_data_type()){
			return false;
		}
		
		switch(dt){
			case data_type::number:
				return as_number_unsafe() == oth.as_number_unsafe();
			case data_type::function:
				return as_function_unsafe() == oth.as_function_unsafe();
			case data_type::string:
				return strcmp(as_string_unsafe(), oth.as_string_unsafe()) == 0;
			case data_type::nothing:
				return true;
			default:
				return as_reference_unsafe() == oth.as_reference_unsafe();
		}
	}
	
	bool operator!=(const stack_var& oth) const{
		return !(*this == oth);
	}
	
	bool operator<(const stack_var& oth) const{
		data_type dt = get_data_type();
		if(dt != oth.get_data_type()){
			runtime_error("cannot compare " + get_type_name() + " and " + oth.get_type_name());
		}
		switch(dt){
			case data_type::number:
				return as_number_unsafe() < oth.as_number_unsafe();
			case data_type::string:
				return strcmp(as_string_unsafe(), oth.as_string_unsafe()) < 0;
			default:
				runtime_error("cannot compare " + get_type_name() + " and " + oth.get_type_name());
				return false;
		}
	}
	
	bool operator>(const stack_var& oth) const{
		return oth < *this;
	}
	
	bool operator<=(const stack_var& oth) const{
		return !(oth < *this);
	}
	
	bool operator>=(const stack_var& oth) const{
		return !(*this < oth);
	}
};

}//donkey

#endif /*__variables_hpp__*/
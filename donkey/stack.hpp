#ifndef __stack_hpp__
#define __stack_hpp__

#include "variables.hpp"

namespace donkey{

class stack{
	stack(const stack&) = delete;
	void operator=(const stack&) = delete;
private:
	variable* _v;
	const size_t _cap;
	size_t _sz;
public:
	stack(size_t sz):
		_v(new variable[sz]),
		_cap(sz),
		_sz(0){
		if(!_v){
			runtime_error("too big stack size");
		}
	}
	
	~stack(){
		delete[] _v;
	}
	
	void add_size(size_t sz){
		_sz += sz;
		if(_sz > _cap){
			runtime_error("stack overflow");
		}
	}
	
	void push(variable&& v){
		if(_sz >= _cap){
			runtime_error("stack overflow");
		}
		_v[_sz++] = std::move(v);
	}
	
	void pop(size_t cnt){
		for(size_t i = 0; i < cnt; ++i){
			_v[--_sz].reset();
		}
	}
	
	variable& top(size_t idx){
		return _v[_sz - idx - 1];
	}
	
	variable& operator[](size_t idx){
		return _v[idx];
	}
	
	size_t size() const{
		return _sz;
	}
};


}//donkey

#endif /* __stack_hpp__*/


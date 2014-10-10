#ifndef __stack_hpp__
#define __stack_hpp__

#include "variables.hpp"
#include <vector>

namespace donkey{

class stack{
	stack(const stack&) = delete;
	void operator=(const stack&) = delete;
private:
	std::vector<variable> _v;
public:
	stack(){
	}
	
	void add_size(size_t sz){
		_v.resize(_v.size() + sz);
	}
	
	void push(variable&& v){
		_v.push_back(std::move(v));
	}
	
	void pop(size_t cnt){
		_v.resize(_v.size() - cnt);
	}
	
	variable& top(size_t idx){
		return _v[_v.size() - idx - 1];
	}
	
	variable& operator[](size_t idx){
		return _v[idx];
	}
	
	size_t size() const{
		return _v.size();
	}
};


}//donkey

#endif /* __stack_hpp__*/


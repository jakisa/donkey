#ifndef __stack_alloc_hpp__
#define __stack_alloc_hpp__

#include "variables.hpp"
#include <memory>

namespace donkey{

class stack_free{
	stack_free(const stack_free&) = delete;
	void operator=(const stack_free&) = delete;
private:
	variable* _p;
	size_t _n;
public:
	stack_free(variable* p, size_t n):
		_p(p),
		_n(n){
	}
	~stack_free(){
		for(size_t i = _n; i; --i){
			_p[i-1].reset();
		}
	}
};

#define STACK_ALLOC(name, n)\
	size_t name##_size = (n);\
	variable* name = static_cast<variable*>(name##_size > 0 ? alloca(name##_size*sizeof(variable)) : nullptr);\
	memset(name, 0, name##_size*sizeof(variable));\
	stack_free name##_stack_free(name, name##_size)

}//donkey


#endif // __stack_alloc_hpp__

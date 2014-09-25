#ifndef __context_hpp__
#define __context_hpp__

#include <vector>
#include <memory>
#include <functional>

namespace donkey{

class variable;
typedef std::shared_ptr<variable> variable_ptr;

struct runtime_context{
	std::vector<variable_ptr> global;
	std::vector<variable_ptr> stack;
	variable_ptr retval;

	void push(variable_ptr v){
		stack.push_back(v);
	}

	variable_ptr pop(){
		variable_ptr ret = stack.back();
		stack.pop_back();
		return ret;
	}
	variable_ptr top(){
		return stack.back();
	}
};

typedef std::function<void(runtime_context&)> function;

}//namespace donkey


#endif /*__context_hpp__*/


#ifndef __function_hpp__
#define __function_hpp__

#include <functional>
#include <memory>

namespace donkey{

class runtime_context;
class variable;

typedef std::function<variable(runtime_context&, variable*, size_t)> function;

typedef std::function<variable(const variable&, runtime_context&, variable*, size_t)> method;

typedef std::shared_ptr<method> method_ptr;

}//donkey

#endif /*__function_hpp__*/


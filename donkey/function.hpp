#ifndef __function_hpp__
#define __function_hpp__

#include <functional>

namespace donkey{

struct runtime_context;
class variable;

typedef std::function<variable(runtime_context&, size_t)> function;

typedef std::function<variable(variable&, runtime_context&, size_t)> method;

}//donkey

#endif /*__function_hpp__*/


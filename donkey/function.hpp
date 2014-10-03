#ifndef __function_hpp__
#define __function_hpp__

#include <functional>
#include "variables.hpp"
#include "runtime_context.hpp"

namespace donkey{

typedef std::function<variable(runtime_context&, size_t)> function;

typedef std::function<variable(variable&, runtime_context&, size_t)> method;

}//donkey

#endif /*__function_hpp__*/


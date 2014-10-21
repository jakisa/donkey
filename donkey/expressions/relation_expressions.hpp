#ifndef __relation_expressions_hpp__
#define __relation_expressions_hpp__

#include "expressions.hpp"
#include "operators.hpp"

namespace donkey{

NUMBER_BINARY_CPP(less, lt, <)
NUMBER_BINARY_CPP(greater, gt, >)
NUMBER_BINARY_CPP(less_equal, le, <=)
NUMBER_BINARY_CPP(greater_equal, ge, >=)
NUMBER_BINARY_CPP(equal, eq, ==)
NUMBER_BINARY_CPP(unequal, ne, !=)



}//donkey

#endif /*__relation_expressions_hpp__*/

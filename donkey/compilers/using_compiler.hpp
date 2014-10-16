#ifndef __using_compiler_hpp__
#define __using_compiler_hpp__

#include "scope.hpp"
#include "tokenizer.hpp"

namespace donkey{

void compile_using(scope& target, tokenizer& parser);

}//donkey;


#endif /* using_compiler_hpp*/

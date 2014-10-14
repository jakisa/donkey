#ifndef __class_compiler_hpp__
#define __class_compiler_hpp__

#include "scope.hpp"
#include "tokenizer.hpp"

namespace donkey{

void compile_class(scope& target, tokenizer& parser);

}//donkey

#endif

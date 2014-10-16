#ifndef __scope_compiler_hpp__
#define __scope_compiler_hpp__

#include "scope.hpp"
#include "tokenizer.hpp"

namespace donkey{

void compile_local_scope(scope& target, tokenizer& parser);

}//donkey

#endif /*__scope_compiler_hpp__*/

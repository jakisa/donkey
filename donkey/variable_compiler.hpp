#ifndef __variable_compiler_hpp__
#define __variable_compiler_hpp__

#include "scope.hpp"
#include "tokenizer.hpp"

namespace donkey{

expression_ptr compile_variable(scope& target, tokenizer& parser, bool is_public = false);

}//donkey

#endif /* __variable_compiler_hpp__*/

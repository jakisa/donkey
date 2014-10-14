#ifndef __function_compiler_hpp__
#define __function_compiler_hpp__

#include "scope.hpp"
#include "tokenizer.hpp"

namespace donkey{

void compile_function(scope& target, tokenizer& parser);

void compile_constructor(class_scope& target, tokenizer& parser, const std::vector<std::string>& bases);

void compile_destructor(class_scope& target, tokenizer& parser, const std::vector<std::string>& bases);


}//donkey

#endif /*__function_compiler_hpp__*/

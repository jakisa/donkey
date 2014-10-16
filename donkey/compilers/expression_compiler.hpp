#ifndef __expression_compiler_hpp__
#define __expression_compiler_hpp__

#include "scope.hpp"
#include "tokenizer.hpp"

namespace donkey{

void compile_expression_statement(scope& target, tokenizer& parser);

}//donkey

#endif

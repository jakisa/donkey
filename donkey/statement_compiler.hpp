#ifndef __statement_compiler_hpp__
#define __statement_compiler_hpp__

#include "scope.hpp"
#include "tokenizer.hpp"

namespace donkey{

void compile_statement(scope& target, tokenizer& parser);

}//donkey;


#endif /* statement_compiler_hpp*/

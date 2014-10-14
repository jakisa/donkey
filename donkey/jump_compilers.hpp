#ifndef __jump_compilers_hpp__
#define __jump_compilers_hpp__

#include "scope.hpp"
#include "tokenizer.hpp"

namespace donkey{

void compile_return(scope& target, tokenizer& parser);
void compile_break(scope& target, tokenizer& parser);
void compile_continue(scope& target, tokenizer& parser);

}//donkey

#endif

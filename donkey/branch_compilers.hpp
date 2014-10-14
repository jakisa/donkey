#ifndef __branch_compilers_hpp__
#define __branch_compilers_hpp__

#include "scope.hpp"
#include "tokenizer.hpp"

namespace donkey{

void compile_if(scope& target, tokenizer& parser);
void compile_switch(scope& target, tokenizer& parser);

}//donkey

#endif

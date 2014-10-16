#ifndef __loop_compilers_hpp__
#define __loop_compilers_hpp__

#include "tokenizer.hpp"
#include "scope.hpp"

namespace donkey{

void compile_for(scope& target, tokenizer& parser);
void compile_while(scope& target, tokenizer& parser);
void compile_do(scope& target, tokenizer& parser);


}//donkey;

#endif /*__loop_compilers_hpp__*/


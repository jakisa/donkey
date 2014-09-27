#ifndef __expression_builder_hpp__
#define __expression_builder_hpp__

#include "expressions.hpp"
#include "identifiers.hpp"
#include "tokenizer.hpp"

namespace donkey{

expression_ptr build_expression(const identifier_lookup& lookup, tokenizer& parser, bool can_be_empty);

}//namespace donkey

#endif /* __expression_builder_hpp__ */

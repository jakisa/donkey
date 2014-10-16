#include "using_compiler.hpp"
#include "compiler.hpp"

namespace donkey {

static void add_using(scope& target, const identifier_lookup& lookup, tokenizer& parser){
	if(parser.get_token_type() != tokenizer::tt_word){
		unexpected_error(*parser);
	}
	
	identifier_ptr id = lookup.get_identifier(*parser);
	
	if(!id){
		semantic_error("unknown identifier " + *parser);
	}
	
	++parser;
	
	if(*parser == "::"){
		if(id->get_type() != identifier_type::module){
			semantic_error(id->get_name() + " is not module");
		}
		++parser;
		add_using(target, static_cast<module_identifier&>(*id).get_lookup(), parser);
		return;
	}
	target.add_using(id);
}

void compile_using(scope& target, tokenizer& parser){
	++parser;
	
	
	add_using(target, target, parser);
	
	parse(";", parser);
}


}//donkey
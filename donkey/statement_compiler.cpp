#include "statement_compiler.hpp"
#include "compiler.hpp"
#include "loop_compilers.hpp"
#include "branch_compilers.hpp"
#include "jump_compilers.hpp"
#include "scope_compiler.hpp"
#include "class_compiler.hpp"
#include "expression_compiler.hpp"
#include "function_compiler.hpp"
#include "variable_compiler.hpp"
#include "using_compiler.hpp"
#include "errors.hpp"

namespace donkey{

void compile_statement(scope& target, tokenizer& parser){
	if(*parser == "function"){
		compile_function(target, parser);
	}else if(*parser == "var"){
		compile_variable(target, parser);
	}else if(*parser == "while"){
		compile_while(target, parser);
	}else if(*parser == "do"){
		compile_do(target, parser);
	}else if(*parser == "if"){
		compile_if(target, parser);
	}else if(*parser == "switch"){
		compile_switch(target, parser);
	}else if(*parser == "return"){
		compile_return(target, parser);
	}else if(*parser == "{"){
		compile_local_scope(target, parser);
	}else if(*parser == "break"){
		compile_break(target, parser);
	}else if(*parser == "continue"){	
		compile_continue(target, parser);
	}else if(*parser == "class"){
		compile_class(target, parser);
	}else if(*parser == "public"){
		if(!target.is_global()){
			unexpected_error(*parser);
		}
		++parser;
		if(*parser == "var"){
			compile_variable(target, parser, true);
		}else if(*parser == "function"){
			compile_function(target, parser, true);
		}else if(*parser == "class"){
			compile_class(target, parser, true);
		}else{
			unexpected_error(*parser);
		}
	}else if(*parser == "using"){
		compile_using(target, parser);
	}else{
		if(is_keyword(*parser) && *parser != "this" && *parser != "self" && *parser != "new" && *parser != "self"){
			unexpected_error(*parser);
		}
		compile_expression_statement(target, parser);
	}
}

}//donkey
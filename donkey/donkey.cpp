#include "donkey.hpp"
#include "tokenizer.hpp"
#include "scope.hpp"
#include "statement_compiler.hpp"
#include "module.hpp"
#include <unordered_map>
#include "module_bundle.hpp"

namespace donkey{

class compiler::priv{
private:
	std::string _root;
	
	module_bundle _modules;
	
	void compile_module(tokenizer& parser, std::string module_name){
		size_t idx = _modules.reserve_module(module_name);
		
		global_scope target(_modules, std::move(module_name), idx);
		for(; parser;){
			compile_statement(target, parser);
		}
		
		std::string not_defined = target.get_undefined_function();
		if(not_defined != ""){
			semantic_error(parser.get_line_number(), not_defined + " is not defined");
		}
		
		_modules.add_module(module_name, module_ptr(new module(
			target.get_block(),
			module_name,
			idx,
			target.get_number_of_variables(),
			target.get_functions(),
			target.get_vtables(),
			target.get_public_functions(),
			target.get_public_vars()
		)));
	}
public:
	priv(const char* root, size_t stack_size):
		_root(root),
		_modules(stack_size){
	}

	bool compile_module(const char* module_name){
		FILE* fp = fopen((_root + module_name + ".dky").c_str(), "rb");

		if(!fp){
			return false;
		}

		fseek(fp, 0, SEEK_END);

		size_t len = ftell(fp);

		fseek(fp, 0, SEEK_SET);

		std::vector<char> v(len);

		if(fread(&v[0], 1, len, fp) != len){
			return false;
		}

		fclose(fp);

		//try{
			tokenizer parser (&v[0], &v[0] + len);
			compile_module(parser, module_name);
			return true;
		//}catch(const exception&){
		//	return false;
		//}
	}
	
};

compiler::compiler(const char* root, size_t stack_size):
	_private(new priv(root, stack_size)){
}

bool compiler::compile_module(const char* module_name){
	return _private->compile_module(module_name);
}


compiler::~compiler(){
	delete _private;
}

}//namespace donkey

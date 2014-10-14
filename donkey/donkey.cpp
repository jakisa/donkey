#include "donkey.hpp"
#include "tokenizer.hpp"
#include "scope.hpp"
#include "statement_compiler.hpp"
#include "module.hpp"
#include <unordered_map>

namespace donkey{

class compiler::priv{
private:
	std::string _root;
	
	std::unordered_map<std::string, module_ptr> _modules;
	
	
	module_ptr compile_module(tokenizer& parser){
		global_scope target;
		for(; parser;){
			compile_statement(target, parser);
		}
		
		std::string not_defined = target.get_undefined_function();
		if(not_defined != ""){
			semantic_error(parser.get_line_number(), not_defined + " is not defined");
		}
		
		return module_ptr(new module(target.get_block(), target.get_number_of_variables(), target.get_functions(), target.get_vtables()));
	}
public:
	priv(const char* root):
		_root(root){
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
			module_ptr m = compile_module(parser);
			_modules[module_name] = m;
			return true;
		//}catch(const exception&){
		//	return false;
		//}
	}
	
	bool execute_module(const char* module_name, size_t stack_size){
//		try{
			auto it = _modules.find(module_name);
			
			if(it == _modules.end()){
				return false;
			}
			
			runtime_context ctx(it->second.get(), it->second->get_globals_count(), stack_size);
			
			it->second->load(ctx);
			
			variable that = ctx.global(0);
			
			printf("%s\n", get_vtable(ctx, that)->call_member(that, ctx, 0, "toString").to_string().c_str());
			
			return true;
//		}catch(const exception&){
//			return false;
//		}
	}
};

compiler::compiler(const char* root):
	_private(new priv(root)){
}

bool compiler::compile_module(const char* module_name){
	return _private->compile_module(module_name);
}

bool compiler::execute_module(const char* module_name, size_t stack_size){
	return _private->execute_module(module_name, stack_size);
}

compiler::~compiler(){
	delete _private;
}

}//namespace donkey

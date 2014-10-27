#include "donkey.hpp"
#include "tokenizer.hpp"
#include "scope.hpp"
#include "compilers/statement_compiler.hpp"
#include "module.hpp"
#include "compiler.hpp"
#include <unordered_map>
#include "module_bundle.hpp"

namespace donkey{

static bool get_file(const std::string fname, std::vector<char>& v){
#ifdef _MSC_VER
	//FILE* fp = fopen(fname.c_str(), "rb");
	FILE* fp = nullptr;
	fopen_s(&fp, fname.c_str(), "rb");
#else
	FILE* fp = fopen(fname.c_str(), "rb");
#endif

	if(!fp){
		return false;
	}

	fseek(fp, 0, SEEK_END);

	size_t len = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	v.clear();
	v.resize(len);

	if(fread(&v[0], 1, len, fp) != len){
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}


class compiler::priv{
private:
	std::string _root;
	
	module_bundle _modules;
	
	std::unordered_map<std::string, module_loader> _loaders;
	
	void load_donkey_module(tokenizer& parser, std::string module_name){
		size_t idx = _modules.reserve_module(module_name);
		
		
		global_scope target(_modules, std::move(module_name), idx);
		
		while(*parser == "import" || *parser == "using"){
			bool use = (*parser == "using");
			++parser;
			std::string import_name = *parser;
			
			++parser;
			
			module_ptr m = _modules.get_module(import_name);
			
			if(!m){	
				if(_modules.module_in_progress(import_name)){
					semantic_error("circular import detected for " + import_name);
				}
				if(!load_module(import_name.c_str())){
					semantic_error("unknown module " + import_name);
				}
				m = _modules.get_module(import_name);
			}
			
			target.import(import_name, *m);
			
			if(use){
				target.add_using(target.get_identifier(import_name));
			}
			
			parse(";", parser);
		}
		
		for(; parser;){
			compile_statement(target, parser);
		}
		
		std::string not_defined = target.get_undefined_function();
		if(not_defined != ""){
			semantic_error(not_defined + " is not defined");
		}
		
		_modules.add_module(module_name, module_ptr(new module(
			target.get_block(),
			module_name,
			idx,
			target.get_number_of_variables(),
			target.get_functions(),
			target.get_vtables(),
			target.get_public_functions(),
			target.get_public_vars(),
			target.get_public_constants()
		)));
	}
	
	void load_native_module(const std::string& name, const module_loader& loader){
		size_t idx = _modules.reserve_module(name);
		
		module_ptr module = loader(idx);
		
		_modules.add_module(name, module);
	}
	
public:
	priv(const char* root, size_t stack_size):
		_root(root),
		_modules(stack_size){

		if(!_root.empty() && _root.back() != '/'){
			_root += '/';
		}
	}
	
	void add_module_loader(const char* module_name, const module_loader& loader){
		_loaders.emplace(module_name, loader);
	}
	

	bool load_module(const char* module_name){
		auto it = _loaders.find(module_name);
		
		if(it != _loaders.end()){
			load_native_module(it->first, it->second);
			return true;
		}
	
		
		std::vector<char> v;
		if(!get_file(_root + module_name + ".dky", v)){
			return false;
		}
		
		std::unique_ptr<tokenizer> parser;
		
		size_t idx = _modules.get_modules_count();
		
		try{
			try{
				parser.reset(new tokenizer(module_name, &v[0], &v[0] + v.size()));
				load_donkey_module(*parser, module_name);
				return true;
			}catch(const exception_raw& e){
				e.throw_formatted(parser->get_file_name(), parser->get_line_number() + 1);
			}
		}catch(const exception& e){
			_modules.unload_from(idx);
			fprintf(stderr, "%s\n", e.what());
		}
		return false;
	}
	
};

compiler::compiler(const char* root, size_t stack_size):
	_private(new priv(root, stack_size)){
}

void compiler::add_module_loader(const char* module_name, const module_loader& loader){
	_private->add_module_loader(module_name, loader);
}

bool compiler::load_module(const char* module_name){
	return _private->load_module(module_name);
}


compiler::~compiler(){
	delete _private;
}

}//namespace donkey

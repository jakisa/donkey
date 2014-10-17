#include "io_module.hpp"
#include "module.hpp"
#include "cpp/native_module.hpp"
#include <stdio.h>

namespace donkey{

class stream{
	stream(const stream&) = delete;
	void operator=(const stream&) = delete;
private:
	FILE* _fp;
protected:
	stream(FILE* fp):
		_fp(fp){
	}
	void write(const std::string& str){
		fputs(str.c_str(), _fp ? _fp : stdout);
	}
	
	void writeln(const std::string& str){
		fputs(str.c_str(), _fp ? _fp : stdout);
		fputs("\n", _fp ? _fp : stdout);
	}
};

class console: public stream{
public:
	console():
		stream(0){
	}
	
	static std::string handle_name(){
		return "io:ConsoleStream";
	}
	
	void write(const std::string& str){
		stream::write(str);
	}
	
	void writeln(const std::string& str){
		stream::writeln(str);
	}
	
	static vtable_ptr vt(){
		static vtable_ptr ret;
		if(!ret){
			std::unordered_map<std::string, method_ptr> methods;
			
			methods.emplace("write", create_native_method(&console::write));
			methods.emplace("writeln", create_native_method(&console::writeln));
			
			std::unordered_map<std::string, size_t> fields;
		
			ret.reset(new vtable(
				"io",
				"ConsoleStream",
				method_ptr(new method(native_constructor<std::tuple<>, console>())),
				method_ptr(new method(native_destructor<console>())),
				std::move(methods),
				std::move(fields),
				fields.size(),
				false,
				true
			));
			ret->derive_from(*object_vtable());
		}
		return ret;
	}
	
	vtable* get_vtable(){
		return vt().get();
	}
};

static statement_retval init_io(runtime_context& ctx, size_t module_idx, size_t console_idx){
	variable& v_console = global_variable(ctx, module_idx, console_idx);
	
	v_console = variable(console::vt().get(), ctx);
	
	v_console.as_donkey_object_unsafe()->set_handle(console::handle_name(), new console());
	
	return statement_retval::nxt;
}

module_ptr load_io_module(size_t module_idx){
	using namespace std::placeholders;
	
	native_module m("io", module_idx);
	
	m.add_vtable(console::vt());
	
	size_t console_idx = m.add_global("console");
	
	m.set_init(std::bind(&init_io, _1, module_idx, console_idx));
	
	return m.create_module();
}



}//donkey
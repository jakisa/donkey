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
	
	static variable write(const variable& that, const std::string& str){
		that.as_t_unsafe<console>()->stream::write(str);
		return that;
	}
	
	
	static variable writeln(const variable& that, const std::string& str){
		that.as_t_unsafe<console>()->stream::writeln(str);
		return that;
	}
	
	static vtable_ptr vt(){
		static vtable_ptr ret([](){
			std::unordered_map<std::string, method_ptr> methods;
			
			methods.emplace("write", create_native_method("io::console.write", &console::write));
			methods.emplace("writeln", create_native_method("io::console.writeln", &console::writeln));
			
			vtable* vt = new vtable(
				"io",
				"ConsoleStream",
				function(),
				std::move(methods),
				false
			);
			vt->derive_from(*object_vtable());
			return vt;
		}());
		
		return ret;
	}
	
	vtable* get_vtable(){
		return vt().get();
	}
};

static statement_retval init_io(runtime_context& ctx, size_t module_idx, size_t console_idx){
	variable& v_console = global_variable(ctx, module_idx, console_idx);
	
	v_console = variable(new console());
	
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
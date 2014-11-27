#include "gui_module.hpp"
#include "module.hpp"
#include "cpp/native_module.hpp"

namespace donkey{

void add_window_vtables(native_module& m);

module_ptr load_gui_module(size_t module_idx){
	using namespace std::placeholders;
	
	native_module m("gui", module_idx);
	
	add_window_vtables(m);
	
	return m.create_module();
}


}//donkey
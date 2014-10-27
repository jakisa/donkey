#include "containers_module.hpp"
#include "cpp/native_module.hpp"

namespace donkey{


void add_containers_vtables(native_module& m); //container.cpp

module_ptr load_containers_module(size_t module_idx){
	native_module m("containers", module_idx);
	
	add_containers_vtables(m);
	
	return m.create_module();
}


}//donkey


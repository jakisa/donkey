#ifndef __gui_module_hpp__
#define __gui_module_hpp__

#include <memory>

namespace donkey{

class module;
typedef std::shared_ptr<module> module_ptr;


module_ptr load_gui_module(size_t module_idx);



}//donkey


#endif /*__gui_module_hpp__*/

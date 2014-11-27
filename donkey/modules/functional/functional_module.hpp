#ifndef __functional_module_hpp__
#define __functional_module_hpp__

#include <memory>

namespace donkey{

class module;
typedef std::shared_ptr<module> module_ptr;

module_ptr load_functional_module(size_t module_idx);



}//donkey


#endif /*__functional_module_hpp__*/

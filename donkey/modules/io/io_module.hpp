#include <memory>

namespace donkey{

class module;
typedef std::shared_ptr<module> module_ptr;


module_ptr load_io_module(size_t module_idx);



}//donkey
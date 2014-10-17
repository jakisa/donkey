#ifndef __donkey_hpp__
#define __donkey_hpp__

#include <memory>
#include <functional>

namespace donkey{

struct module;

typedef std::shared_ptr<module> module_ptr;

//module_ptr compile(const char* begin, const char* end);


typedef std::function<module_ptr(size_t)> module_loader;

class compiler{
	compiler(const compiler&) = delete;
	void operator=(const compiler&) = delete;
private:
	class priv;
	priv* _private;
public:
	compiler(const char* root, size_t stack_size = 1024);
	
	void add_module_loader(const char* module_name, const module_loader& loader);
	
	bool compile_module(const char* module_name);
	~compiler();
};

}//namespace donkey

#endif /* __donkey_hpp__ */

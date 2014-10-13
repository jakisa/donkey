#ifndef __donkey_hpp__
#define __donkey_hpp__

#include <memory>

namespace donkey{

struct module;

typedef std::shared_ptr<module> module_ptr;

module_ptr compile(const char* begin, const char* end);

class compiler{
	compiler(const compiler&) = delete;
	void operator=(const compiler&) = delete;
private:
	class priv;
	priv* _private;
public:
	compiler(const char* root);
	bool compile_module(const char* module_name);
	bool execute_module(const char* module_name);
	~compiler();
};

}//namespace donkey

#endif /* __donkey_hpp__ */

#ifndef __donkey_object_hpp__
#define __donkey_object_hpp__

#include <string>

namespace donkey{

class variable;

class runtime_context;

class vtable;

class donkey_object{
	struct data;
	donkey_object(const donkey_object&) = delete;
	void operator=(const donkey_object&) = delete;
private:
	data* _data;
public:
	donkey_object(vtable* vt);
	vtable* get_vtable();
	variable& get_field(size_t i);
	void set_handle(const std::string& name, void* handle);
	void* get_handle(const std::string& name) const;
	const std::string& get_type_name() const;
	const std::string& get_module_name() const;
	std::string to_string() const;
	void dispose(const variable& v, runtime_context& ctx);
	~donkey_object();
};


}//donkey


#endif /* __donkey_object_hpp__ */

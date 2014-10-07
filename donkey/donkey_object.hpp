#ifndef __donkey_object_hpp__
#define __donkey_object_hpp__

#include <string>

namespace donkey{

class variable;

class vtable;

class donkey_object{
	struct data;
	donkey_object(const donkey_object&) = delete;
	void operator=(const donkey_object&) = delete;
private:
	data* _data;
public:
	donkey_object(std::string type_name, size_t fields_size, vtable* vt);
	vtable* get_vtable();
	variable& get_field(size_t i);
	const std::string& get_type_name() const;
	std::string to_string() const;
	~donkey_object();
};


}//donkey


#endif /* __donkey_object_hpp__ */

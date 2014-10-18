#include "donkey_object.hpp"
#include "variables.hpp"
#include "vtable.hpp"
#include "runtime_context.hpp"

#include <vector>


namespace donkey{

struct donkey_object::data{
	std::vector<variable> fields;
	vtable* vt;
	runtime_context* ctx;
	
	data(vtable* vt, runtime_context* ctx):
		fields(vt->get_fields_size()),
		vt(vt),
		ctx(ctx){
	}
};

donkey_object::donkey_object(vtable* vt, runtime_context* ctx):
	_data(new data(vt, ctx)){
	if(!_data){
		runtime_error("out of memory");
	}
}

vtable* donkey_object::get_vtable(){
	return _data->vt;
}

variable& donkey_object::get_field(size_t i){
	return _data->fields[i];
}

const std::string& donkey_object::get_type_name() const{
	return _data->vt->get_name();
}

const std::string& donkey_object::get_module_name() const{
	return _data->vt->get_module_name();
}

std::string donkey_object::to_string() const{
	return "object";
}

donkey_object::~donkey_object(){
	delete _data;
}

void donkey_object::dispose(const variable& v){
	_data->vt->call_destructor(v, *_data->ctx);
}


}//donkey

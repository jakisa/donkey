#include "donkey_object.hpp"
#include "variables.hpp"
#include "vtable.hpp"

#include <vector>


namespace donkey{

struct donkey_object::data{
	std::vector<variable> fields;
	vtable* vt;
	
	data(vtable* vt):
		fields(vt->get_fields_size()),
		vt(vt){
	}
};

donkey_object::donkey_object(vtable* vt):
	_data(new data(vt)){
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

std::string donkey_object::to_string() const{
	return "object";
}

donkey_object::~donkey_object(){
	delete _data;
}


}//donkey

#include "donkey_object.hpp"
#include "variables.hpp"

#include <vector>


namespace donkey{

struct donkey_object::data{
	std::vector<variable> fields;
	std::string type_name;
	
	data(std::string type_name, size_t fields_size):
		fields(fields_size),
		type_name(type_name){
	}
};

donkey_object::donkey_object(std::string type_name, size_t fields_size):
	_data(new data(type_name, fields_size)){
	if(!_data){
		runtime_error("out of memory");
	}
}

variable& donkey_object::get_field(size_t i){
	return _data->fields[i];
}

const std::string& donkey_object::get_type_name() const{
	return _data->type_name;
}

std::string donkey_object::to_string() const{
	return "object";
}

donkey_object::~donkey_object(){
	delete _data;
}


}//donkey

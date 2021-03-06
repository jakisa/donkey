#include "variables.hpp"
#include "vtable.hpp"

namespace donkey{

void variable::_runtime_error(std::string msg) const{
	if(is_weak(_vt) && _h_ptr->expired()){
		runtime_error("expired object access");
	}else{
		runtime_error(msg);
	}
}

void variable::_dec_counts_impl() const{
	switch(_vt){
		case var_type::object:
			_h_ptr->remove_shared_object(*this);
			break;
		case var_type::native:
		case var_type::string:
		case var_type::function:
			_h_ptr->remove_shared();
			break;
		case var_type::weak_string:
		case var_type::weak_object:
		case var_type::weak_function:
		case var_type::weak_native:
			_h_ptr->remove_weak();
			break;
		default:
			break;
	}
}

variable variable::call_functor(runtime_context& ctx, size_t params_size) const{
	vtable* vt = get_vtable();
	if(!vt->opCall){
		_runtime_error("opCall is not defined for " + get_full_type_name());
	}
	return (*vt->opCall)(*this, ctx, params_size);
}

void variable::_inc_counts_impl() const{
	if(is_shared(_vt)){
		_h_ptr->add_shared();
	}else{
		_h_ptr->add_weak();
	}
}

vtable* variable::get_vtable() const{	
	var_type dt = get_data_type();
	switch(dt){
		case var_type::number:
			return number_vtable().get();
		case var_type::code_address:
			return function_vtable().get();
		case var_type::nothing:
			return null_vtable().get();
		default:
			return _h_ptr->get_vtable();
	}
}

std::string variable::get_full_type_name() const{
	return get_vtable()->get_full_name();
}


std::string variable::to_string(runtime_context& ctx) const{
	switch(get_data_type()){
		case var_type::string:
			return std::string(as_string_unsafe());
		case var_type::number:
			return donkey::to_string(as_number_unsafe());
		case var_type::code_address:
		case var_type::function:
			return std::string("function");
		case var_type::object:
		case var_type::native:
			{
				vtable* vt = get_vtable();
				if(!vt->toString){
					return get_full_type_name();
				}
				return (*vt->toString)(*this, ctx, 0).as_string();
			}
		default:
			return std::string("null");
	}
}

bool variable::_to_bool(runtime_context& ctx) const{
	vtable* vt = get_vtable();
	if(!vt->toBool){
		_runtime_error("toBool is not defined for " + get_full_type_name());
	}
	return (*vt->toBool)(*this, ctx, 0).as_number();
}

}//donkey

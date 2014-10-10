#include "variables.hpp"


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
		case var_type::string:
			_h_ptr->remove_shared_array<char>();
			break;
		case var_type::object:
			_h_ptr->remove_shared<donkey_object>();
			break;
		case var_type::function:
			_h_ptr->remove_shared<function>();
			break;
		case var_type::weak_string:
		case var_type::weak_object:
		case var_type::weak_function:
			_h_ptr->remove_weak();
			break;
		default:
			break;
	}
}

void variable::_inc_counts_impl() const{
	if(is_shared(_vt)){
		_h_ptr->add_shared();
	}else{
		_h_ptr->add_weak();
	}
}


}//donkey

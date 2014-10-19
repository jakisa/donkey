#include "vtable.hpp"
#include "cpp/native_function.hpp"

namespace donkey{

class array{
private:
	array(const array&) = delete;
	void operator=(const array&) = delete;
	variable* _data;
	integer _cnt;
public:
	array(integer cnt):
		_data(new variable[cnt]),
		_cnt(cnt){
	}
	
	array(variable* data, integer cnt):
		_data(data),
		_cnt(cnt){
	}
	
	~array(){
		delete[] _data;
	}
	
	vtable* get_vtable(){
		return array_vtable().get();
	}
	
	variable* get_data(){
		return _data;
	}
	
	integer size(){
		return _cnt;
	}
	
	variable get_item(integer idx){
		return idx < 0 ? variable() : idx >= _cnt ? variable() : _data[idx];
	}
	
	void set_item_unsafe(integer idx, variable v){
		_data[idx] = v;
	}
	
	void set_item(integer idx, variable v){
		if(idx < 0 || idx >= _cnt){
			return;
		}
		set_item_unsafe(idx, v);
	}
	
	static variable clone(const variable& that, runtime_context& ctx, size_t){
		
		array* arr = that.as_t_unsafe<array>();
		
		std::unique_ptr<variable[]> p(new variable[arr->_cnt]);
		
		for(integer i = 0; i < arr->_cnt; ++i){
			p[i] = (*arr->_data[i].get_vtable()->get_method("clone"))(arr->_data[i], ctx, 0);
		}
		
		variable ret(new array(p.get(), arr->_cnt));
		
		p.release();
		
		return ret;
	}
};

variable create_initialized_array(variable* vars, size_t sz){
	return variable(new array(vars, integer(sz)));
}

std::pair<variable*, size_t> get_array_data_unsafe(const variable& v){
	array* arr =  v.as_t_unsafe<array>();
	return std::make_pair(arr->get_data(), size_t(arr->size()));
}

static variable create_array_helper(runtime_context& ctx, size_t current_param){
	integer sz = ctx.top(current_param).as_integer();
	
	variable ret(new array(sz));
	
	if(current_param > 0){
		for(integer i = 0; i != sz; ++i){
			ret.as_t_unsafe<array>()->set_item(i, create_array_helper(ctx, current_param-1));
		}
	}
	
	return ret;
}

static variable create_array(runtime_context& ctx, size_t params_count){
	if(params_count == 0){
		return variable(new array(0));
	}
	return create_array_helper(ctx, params_count-1);
}


vtable_ptr array_vtable(){
	static vtable_ptr ret;
	if(!ret){
		std::unordered_map<std::string, method_ptr> methods;
		
		methods.emplace("size", create_native_method("array::size", &array::size));
		methods.emplace("getItem", create_native_method("array::getItem", &array::get_item));
		methods.emplace("setItem", create_native_method("array::setItem", &array::set_item));
		
		ret.reset(new vtable("", "array", &create_array, std::move(methods), true));
		
		ret->derive_from(*object_vtable());
	}
	return ret;
}

}//donkey

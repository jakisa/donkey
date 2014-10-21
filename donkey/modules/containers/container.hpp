#ifndef __container_hpp__
#define __container_hpp__

#include "variables.hpp"
#include "vtable.hpp"

namespace donkey{

template<class T>
struct container_virtual_tables;

template<class T>
class iterator{
	iterator(const iterator&) = delete;
	void operator=(const iterator&) = delete;
private:
	typedef iterator<T> ThisType;
	typedef typename T::iterator IteratorType;
	
	variable _container;
	IteratorType _it;

	bool is_deleted(){
		return _container.get_data_type() == var_type::nothing;
	}
	
	void check_deleted(){
		if(is_deleted()){
			runtime_error("container is deleted");
		}
	}

	bool is_same(ThisType* oth){
		return _container.as_reference_unsafe() == oth->_container.as_reference_unsafe();
	}
	
	void check_same(ThisType* oth){
		if(!is_same(oth)){
			runtime_error("iterators to different containers detected");
		}
	}
	
	void check_not_begin(){
		if(_it == get_container().begin()){
			runtime_error("iterator points to begin");
		}
	}
	
	void check_not_end(){
		if(_it == get_container().end()){
			runtime_error("iterator points to end");
		}
	}
	
	T& get_container();
	
public:
	iterator(variable container, IteratorType it):
		_container(container.non_shared()),
		_it(it){
	}
	
	void pre_inc(){
		check_deleted();
		check_not_end();
		++_it;
	}
	
	variable post_inc(){
		auto it = _it;
		pre_inc();
		return variable(new iterator(_container, it));
	}
	
	void pre_dec(){
		check_deleted();
		check_not_begin();
		--_it;
	}
	
	variable post_dec(){
		auto it = _it;
		pre_dec();
		return variable(new iterator(_container, it));
	}
	
	integer diff(ThisType* oth){
		check_same(oth);
		check_deleted();
		return _it - oth->_it;
	}
	
	variable add(number n){
		check_deleted();
		
		variable ret(new iterator(_container, _it));
		
		ret.as_t_unsafe<ThisType>()->advance(n);
		
		return ret;
	}
	
	variable sub(variable voth){
		if(voth.get_var_type() == var_type::number){
			return add(-voth.as_integer_unsafe());
		}
		
		if(voth.get_vtable() != get_vtable()){
			runtime_error(get_vtable()->get_full_name() + " expected");
		}
		
		check_deleted();
		
		ThisType* oth = voth.as_t_unsafe<ThisType>();
		
		check_same(oth);
		return variable(_it - oth->_it);
	}
	
	variable get_item(){
		check_deleted();
		check_not_end();
		return *_it;
	}
	
	void set_item(variable v){
		check_deleted();
		check_not_end();
		*_it = v;
	}
	
	void advance(integer sz){
		check_deleted();
		if(sz > 0){
			if(get_container().end() - _it < sz){
				runtime_error("iterating after container");
			}
		}else if(sz < 0){
			if(get_container().begin() - _it > sz){
				runtime_error("iterating before container");
			}
		}
		_it += sz;
	}
	
	void advance_back(integer sz){
		return advance(-sz);
	}

	integer lt(ThisType* oth){
		check_same(oth);
		check_deleted();
		return _it < oth->_it;
	}
	
	integer gt(ThisType* oth){
		return oth->lt(this);
	}
	
	integer le(ThisType* oth){
		return !oth->lt(this);
	}
	
	integer ge(ThisType* oth){
		return !lt(oth);
	}
	
	integer eq(variable voth){
		if(voth.get_vtable() != get_vtable()){
			return 0;
		}
		
		ThisType* oth = voth.as_t_unsafe<ThisType>();
		
		if(!is_same(oth)){
			return 0;
		}
		
		if(is_deleted()){
			return 1;
		}
		
		return _it == oth->_it;
	}
	
	integer ne(variable voth){
		return !eq(voth);
	}
	
	integer to_bool(){
		check_deleted();
		return _it != get_container().end();
	}
	
	vtable* get_vtable(){
		return container_virtual_tables<T>::iterator();
	}
	
	static std::string full_type_name(){
		return container_virtual_tables<T>::iterator()->get_full_name();
	}
};

template<class T>
class container{
	container(const container&) = delete;
	void operator=(const container&) = delete;
	
	friend class iterator<T>;
private:
	typedef container<T> ThisType;
	T _data;
public:
	container(){
	}
	
	container(size_t sz):
		_data(sz){
	}
	
	container(variable* v, size_t sz):
		_data(v, v + sz){
	}
	
	vtable* get_vtable(){
		return container_virtual_tables<T>::main();
	}
	
	variable get_item(integer idx){
		if(idx < 0 || idx >= integer(_data.size())){
			runtime_error("subscript out of range");
		}
		return _data[idx];
	}
	
	void set_item(variable v, integer idx){
		if(idx < 0 || idx >= integer(_data.size())){
			runtime_error("subscript out of range");
		}
		_data[idx] = v;
	}
	
	variable front(){
		if(_data.empty()){
			runtime_error("container is empty");
		}
		return _data.front();
	}
	
	variable back(){
		if(_data.empty()){
			runtime_error("container is empty");
		}
		return _data.back();
	}
	
	number size(){
		return _data.size();
	}
	
	void resize(integer sz){
		if(sz < 0){
			sz = 0;
		}
		_data.resize(size_t(sz));
	}
	
	number capacity(){
		return _data.capacity();
	}
	
	void reserve(integer sz){
		if(sz < 0){
			sz = 0;
		}
		_data.reserve(size_t(sz));
	}
	
	number is_empty(){
		return _data.empty();
	}
	
	static variable clone(const variable& that, runtime_context& ctx, size_t){
		
		T& data = that.as_t_unsafe<ThisType>()->_data;
		
		std::unique_ptr<ThisType> p(new ThisType());
		
		p->_data.reserve(data.size());
		
		
		for(const variable& v: data){
			p->_data.push_back((*v.get_vtable()->get_method("clone"))(v, ctx, 0));
		}
		
		variable ret(p.get());
		
		p.release();
		
		return ret;
	}
	
	void push_back(variable v){
		_data.push_back(v);
	}
	
	void pop_back(){
		if(_data.empty()){
			runtime_error("container is empty");
		}
		_data.pop_back();
	}
	
	void push_front(variable v){
		_data.push_front(v);
	}
	
	void pop_front(){
		if(_data.empty()){
			runtime_error("container is empty");
		}
		_data.pop_front();
	}
	
	void clear(){
		_data.clear();
	}
	
	static variable create(runtime_context& ctx, size_t sz){
		if(sz == 0){
			return variable(new ThisType());
		}
		
		variable& v = ctx.top();
		
		if(v.get_data_type() == var_type::number){
			return variable(new ThisType(size_t(v.as_integer())));
		}
		
		if(v.get_vtable() == array_vtable().get()){
			auto data = get_array_data_unsafe(v);
			return variable(new ThisType(data.first, data.second));
		}
		
		runtime_error("number or array expected");
		
		return variable();
	}
	
	static variable begin(const variable& that, runtime_context&, size_t){
		return variable(new iterator<T>(that, that.as_t_unsafe<ThisType>()->_data.begin()));
	}
	
	static variable end(const variable& that, runtime_context&, size_t){
		return variable(new iterator<T>(that, that.as_t_unsafe<ThisType>()->_data.end()));
	}
	
	static std::string full_type_name(){
		return container_virtual_tables<T>::main()->get_full_name();
	}
};

template<class T>
T& iterator<T>::get_container(){
	return _container.as_t_unsafe<container<T> >()->_data;
}

}//donkey

#endif /*__container_hpp__*/


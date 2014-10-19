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
	
	void check_deleted(){
		if(_container.get_data_type() == var_type::nothing){
			runtime_error("container is deleted");
		}
	}
	
	void check_same(ThisType* oth){
		if(_container != oth->_container){
			runtime_error("iterators to different containers detected");
		}
	}
	
	T& get_container();
	
public:
	iterator(variable container, IteratorType it):
		_container(container.non_shared()),
		_it(it){
	}
	
	void inc(){
		check_deleted();
		if(_it == get_container().end()){
			runtime_error("iterating after container");
		}
		++_it;
	}
	
	void dec(){
		check_deleted();
		if(_it == get_container().begin()){
			runtime_error("iterating before container");
		}
		--_it;
	}
	
	integer diff(ThisType* oth){
		check_same(oth);
		check_deleted();
		return _it - oth->_it;
	}
	
	variable value(){
		check_deleted();
		return *_it;
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
	
	integer lt(ThisType* oth){
		check_same(oth);
		check_deleted();
		return _it < oth->_it;
	}
	
	integer gt(ThisType* oth){
		check_same(oth);
		check_deleted();
		return _it > oth->_it;
	}
	
	integer le(ThisType* oth){
		check_same(oth);
		check_deleted();
		return _it <= oth->_it;
	}
	
	integer ge(ThisType* oth){
		check_same(oth);
		check_deleted();
		return _it >= oth->_it;
	}
	
	integer eq(ThisType* oth){
		check_same(oth);
		check_deleted();
		return _it == oth->_it;
	}
	
	integer ne(ThisType* oth){
		check_same(oth);
		check_deleted();
		return _it != oth->_it;
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
	
	void set_item(integer idx, variable v){
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


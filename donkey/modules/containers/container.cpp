#include "container.hpp"
#include "vtable.hpp"
#include "cpp/native_function.hpp"
#include "cpp/native_module.hpp"
#include <vector>
#include <deque>
#include <list>

namespace donkey{

template<typename T>
static void add_sequence_methods(const std::string& type, std::unordered_map<std::string, method_ptr>& methods){
	methods.emplace("size", create_native_method("containers::"+type+"::size", &T::size));
	methods.emplace("resize", create_native_method("containers::"+type+"::resize", &T::resize));
	methods.emplace("front", create_native_method("containers::"+type+"::front", &T::front));
	methods.emplace("back", create_native_method("containers::"+type+"::back", &T::back));
	methods.emplace("is_empty", create_native_method("containers::"+type+"::is_empty", &T::is_empty));
	methods.emplace("push_back", create_native_method("containers::"+type+"::push_back", &T::push_back));
	methods.emplace("pop_back", create_native_method("containers::"+type+"::pop_back", &T::pop_back));
	methods.emplace("clear", create_native_method("containers::"+type+"::clear", &T::clear));
	methods.emplace("begin", method_ptr(new method(&T::begin)));
	methods.emplace("end", method_ptr(new method(&T::end)));
}

template<typename T>
static void add_sequence_iterator_methods(const std::string& type, std::unordered_map<std::string, method_ptr>& methods){
	methods.emplace("opGet", create_native_method("containers::"+type+"::opGet", &T::get_item));
	methods.emplace("opSet", create_native_method("containers::"+type+"::opSet", &T::set_item));
	methods.emplace("opPreInc", create_native_method("containers::"+type+"::preInc", &T::pre_inc));
	methods.emplace("opPreDec", create_native_method("containers::"+type+"::preDec", &T::pre_dec));
	methods.emplace("opPostInc", create_native_method("containers::"+type+"::postInc", &T::post_inc));
	methods.emplace("opPostDec", create_native_method("containers::"+type+"::postDec", &T::post_dec));
	methods.emplace("opEQ", create_native_method("containers::"+type+"::opEQ", &T::eq));
	methods.emplace("opNE", create_native_method("containers::"+type+"::opNE", &T::ne));
	
	methods.emplace("toBool", create_native_method("containers::"+type+"::toBool", &T::to_bool));
}

template<typename T>
static void add_random_iterator_methods(const std::string& type, std::unordered_map<std::string, method_ptr>& methods){
	
	methods.emplace("opLT", create_native_method("containers::"+type+"::opLT", &T::lt));
	methods.emplace("opGT", create_native_method("containers::"+type+"::opGT", &T::gt));
	methods.emplace("opLE", create_native_method("containers::"+type+"::opLE", &T::le));
	methods.emplace("opGE", create_native_method("containers::"+type+"::opGE", &T::ge));
	
	methods.emplace("opSub", create_native_method("containers::"+type+"::opSub", &T::sub));
	methods.emplace("opAdd", create_native_method("containers::"+type+"::opSub", &T::add));
	methods.emplace("opAddSet", create_native_method("containers::"+type+"::opAddSet", &T::advance));
	methods.emplace("opSubSet", create_native_method("containers::"+type+"::opSubSet", &T::advance_back));
}

template<typename T>
static void add_back_insertion_methods(const std::string& type, std::unordered_map<std::string, method_ptr>& methods){
	methods.emplace("push_back", create_native_method("containers::"+type+"::push_back", &T::push_back));
	methods.emplace("pop_back", create_native_method("containers::"+type+"::pop_back", &T::pop_back));
}

template<typename T>
static void add_front_insertion_methods(const std::string& type, std::unordered_map<std::string, method_ptr>& methods){
	methods.emplace("push_front", create_native_method("containers::"+type+"::push_front", &T::push_front));
	methods.emplace("pop_front", create_native_method("containers::"+type+"::pop_front", &T::pop_front));
}

template<typename T>
static void add_random_access_methods(const std::string& type, std::unordered_map<std::string, method_ptr>& methods){
	methods.emplace("opGet", create_native_method("containers::"+type+"::opGet", &T::get_item));
	methods.emplace("opSet", create_native_method("containers::"+type+"::opSet", &T::set_item));
}


vtable_ptr vector_vt(){
	typedef container<std::vector<variable> > vector;
	
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
		
		add_sequence_methods<vector>("Vector", methods);
		add_back_insertion_methods<vector>("Vector", methods);
		add_random_access_methods<vector>("Vector", methods);
		
		methods.emplace("reserve", create_native_method("containers::Vector::reserve", &vector::reserve));
		methods.emplace("capacity", create_native_method("containers::Vector::capacity", &vector::capacity));
	
		auto vt = new vtable(
			"containers",
			"Vector",
			&vector::create,
			std::move(methods),
			true
		);
			
		vt->derive_from(*object_vtable());
		return vt;
	}());
	
	return ret;
}

vtable_ptr vector_iterator_vt(){
	typedef iterator<std::vector<variable> > iterator;
	
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
	
		add_sequence_iterator_methods<iterator>("VectorIterator", methods);
		add_random_iterator_methods<iterator>("VectorIterator", methods);
		
		auto vt = new vtable(
			"containers",
			"VectorIterator",
			function(),
			std::move(methods),
			false
		);
			
		vt->derive_from(*object_vtable());
		return vt;
	}());
	
	return ret;
}

vtable_ptr deque_vt(){
	typedef container<std::deque<variable> > deque;
	
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
		
		add_sequence_methods<deque>("Deque", methods);
		add_back_insertion_methods<deque>("Deque", methods);
		add_front_insertion_methods<deque>("Deque", methods);
		add_random_access_methods<deque>("Deque", methods);
	
		auto vt = new vtable(
			"containers",
			"Deque",
			&deque::create,
			std::move(methods),
			true
		);
			
		vt->derive_from(*object_vtable());
		return vt;
	}());
	
	return ret;
}


vtable_ptr deque_iterator_vt(){
	typedef iterator<std::deque<variable> > iterator;
	
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
	
		add_sequence_iterator_methods<iterator>("DequeIterator", methods);
		add_random_iterator_methods<iterator>("DequeIterator", methods);
		
		auto vt = new vtable(
			"containers",
			"DequeIterator",
			function(),
			std::move(methods),
			false
		);
			
		vt->derive_from(*object_vtable());
		return vt;
	}());
	
	return ret;
}

vtable_ptr list_vt(){
	typedef container<std::list<variable> > list;
	
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
		
		add_sequence_methods<list>("List", methods);
		add_back_insertion_methods<list>("List", methods);
		add_front_insertion_methods<list>("List", methods);
	
		auto vt = new vtable(
			"containers",
			"List",
			&list::create,
			std::move(methods),
			true
		);
			
		vt->derive_from(*object_vtable());
		return vt;
	}());
	
	return ret;
}

vtable_ptr list_iterator_vt(){
	typedef iterator<std::list<variable> > iterator;
	
	static vtable_ptr ret([](){
		std::unordered_map<std::string, method_ptr> methods;
	
		add_sequence_iterator_methods<iterator>("ListIterator", methods);
		
		auto vt = new vtable(
			"containers",
			"ListIterator",
			function(),
			std::move(methods),
			false
		);
			
		vt->derive_from(*object_vtable());
		return vt;
	}());
	
	return ret;
}

template<>
struct container_virtual_tables<std::vector<variable> >{
	static vtable* main(){
		return vector_vt().get();
	}
	static vtable* iterator(){
		return vector_iterator_vt().get();
	}
};

template<>
struct container_virtual_tables<std::deque<variable> >{
	static vtable* main(){
		return deque_vt().get();
	}
	static vtable* iterator(){
		return deque_iterator_vt().get();
	}
};

template<>
struct container_virtual_tables<std::list<variable> >{
	static vtable* main(){
		return list_vt().get();
	}
	static vtable* iterator(){
		return list_iterator_vt().get();
	}
};

void add_containers_vtables(native_module& m){
	m.add_vtable(vector_vt());
	m.add_vtable(vector_iterator_vt());
	m.add_vtable(deque_vt());
	m.add_vtable(deque_iterator_vt());
	m.add_vtable(list_vt());
	m.add_vtable(list_iterator_vt());
}


}//donkey

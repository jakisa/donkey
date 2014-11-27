#ifndef __vtable_h__
#define __vtable_h__

#include "function.hpp"
#include "errors.hpp"
#include "variables.hpp"
#include "runtime_context.hpp"

#include <unordered_map>

namespace donkey{

class vtable;

struct base_class{
	const vtable* vt;
	size_t data_begin;
};

class vtable{
	void operator=(const vtable&) = delete;
private:
	std::string _full_name;
	std::string _module_name;
	std::string _name;
	std::unordered_map<std::string, base_class> _bases;
	std::unordered_map<std::string, method_ptr> _methods;
	std::unordered_map<std::string, size_t> _fields;
	method_ptr _constructor;
	method_ptr _destructor;
	size_t _fields_size;
	bool _is_public;
	bool _is_final;
	bool _is_native;
	function _creator;
	
	variable call_field(const variable& that, runtime_context& ctx, size_t params_size, const std::string& name) const;
	
	void update_predefined_methods();
	
	typedef method* pmethod;
public:
	pmethod opGet, opSet, opCall,
	        opEQ, opNE,
	        opLT, opGT, opLE, opGE,
	        opLTInv, opGTInv, opLEInv, opGEInv,
	        opNot,
	        opAnd, opOr, opXor,
	        opAndInv, opOrInv, opXorInv,
	        opAndSet, opOrSet, opXorSet,
	        opPreInc, opPreDec,
	        opPostInc, opPostDec,
	        opPos, opNeg,
	        opMul, opDiv, opIDiv, opMod, opAdd, opSub, opSL, opSR,
	        opMulInv, opDivInv, opIDivInv, opModInv, opAddInv, opSubInv, opSLInv, opSRInv,
	        opMulSet, opDivSet, opModSet, opAddSet, opSubSet, opSLSet, opSRSet;
	
	pmethod clone, strong, weak, toString, toBool;

	vtable(std::string&& module_name, std::string&& name, method_ptr constructor, method_ptr destructor,
	       std::unordered_map<std::string, method_ptr>&& methods, std::unordered_map<std::string, size_t>&& fields,
	       size_t fields_size, bool is_public, bool is_final);
	
	vtable(std::string&& module_name, std::string&& name, function creator, std::unordered_map<std::string, method_ptr>&& methods, bool is_public);
	
	variable create(runtime_context& ctx, size_t params_size) const;
	
	void call_base_constructor(const variable& that, runtime_context& ctx, size_t params_size) const;
	
	void call_constructor(const variable& that, runtime_context& ctx, size_t params_size) const;
	
	void call_base_destructor(const variable& that, runtime_context& ctx) const;
	
	void call_destructor(const variable& that, runtime_context& ctx) const;

	variable call_member(const variable& that, runtime_context& ctx, size_t params_size, const std::string& name) const;
	
	bool try_call_member(const variable& that, runtime_context& ctx, size_t params_size, const std::string& name, variable& rslt) const;
	
	variable& get_field(const variable& that, const std::string& name) const;
	
	variable call_method(const variable& that, runtime_context& ctx, size_t params_size, const std::string& type, method& m){
		if(_full_name != type && _bases.find(type) == _bases.end()){
			runtime_error(_full_name + " is not derived from " + type);
		}
		
		return m(that, ctx, params_size);
	}
	
	variable& get_field(const variable& that, const std::string& type, size_t idx){
		if(type == _full_name){
			return that.nth_field(idx);
		}
		auto it = _bases.find(type);
		if(it == _bases.end()){
			runtime_error(_full_name + " is not derived from " + type);
		}
		
		return that.nth_field(it->second.data_begin + idx);
	}
	
	bool has_member(const std::string& name) const{
		return has_method(name) || has_field(name);
	}
	
	bool has_method(const std::string& name) const{
		return _methods.find(name) != _methods.end();
	}
	
	bool has_field(const std::string& name) const{
		return _fields.find(name) != _fields.end();
	}
	
	method_ptr get_method(const std::string& name) const{
		return _methods.find(name)->second;
	}
	
	size_t get_field_index(const std::string& name) const{
		return _fields.find(name)->second;
	}
	
	size_t get_fields_size() const{
		return _fields_size;
	}
	
	void derive_from(const vtable& base);
	
	const std::string& get_name() const{
		return _name;
	}
	
	const std::string& get_module_name() const{
		return _module_name;
	}
	
	const std::string& get_full_name() const{
		return _full_name;
	}
	
	std::vector<const vtable*> get_bases() const;
	
	bool is_public() const{
		return _is_public;
	}
	
	bool is_final() const{
		return _is_final;
	}
	
	bool is_native() const{
		return _is_native;
	}
};

typedef std::shared_ptr<vtable> vtable_ptr;

vtable_ptr object_vtable();
vtable_ptr string_vtable();
vtable_ptr number_vtable();
vtable_ptr function_vtable();
vtable_ptr null_vtable();
vtable_ptr array_vtable();

variable create_initialized_array(variable* vars, size_t sz);
std::pair<variable*, size_t> get_array_data_unsafe(const variable& v);

}//donkey

#endif /*__vtable_h__*/

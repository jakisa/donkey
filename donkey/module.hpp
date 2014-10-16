#ifndef __module_hpp__
#define __module_hpp__

#include "function.hpp"
#include "runtime_context.hpp"
#include "statements.hpp"
#include "identifiers.hpp"

namespace donkey{

class module final: public identifier_lookup{
	module(const module&) = delete;
	void operator=(const module&) = delete;
private:
	std::vector<function> _functions;
	std::unordered_map<std::string, vtable_ptr> _vtables;
	statement _s;
	std::string _module_name;
	size_t _module_index;
	size_t _globals_count;
	
	std::unordered_map<std::string, size_t> _public_functions;
	std::unordered_map<std::string, size_t> _public_globals;
	
public:
	module(statement&& s,
	       std::string module_name,
	       size_t module_index,
	       size_t globals_count,
	       std::vector<function>&& functions,
	       std::unordered_map<std::string, vtable_ptr>&& vtables,
	       std::unordered_map<std::string, size_t>&& public_functions,
	       std::unordered_map<std::string, size_t>&& public_globals);
	       
	void load(runtime_context& ctx);
	
	size_t get_globals_count() const{
		return _globals_count;
	}
	
	variable call_function_by_index(size_t idx, runtime_context& ctx, size_t prms) const{
		return _functions[idx](ctx, prms);
	}
	
	vtable* get_vtable(std::string name) const{
		auto it = _vtables.find(name);
		return it == _vtables.end() ? nullptr : it->second.get();
	}
	
	virtual identifier_ptr get_identifier(std::string name) const override;
	
	virtual bool is_allowed(std::string) const override{
		return false;
	}
	
	virtual bool in_class() const override{	
		return false;
	}
	
	virtual std::string get_current_class() const override{
		return "";
	}
	
	virtual vtable* get_current_vtable() const override{
		return nullptr;
	}
	
	virtual const std::string& get_module_name() const override{
		return _module_name;
	}
	
	virtual std::vector<identifier_ptr> get_all_public() const override;
};

typedef std::shared_ptr<module> module_ptr;


}//donkey

#endif /*__module_hpp__*/


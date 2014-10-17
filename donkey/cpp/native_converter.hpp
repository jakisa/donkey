#ifndef __native_converter_hpp__
#define __native_converter_hpp__

#include "runtime_context.hpp"
#include "function.hpp"
#include "variables.hpp"

namespace donkey{

namespace detail{

	template<typename T>
	struct param_converter;
	
	template<>
	struct param_converter<void>{
		static void to_native(const variable&, runtime_context&){
		}
	};
	
	
	template<>
	struct param_converter<number>{
		static number to_native(const variable& v, runtime_context&){
			return v.as_number();
		}
		static variable from_native(number n, runtime_context&){
			return variable(n);
		}
	};
	
	template<>
	struct param_converter<number&>{
		static number& to_native(variable v, runtime_context&){
			return v.as_lnumber();
		}
	};
	
	template<>
	struct param_converter<integer>{
		static integer to_native(const variable& v, runtime_context&){
			return v.as_integer();
		}
		static variable from_native(integer n, runtime_context&){
			return variable(n);
		}
	};
	
	template<>
	struct param_converter<std::string>{
		static std::string to_native(const variable& v, runtime_context&){
			return v.to_string();
		}
		static variable from_native(const std::string& s, runtime_context&){
			return variable(s);
		}
	};
	
	template<>
	struct param_converter<const char*>{
		static const char* to_native(const variable& v, runtime_context&){
			return v.as_string();
		}
		static variable from_native(const char* s, runtime_context&){
			return variable(s);
		}
	};
	
	template<typename Derived>
	static void set_base_handle(typename Derived::base_type*, donkey_object* that, void* handle){
		typedef typename Derived::base_type Base;
		that->set_handle(Base::handle_name(), handle);
		set_base_handle<Base>(0, that, handle);
	}
	
	template<typename>
	static void set_base_handle(...){
	}
	
	template<class T>
	struct param_converter<T*>{
		static T* to_native(const variable& v, runtime_context&){
			return v.as_handle(T::handle_name());
		}
		
		static variable from_native(T* t, runtime_context& ctx){
			variable ret(t->get_vtable(), ctx);
			
			donkey_object* obj = ret.as_reference_unsafe()->as_t<donkey_object>();
			
			obj->set_handle(T::handle_name, t);
			
			set_base_handle<T>(nullptr, obj, t);
			
			return ret;
		}
		
	};
	
	template<class T>
	struct this_converter;
	
	template<>
	struct this_converter<std::string>{
		static std::string to_native(const variable& v, runtime_context&){
			return v.as_string();
		}
	};
	
	template<>
	struct this_converter<const char*>{
		static const char* to_native(const variable& v, runtime_context&){
			return v.as_string_unsafe();
		}
	};
	
	template<class T>
	struct this_converter<T*>{
		static T* to_native(const variable& v, runtime_context&){
			return v.as_handle_unsafe(T::handle_name());
		}
	};
	
}//detail

}//donkey

#endif /* __native_converter_hpp__ */

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
	struct param_converter<variable>{
		static variable to_native(const variable& v, runtime_context&){
			return v;
		}
		
		static variable to_native(variable&& v, runtime_context&){
			return v;
		}
		
		static variable from_native(const variable& v, runtime_context&){
			return v;
		}
		
		static variable from_native(variable&& v, runtime_context&){
			return v;
		}
	};
	
	
	template<typename T>
	struct param_converter<const T&>{
		static T to_native(const variable& v, runtime_context& ctx){
			return param_converter<T>::to_native(v, ctx);
		}
		static variable from_native(const T& t, runtime_context& ctx){
			return param_converter<T>::from_native(t, ctx);
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
		
		static variable from_native(std::string&& s, runtime_context&){
			return variable(std::move(s));
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
	
	template<class T>
	struct param_converter<T*>{
		static T* to_native(const variable& v, runtime_context&){
			return v.as_t<T>(T::full_type_name());
		}
		
		static variable from_native(T* t, runtime_context&){
			return variable(t);
		}
		
	};
	
	template<class T>
	struct this_converter;
	
	template<>
	struct this_converter<variable>{
		static variable to_native(const variable& v, runtime_context&){
			return v;
		}
	};
	
	template<>
	struct this_converter<const variable&>{
		static variable to_native(const variable& v, runtime_context&){
			return v;
		}
	};
	
	template<>
	struct this_converter<std::string>{
		static std::string to_native(const variable& v, runtime_context&){
			return v.as_string_unsafe();
		}
	};
	
	template<>
	struct this_converter<const std::string&>{
		static std::string to_native(const variable& v, runtime_context&){
			return v.as_string_unsafe();
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
			return v.as_t_unsafe<T>();
		}
	};
	
}//detail

}//donkey

#endif /* __native_converter_hpp__ */

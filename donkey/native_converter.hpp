#ifndef __native_converter_hpp__
#define __native_converter_hpp__

#include <variables.hpp>
#include <runtime_context.hpp>
#include <function.hpp>

namespace donkey{

namespace detail{

	template<typename T>
	struct param_converter;
	
	template<>
	struct param_converter<number>{
		static number to_native(const variable& v, runtime_context&){
			return v.as_number();
		}
		static variable from_native(number n){
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
		static variable from_native(integer n){
			return variable(n);
		}
	};
	
	template<>
	struct param_converter<std::string>{
		static std::string to_native(const variable& v, runtime_context&){
			return v.to_string();
		}
		static variable from_native(const std::string& s){
			return variable(s);
		}
	};
	
	template<>
	struct param_converter<const char*>{
		static const char* to_native(const variable& v, runtime_context&){
			return v.as_string();
		}
		static variable from_native(const char* s){
			return variable(s);
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
}//detail

}//donkey

#endif /* __native_converter_hpp__ */

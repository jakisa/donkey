#ifndef __native_function_hpp__
#define __native_function_hpp__

#include "function.hpp"

#include <tuple>
#include <type_traits>

namespace donkey{

namespace details{

	template<typename T>
	struct converter;
	
	template<>
	struct converter<std::string>{
		static std::string convert_this(variable& v){
			return std::string(v.as_string_unsafe());
		}
		
		static std::string convert(variable v){
			return v.to_string();
		}
	};
	
	template<>
	struct converter<double>{
		static double convert(variable v){
			return v.as_number();
		}
	};
	
	template<>
	struct converter<int>{
		static int convert(variable v){
			return (int)v.as_number();
		}
	};
	
	template<>
	struct converter<size_t>{
		static int convert(variable v){
			return (size_t)v.as_number();
		}
	};
	
}//details

template<typename F>
class native_method;

template<typename R, typename T, typename... Args>
class native_method<R(T, Args...)>{
private:
	typedef std::function<R(T,Args...)> F;
	F _f;

	template<typename RR, typename LT, typename RT>
	struct caller;
	
	
	template<typename RR, typename... ArgsL>
	struct caller<RR, std::tuple<ArgsL...>, std::tuple<> >{
		static variable call(size_t, F& f, runtime_context&, T t, ArgsL... argsl){
			return variable(f(t, argsl...));
		}
	};
	
	template<typename... ArgsL>
	struct caller<void, std::tuple<ArgsL...>, std::tuple<> >{
		static variable call(size_t, F& f, runtime_context&, T t, ArgsL... argsl){
			f(t, argsl...);
			return variable();
		}
	};
	
	template<typename RR, typename... ArgsL, typename M, typename... ArgsR>
	struct caller<RR, std::tuple<ArgsL...>, std::tuple<M, ArgsR...> >{
		static variable call(size_t idx, F& f, runtime_context& ctx, T t, ArgsL... argsl){
			variable m = ctx.stack[ctx.stack.size()-idx];
			return caller<
				RR,
				std::tuple<ArgsL...,M>,
				std::tuple<ArgsR...>
			>::call(
				idx-1, f, ctx, t, argsl..., details::converter<M>::convert(std::move(m))
			);
		}
	};
	
public:
	template<typename Any>
	native_method(Any f):
		_f(f){
	}

	variable operator()(variable& that, runtime_context& ctx, size_t sz){
		if(sizeof...(Args) > sz){
			runtime_error("not enough function parameters provided");
		}
		
		while(sz > sizeof...(Args)){
			ctx.stack.pop_back();
			--sz;
		}
	
		return caller<R, std::tuple<>, std::tuple<Args...> >::call(sz, _f, ctx, details::converter<T>::convert_this(that));
	}
};




}//donkey


#endif /*__native_function_hpp__*/

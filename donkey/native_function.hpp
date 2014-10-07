#ifndef __native_function_hpp__
#define __native_function_hpp__

#include "function.hpp"
#include "native_converter.hpp"
#include "donkey_callback.hpp"

#include <tuple>
#include <type_traits>

namespace donkey{

namespace detail{
	template<typename F, typename R, typename LT, typename RT>
	struct caller;
	
	template<typename F, typename R, typename... ArgsL>
	struct caller<F, R, std::tuple<ArgsL...>, std::tuple<> >{
		static variable call(size_t, const F& f, runtime_context&, ArgsL... argsl){
			return detail::param_converter<R>::from_native(f(argsl...));
		}
	};
	
	template<typename F, typename... ArgsL>
	struct caller<F, void, std::tuple<ArgsL...>, std::tuple<> >{
		static variable call(size_t, const F& f, runtime_context&, ArgsL... argsl){
			f(argsl...);
			return variable();
		}
	};
	
	template<typename F, typename R, typename... ArgsL, typename M, typename... ArgsR>
	struct caller<F, R, std::tuple<ArgsL...>, std::tuple<M, ArgsR...> >{
	
		static variable call(size_t idx, const F& f, runtime_context& ctx, ArgsL... argsl){
			variable m = idx == 0 ? variable() : ctx.stack[ctx.stack.size()-idx];
			return caller<
				F,
				R,
				std::tuple<ArgsL...,M>,
				std::tuple<ArgsR...>
			>::call(
				idx == 0 ? 0 : idx-1, f, ctx, argsl..., detail::param_converter<M>::to_native(std::move(m), ctx)
			);
		}
	};
	
	
	
	template<class Tupple, bool HasDflt, int Idx = 0>
	struct unpacker{
		static void unpack(Tupple, std::vector<variable>&){
		}
	};
	
	template<typename... TArgs, int Idx>
	struct unpacker<std::tuple<TArgs...>, true, Idx>{
		typedef std::tuple<TArgs...> Tupple;
		static void unpack(Tupple t, std::vector<variable>& dflts){
			dflts.push_back(detail::param_converter<typename std::tuple_element<Idx, Tupple>::type>::from_native(std::get<Idx>(t)));
			unpacker<Tupple, Idx+1 < sizeof...(TArgs), Idx+1>::unpack(t, dflts);
		}
	};
}//detail

template<typename R, typename... Args>
class native_function{
private:
	typedef std::function<R(Args...)> F;
	F _f;
	std::vector<variable> _dflts;
	
public:
	template<class D = std::tuple<> >
	native_function(F&& f, D d = D()):
		_f(std::move(f)){
		_dflts.reserve(std::tuple_size<D>::value);
		detail::unpacker<D, std::tuple_size<D>::value != 0, 0>::unpack(d, _dflts);
	}

	variable operator()(runtime_context& ctx, size_t sz){
		if(sz < sizeof...(Args)){
			size_t missing = sizeof...(Args) - sz;
			if(missing > _dflts.size()){
				runtime_error("not enough function parameters provided");
				return variable();
			}
			ctx.stack.insert(ctx.stack.end(), _dflts.end() - missing, _dflts.end());
			sz = sizeof...(Args);
		}
	
		return detail::caller<F, R, std::tuple<>, std::tuple<Args...> >::call(sz, _f, ctx);
	}
};

template<typename R, typename T, typename... Args>
class native_method{
private:
	typedef std::function<R(T,Args...)> F;
	F _f;
	std::vector<variable> _dflts;
	
public:
	template<class D = std::tuple<> >
	native_method(F&& f, D d = D()):
		_f(std::move(f)){
		_dflts.reserve(std::tuple_size<D>::value);
		detail::unpacker<D, std::tuple_size<D>::value != 0, 0>::unpack(d, _dflts);
	}

	variable operator()(variable& that, runtime_context& ctx, size_t sz){
		if(sz < sizeof...(Args)){
			size_t missing = sizeof...(Args) - sz;
			if(missing > _dflts.size()){
				runtime_error("not enough function parameters provided");
				return variable();
			}
			ctx.stack.insert(ctx.stack.end(), _dflts.end() - missing, _dflts.end());
			sz = sizeof...(Args);
		}
	
		return detail::caller<F, R, std::tuple<T>, std::tuple<Args...> >::call(sz, _f, ctx, detail::this_converter<T>::to_native(that, ctx));
	}
};

template<typename R, typename... Args>
function create_native_functions(R(*f)(Args...)){
	return native_function<R, Args...>(f);
}

template<class D, typename R, typename... Args>
function create_native_function(R(*f)(Args...), D d){
	return native_function<R, Args...>(f, d);
}

namespace detail{
	template<class F, typename R, typename... Args>
	function create_native_function(F f, R(F::*)(Args...)){
		return native_function<R, Args...>(std::forward<F>(f));
	}
	
	template<class F, typename R, typename... Args>
	function create_native_function(F f, R(F::*)(Args...) const){
		return native_function<R, Args...>(std::forward<F>(f));
	}
	
	template<class D, class F, typename R, typename... Args>
	function create_native_function(F f, D d, R(F::*)(Args...)){
		return native_function<R, Args...>(std::forward<F>(f), d);
	}
	
	template<class D, class F, typename R, typename... Args>
	function create_native_function(F f, D d, R(F::*)(Args...) const){
		return native_function<R, Args...>(std::forward<F>(f), d);
	}
}

template<class F>
function create_native_function(F f){
	return detail::create_native_function(f, &F::operator());
}

template<class F, class D>
function create_native_function(F f, D d){
	return detail::create_native_function(f, d, &F::operator());
}

template<typename R, typename T, typename... Args>
method create_native_method(R(*f)(T, Args...)){
	return native_method<R, T, Args...>(f);
}

template<class D, typename R, typename T, typename... Args>
method create_native_method(R(*f)(T, Args...), D d){
	return native_method<R, T, Args...>(f, d);
}

namespace detail{
	template<class F, typename R, typename T, typename... Args>
	method create_native_method(F f, R(F::*)(T, Args...)){
		return native_method<R, T, Args...>(std::forward<F>(f));
	}
	
	template<class F, typename R, typename T, typename... Args>
	method create_native_method(F f, R(F::*)(T, Args...) const){
		return native_method<R, T, Args...>(std::forward<F>(f));
	}
	
	template<class D, class F, typename R, typename T, typename... Args>
	method create_native_method(F f, D d, R(F::*)(T, Args...)){
		return native_method<R, T, Args...>(std::forward<F>(f), d);
	}
	
	template<class D, class F, typename R, typename T, typename... Args>
	method create_native_method(F f, D d, R(F::*)(T, Args...) const){
		return native_method<R, T, Args...>(std::forward<F>(f), d);
	}
}

template<class F>
method create_native_method(F f){
	return detail::create_native_method(f, &F::operator());
}

template<class F, class D>
method create_native_method(F f, D d){
	return detail::create_native_method(f, d, &F::operator());
}


namespace detail{
	template<typename R, typename... Args>
	struct param_converter<std::function<R(Args...)> >{
		typedef std::function<R(Args...)> F;
		
		static F to_native(variable v, runtime_context& ctx){
			return donkey_callback<R,Args...>(v, ctx);
		}
		
		static function from_native(F f){
			return donkey::create_native_function(f);
		}
	};
}

}//donkey


#endif /*__native_function_hpp__*/

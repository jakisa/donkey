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
		static variable call(size_t, const F& f, runtime_context& ctx, ArgsL... argsl){
			return detail::param_converter<R>::from_native(f(argsl...), ctx);
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
			variable m = ctx.top(idx);
			return caller<
				F,
				R,
				std::tuple<ArgsL...,M>,
				std::tuple<ArgsR...>
			>::call(
				idx-1, f, ctx, argsl..., detail::param_converter<M>::to_native(std::move(m), ctx)
			);
		}
	};
	
	
	
	template<class Tupple, bool HasDflt, size_t Idx>
	struct dflt_unpacker{
		static void unpack(size_t, Tupple, runtime_context&, stack_pusher&){
		}
	};
	
	template<typename... TArgs, size_t Idx>
	struct dflt_unpacker<std::tuple<TArgs...>, true, Idx>{
		typedef std::tuple<TArgs...> Tupple;
		static void unpack(size_t start_idx, Tupple t, runtime_context& ctx, stack_pusher& pusher){
			if(start_idx <= Idx){
				pusher.push(detail::param_converter<typename std::tuple_element<Idx, Tupple>::type>::from_native(std::get<Idx>(t), ctx));
			}
			dflt_unpacker<Tupple, Idx+1 < sizeof...(TArgs), Idx+1>::unpack(start_idx, t, ctx, pusher);
		}
	};
}//detail

template<typename D, typename R, typename... Args>
class native_function{
private:
	enum{
		dflts_size = std::tuple_size<D>::value
	};
	typedef std::function<R(Args...)> F;
	std::string _name;
	F _f;
	D _d;
public:
	native_function(std::string name, F&& f, D d = D()):
		_name(std::move(name)),
		_f(std::move(f)),
		_d(d){
	}

	variable operator()(runtime_context& ctx, size_t sz){
		try{
			stack_pusher pusher(ctx);
			if(sz < sizeof...(Args)){
				size_t missing = sizeof...(Args) - sz;
				if(missing > dflts_size){
					runtime_error("not enough function parameters provided");
					return variable();
				}
				
				detail::dflt_unpacker<D, dflts_size != 0, 0>::unpack(dflts_size - missing, _d, ctx, pusher);
				sz = sizeof...(Args);
			}
		
			return detail::caller<F, R, std::tuple<>, std::tuple<Args...> >::call(sz-1, _f, ctx);
		}catch(const runtime_exception& e){
			e.add_stack_trace(_name);
		}
		return variable();
	}
};

template<typename D, typename R, typename T, typename... Args>
class native_method{
private:
	enum{
		dflts_size = std::tuple_size<D>::value
	};
	typedef std::function<R(T,Args...)> F;
	std::string _name;
	F _f;
	D _d;
	
public:
	native_method(std::string name, F&& f, D d = D()):
		_name(std::move(name)),
		_f(std::move(f)),
		_d(d){
	}

	variable operator()(const variable& that, runtime_context& ctx, size_t sz){
		try{
			stack_pusher pusher(ctx);
			if(sz < sizeof...(Args)){
				size_t missing = sizeof...(Args) - sz;
				if(missing > dflts_size){
					runtime_error("not enough function parameters provided");
					return variable();
				}
				
				detail::dflt_unpacker<D, dflts_size != 0, 0>::unpack(dflts_size - missing, _d, ctx, pusher);
				sz = sizeof...(Args);
			}
		
			return detail::caller<F, R, std::tuple<T>, std::tuple<Args...> >::call(sz-1, _f, ctx, detail::this_converter<T>::to_native(that, ctx));
		}catch(const runtime_exception& e){
			e.add_stack_trace(_name);
		}
		return variable();
	}
};

template<typename R, typename... Args>
function create_native_functions(std::string name, R(*f)(Args...)){
	return native_function<std::tuple<>, R, Args...>(std::move(name), f);
}

template<class D, typename R, typename... Args>
function create_native_function(std::string name, R(*f)(Args...), D d){
	return native_function<D, R, Args...>(std::move(name), f, d);
}

namespace detail{
	template<class F, typename R, typename... Args>
	function create_native_function(std::string name, F f, R(F::*)(Args...)){
		return native_function<std::tuple<>, R, Args...>(std::move(name), std::forward<F>(f));
	}
	
	template<class F, typename R, typename... Args>
	function create_native_function(std::string name, F f, R(F::*)(Args...) const){
		return native_function<std::tuple<>, R, Args...>(std::move(name), std::forward<F>(f));
	}
	
	template<class D, class F, typename R, typename... Args>
	function create_native_function(std::string name, F f, D d, R(F::*)(Args...)){
		return native_function<D, R, Args...>(std::move(name), std::forward<F>(f), d);
	}
	
	template<class D, class F, typename R, typename... Args>
	function create_native_function(std::string name, F f, D d, R(F::*)(Args...) const){
		return native_function<D, R, Args...>(std::move(name), std::forward<F>(f), d);
	}
}

template<class F>
function create_native_function(std::string name, F f){
	return detail::create_native_function(std::move(name), f, &F::operator());
}

template<class F, class D>
function create_native_function(std::string name, F f, D d){
	return detail::create_native_function(std::move(name), f, d, &F::operator());
}

template<typename R, typename T, typename... Args>
method_ptr create_native_method(std::string name, R(*f)(T, Args...)){
	return method_ptr(new method(native_method<std::tuple<>, R, T, Args...>(std::move(name), f)));
}

template<class D, typename R, typename T, typename... Args>
method_ptr create_native_method(std::string name, R(*f)(T, Args...), D d){
	return method_ptr(new method(native_method<D, R, T, Args...>(std::move(name), f, d)));
}

template<typename R, typename T, typename... Args>
method_ptr create_native_method(std::string name, R (T::*f)(Args...)){
	return method_ptr(new method(native_method<std::tuple<>, R, T*, Args...>(std::move(name), f)));
}

namespace detail{
	template<class F, typename R, typename T, typename... Args>
	method_ptr create_native_method(std::string name, F f, R(F::*)(T, Args...)){
		return method_ptr(new method(native_method<std::tuple<>, R, T, Args...>(std::move(name), std::forward<F>(f))));
	}
	
	template<class F, typename R, typename T, typename... Args>
	method_ptr create_native_method(std::string name, F f, R(F::*)(T, Args...) const){
		return method_ptr(new method(native_method<std::tuple<>, R, T, Args...>(std::move(name), std::forward<F>(f))));
	}
	
	template<class D, class F, typename R, typename T, typename... Args>
	method_ptr create_native_method(std::string name, F f, D d, R(F::*)(T, Args...)){
		return method_ptr(new method(native_method<D, R, T, Args...>(std::move(name), std::forward<F>(f), d)));
	}
	
	template<class D, class F, typename R, typename T, typename... Args>
	method_ptr create_native_method(std::string name, F f, D d, R(F::*)(T, Args...) const){
		return method_ptr(new method(native_method<D, R, T, Args...>(std::move(name), std::forward<F>(f), d)));
	}
}

template<class F>
method_ptr create_native_method(std::string name, F f){
	return detail::create_native_method(std::move(name), f, &F::operator());
}

template<class F, class D>
method_ptr create_native_method(std::string name, F f, D d){
	return detail::create_native_method(std::move(name), f, d, &F::operator());
}


namespace detail{
	template<typename R, typename... Args>
	struct param_converter<std::function<R(Args...)> >{
		typedef std::function<R(Args...)> F;
		
		static F to_native(variable v, runtime_context& ctx){
			return donkey_callback<R,Args...>(v, ctx);
		}
		
		static function from_native(F f, runtime_context&){
			return donkey::create_native_function(f);
		}
	};
}

}//donkey


#endif /*__native_function_hpp__*/

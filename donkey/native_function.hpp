#ifndef __native_function_hpp__
#define __native_function_hpp__

#include "function.hpp"
#include "native_converter.hpp"

#include <tuple>
#include <type_traits>

namespace donkey{

template<typename F, typename D = std::tuple<> >
class native_method;

template<typename R, typename T, typename... Args>
class native_method<R(T, Args...)>{
private:
	typedef std::function<R(T,Args...)> F;
	F _f;
	std::vector<variable> _dflts;
	
	template<typename RR, typename LT, typename RT>
	struct _caller;
	
	template<typename RR, typename... ArgsL>
	struct _caller<RR, std::tuple<ArgsL...>, std::tuple<> >{
		static variable call(size_t, const F& f, runtime_context&, T t, ArgsL... argsl){
			return param_converter<R>::from_native(f(t, argsl...));
		}
	};
	
	template<typename... ArgsL>
	struct _caller<void, std::tuple<ArgsL...>, std::tuple<> >{
		static variable call(size_t, const F& f, runtime_context&, T t, ArgsL... argsl){
			f(t, argsl...);
			return variable();
		}
	};
	
	template<typename RR, typename... ArgsL, typename M, typename... ArgsR>
	struct _caller<RR, std::tuple<ArgsL...>, std::tuple<M, ArgsR...> >{
	
		static variable call(size_t idx, const F& f, runtime_context& ctx, T t, ArgsL... argsl){
			variable m = idx == 0 ? variable() : ctx.stack[ctx.stack.size()-idx];
			return _caller<
				RR,
				std::tuple<ArgsL...,M>,
				std::tuple<ArgsR...>
			>::call(
				idx == 0 ? 0 : idx-1, f, ctx, t, argsl..., param_converter<M>::to_native(std::move(m), ctx)
			);
		}
	};
	
	
	
	template<class Tupple, bool HasDflt, int Idx = 0>
	struct _unpacker{
		static void unpack(Tupple, std::vector<variable>&){
		}
	};
	
	template<typename... TArgs, int Idx>
	struct _unpacker<std::tuple<TArgs...>, true, Idx>{
		typedef std::tuple<Args...> Tupple;
		static void unpack(Tupple t, std::vector<variable>& dflts){
			dflts.push_back(param_converter<typename std::tuple_element<Idx, Tupple>::type>::from_native(std::get<Idx>(t)));
			_unpacker<Tupple, Idx+1 < sizeof...(TArgs), Idx+1>::unpack(t, dflts);
		}
	};
	
public:
	template<typename Any, class D = std::tuple<> >
	native_method(Any f, D d = D()):
		_f(f){
		_dflts.reserve(std::tuple_size<D>::value);
		_unpacker<D, std::tuple_size<D>::value != 0, 0>::unpack(d, _dflts);
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
	
		return _caller<R, std::tuple<>, std::tuple<Args...> >::call(sz, _f, ctx, this_converter<T>::to_native(that, ctx));
	}
};




}//donkey


#endif /*__native_function_hpp__*/

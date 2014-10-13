#ifndef __donkey_callback_hpp__
#define __donkey_callback_hpp__

#include "native_converter.hpp"
#include "function.hpp"
#include "runtime_context.hpp"
#include "stack_alloc.hpp"
#include <functional>

namespace donkey{

template<typename R, typename... Args>
class donkey_callback{
	typedef std::function<R(Args...)> F;
	variable _v;
	runtime_context& _ctx;
	
	variable _push_and_call(variable* params, const variable& v, runtime_context& ctx, size_t sz){
		return v.call(ctx, params, sz);
	}
	
	template<typename T, typename... Args2>
	variable _push_and_call(variable* params, const variable& v, runtime_context& ctx, size_t sz, T t, Args2... args){
		params[sz] = detail::param_converter<T>::from_native(t);
		return _push_and_call(params, v, ctx, sz+1, args...);
	}
	
public:
	donkey_callback(variable v, runtime_context& ctx):
		_v(v),
		_ctx(ctx){
	}
	R operator()(Args... args){
		STACK_ALLOC(params, sizeof...(Args));
		return detail::param_converter<R>::to_native(_push_and_call(params, _v, _ctx, 0, args...), _ctx);
	}
};


}//donkey

#endif /* __donkey_callback_hpp__ */

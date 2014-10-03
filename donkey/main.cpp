#include "donkey.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

#include "runtime_context.hpp"
#include "errors.hpp"
#include "helpers.hpp"


int main(){
	donkey::compiler c("../donkey/");
	
	c.compile_module("test");
	
	c.execute_module("test");
	return 0;
}



/*
#include <cstdio>
#include <tuple>
#include <type_traits>
#include <functional>

template<typename... Args>
struct C{
	template<typename L, typename R>
	struct caller;
	
	template<typename... ArgsL>
	struct caller<std::tuple<ArgsL...>, std::tuple<> >{
		static void call(std::function<void(Args...)>& f, ArgsL... argsl){
			f(argsl...);
		}
	};
	
	template<typename... ArgsL, typename M, typename... ArgsR>
	struct caller<std::tuple<ArgsL...>, std::tuple<M, ArgsR...> >{
		static void call(std::function<void(Args...)>& f, ArgsL... argsl){
			caller<std::tuple<ArgsL..., M>, std::tuple<ArgsR...> >::call(f, argsl..., M());
		}
	};
};

void test(int x, std::string str, double d){
	printf("%d %s, %f\n", x, str.c_str(), d);
}

int main(){
	
	typedef std::function<void(int, std::string, double)> F;
	
	F f(&test);
	
	C<int, std::string, double>::caller<std::tuple<>, std::tuple<int, std::string, double> >::call(f);
	

	return 0;
}
*/
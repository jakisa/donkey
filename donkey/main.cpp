#include "donkey.hpp"

#include <cstdio>
#include <vector>

#include "expression_builder.hpp"

using namespace donkey;

int main(){
	//compiler comp("../donkey/");

	//comp.compile_module("test");

	testRPN("a b*");

	return 0;
}

#include "donkey.hpp"

#include <cstdio>
#include <vector>

using namespace donkey;

int main(){
	compiler comp("../donkey/");

	comp.compile_module("test");

	return 0;
}

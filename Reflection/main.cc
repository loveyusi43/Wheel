#include <iostream>
#include <vector>

#include "Reflect/factory.h"
#include "Test/A.hpp"

int main(void)
{
	Object* a = Factory::Instance()->CreateClass("A");
	std::cout << a->Call("Func1", 1)+1 << std::endl;
	std::cout << a->Call("Func2", 2)+1 << std::endl;

	return 0;
}
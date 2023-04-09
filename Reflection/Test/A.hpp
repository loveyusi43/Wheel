#ifndef A_HPP
#define A_HPP

#include <string>
#include <iostream>
#include <vector>

#include "../Reflect/register.hpp"

class A final : public Object
{
public:
	A() = default;
	~A() override = default;

	void Show() override
	{
		std::cout << "A: " << name_ << std::endl;
	}

	int Func1(int num)
	{
		std::cout << "Func1(): " << num << std::endl;
		return num;
	}

	int Func2(int num)
	{
		std::cout << "Func2(): " << num << std::endl;
		return num;
	}

public:
	std::string name_;
	int age_ = 23;
	std::vector<int> v_;
};

REGISTER_CLASS(A);

REGISTER_CLASS_FIELD(A, name_, std::string);
REGISTER_CLASS_FIELD(A, age_, int);
REGISTER_CLASS_FIELD(A, v_, std::vector<int>);
REGISTER_CLASS_METHOD(A, Func1);
REGISTER_CLASS_METHOD(A, Func2);

#endif  // A_HPP
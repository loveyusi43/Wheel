#ifndef B_HPP
#define B_HPP

#include <string>
#include <iostream>

class B final : public Object
{
public:
	~B() override = default;

	void Show() override
	{
		std::cout << "B: " << name_ << std::endl;
	}
private:
	std::string name_;
};

REGISTER_CLASS(B);

#endif  // B_HPP
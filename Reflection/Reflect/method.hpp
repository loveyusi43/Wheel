#ifndef METHOD_HPP
#define METHOD_HPP

#include <string>

class Method
{
public:
	Method() = default;
	Method(const std::string_view& name, uintptr_t m_ptr) : method_name_(name), method_ptr_(m_ptr) {}

	const std::string& MethodName() const { return method_name_; }

	const uintptr_t& MethodPtr() const { return method_ptr_; }

protected:
	std::string method_name_;
	uintptr_t method_ptr_ = 0;
};

#endif // !METHOD_HPP
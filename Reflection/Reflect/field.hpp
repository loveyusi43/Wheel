#ifndef FIELD_HPP
#define FIELD_HPP

#include <string>

class Field
{
public:
	Field() = default;
	Field(const std::string_view& name, const std::string_view& type, const size_t& offset)
		: name_(name)
		, type_(type)
		, offset_(offset)
	{}
	~Field() = default;

	const std::string& Name() const { return name_; }
	const std::string& Type() const { return type_; }
	const size_t& Offset() const { return offset_; }

protected:
	std::string name_;
	std::string type_;
	size_t offset_;
};

#endif  // FIELD_H
#include <stdexcept>
#include <sstream>
#include <utility>

#include "json.h"
#include "parser.h"

int Json::format_level_ = 0;

// son::Json() : type_(JsonType::json_null), value_(0) {}

Json::Json(bool value) : type_(JsonType::json_bool), value_(value) {}

Json::Json(int32_t value) : type_(JsonType::json_int), value_(value) {}

Json::Json(double value) : type_(JsonType::json_double), value_(value) {}

Json::Json(const char* value) : type_(JsonType::json_string)
{
	value_ = std::string(value);
}

Json::Json(const std::string& value) : type_(JsonType::json_string), value_(value) {}

Json::Json(JsonType type) : type_(type)
{
	switch(type_)
	{
	case JsonType::json_null:
		break;
	case JsonType::json_int:
		value_ = int32_t();
		break;
	case JsonType::json_double:
		value_ = double();
		break;
	case JsonType::json_string:
		value_ = std::string();
		break;
	case JsonType::json_bool:
		value_ = bool();
		break;
	case JsonType::json_array:
		value_ = std::vector<Json>();
		break;
	case JsonType::json_object:
		value_ = std::unordered_map<std::string, Json>();
		break;
	default:
		break;
	}
}

Json::Json(const Json& other) = default;

bool Json::asBool() const
{
	if (type_ != JsonType::json_bool)
	{
		throw std::logic_error("类型错误");
	}
	return std::get<bool>(value_);
}
int Json::asInt() const
{
	if (type_ != JsonType::json_int)
	{
		throw std::logic_error("类型错误");
	}
	return std::get<int>(value_);
}

double Json::asDouble() const
{
	if (type_ != JsonType::json_double)
	{
		throw std::logic_error("类型错误");
	}
	return std::get<double>(value_);
}

std::string Json::asString() const
{
	if (type_ != JsonType::json_string)
	{
		throw std::logic_error("类型错误");
	}
	return std::get<std::string>(value_);
}

Json::operator bool() const
{
	return asBool();
}

Json::operator int() const
{
	return asInt();
}

Json::operator double() const
{
	return asDouble();
}


Json::operator std::string() const
{
	return asString();
}

Json& Json::operator [](size_t index)
{
	if (type_ != JsonType::json_array)
	{
		type_ = JsonType::json_array;
		value_ = std::vector<Json>();
	}
	return std::get<std::vector<Json>>(value_)[index];
}

void Json::append(const Json& other)
{
	if (type_ != JsonType::json_array)
	{
		type_ = JsonType::json_array;
		value_ = std::vector<Json>();
	}
	std::get<std::vector<Json>>(value_).push_back(other);
}


std::string Json::Str()
{
	std::stringstream ans;
	switch(type_)
	{
	case JsonType::json_string:
		ans << '\"' << std::get<std::string>(value_) << '\"';
		break;

	case JsonType::json_int:
		ans << std::get<int32_t>(value_);
		break;

	case JsonType::json_double:
		ans << std::get<double>(value_);
		break;

	case JsonType::json_array:
		ans << "[\n";
		++Count();
		// size_t s = std::get<std::vector<Json>>(value_).size();
		for (auto it = std::get<std::vector<Json>>(value_).begin(); it != std::get<std::vector<Json>>(value_).end(); ++it)
		{
			for (int i = 0; i < Count(); ++i)
			{
				ans << "    ";
			}
			ans << it->Str();
			if (it+1 != array_end())
			{
				ans << ',';
			}
			ans << '\n';
		}
		for (int i = 0; i < Count(); ++i)
		{
			ans << "    ";
		}
		for (int i = 0; i < Count(); ++i)
		{
			ans << "\b";
		}
		ans << ']';
		--Count();
		break;

	case JsonType::json_bool:
		if (std::get<bool>(value_))
		{
			ans << "true";
		}
		else
		{
			ans << "false";
		}
		break;

	case JsonType::json_null:
		ans << "null";
		break;

	case JsonType::json_object:
		ans << "{\n";
		++Count();
		for (auto it = object_begin(); it != object_end(); ++it)
		{
			for (int i = 0; i < Count(); ++i)
			{
				ans << "    ";
			}
			ans << '\"' << it->first << '\"' << ": " << it->second.Str() << ",\n";
		}
		for (int i = 0; i < Count(); ++i)
		{
			ans << "    ";
		}
		for (int i = 0; i < Count(); ++i)
		{
			ans << "\b";
		}
		ans << '}';
		--Count();
		break;

	default:
		break;
	}
	return ans.str();
}

void Json::Swap(Json& other)
{
	std::swap(type_, other.type_);
	std::swap(value_, other.value_);
}

Json& Json::operator=(Json other)
{
	Swap(other);
	return *this;
}

Json& Json::operator [](const char* key)
{
	return operator[](std::string(key));
}

Json& Json::operator [](const std::string& key)
{
	if (type_ != JsonType::json_object)
	{
		type_ = JsonType::json_object;
		value_ = std::unordered_map<std::string, Json>();
	}
	return std::get<std::unordered_map<std::string, Json>>(value_)[key];
}

bool Json::operator==(const Json& other) const
{
	return type_ == other.type_ && value_ == other.value_;
}

bool Json::operator!=(const Json& other) const
{
	return !(*this == other);
}

Json::array_iterator Json::array_begin()
{
	return std::get<std::vector<Json>>(value_).begin();
}
Json::array_iterator Json::array_end()
{
	return std::get<std::vector<Json>>(value_).end();
}

Json::object_iterator Json::object_begin()
{
	return std::get<std::unordered_map<std::string, Json>>(value_).begin();
}

Json::object_iterator Json::object_end()
{
	return std::get<std::unordered_map<std::string, Json>>(value_).end();
}

bool Json::Has(int index) const
{
	if (type_ != JsonType::json_array)
	{
		return false;
	}
	return static_cast<size_t>(index) < std::get<std::vector<Json>>(value_).size() && index >= 0;
}

//bool Json::Has(const char* key) const
//{
//	return Has(std::string(key));
//}

bool Json::Has(const std::string& key) const
{
	if (type_ != JsonType::json_object)
	{
		return false;
	}
	return std::get<std::unordered_map<std::string, Json>>(value_).count(key) != 0;
	//return std::get<std::unordered_map<std::string, Json>>(value_).find(key) != std::unordered_map<std::string, Json>::end();
}

void Json::Remove(int index)
{
	if (!Has(index))
	{
		throw std::logic_error("不存在的元素");
	}
	std::get<std::vector<Json>>(value_).erase(array_begin() + static_cast<int>(index));
}

//void Json::Remove(const char* key)
//{
//	return Remove(std::string(key));
//}

void Json::Remove(const std::string& key)
{
	if (!Has(key))
	{
		throw std::logic_error("不存在的元素");
	}
	std::get<std::unordered_map<std::string, Json>>(value_).erase(key);
}

void Json::Parse(const std::string_view& str)
{
	static Parser parser_s;
	parser_s.Load(str);
	*this = parser_s.Parse();
}
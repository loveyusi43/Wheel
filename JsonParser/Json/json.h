#ifndef JSON_H
#define JSON_H

#include <variant>
#include <string>
#include <vector>
#include <unordered_map>

class Json
{
public:
	enum class JsonType
	{
		json_null,
		json_bool,
		json_int,
		json_double,
		json_string,
		json_array,
		json_object
	};

	// Json();                           // 构造一个Json类型为null的Json
	Json(bool value);                 // Json obj = true or Json obj = true
	Json(int32_t value);              // Json obj = int()
	Json(double value);               // Json obj = double()
	Json(const char* value);          // Json obj = "hello world"
	Json(const std::string& value);   // Json obj = std::string{}
	Json(JsonType type = JsonType::json_null);              // Json obj = JsonType::
	Json(const Json& other);          // Json v(obj);

	operator bool() const;            // bool x1 = obj
	operator int() const;             // int x2 = obj
	operator double() const;          // double x3 = obj
	operator std::string() const;     // std::string x4 = obj

	Json& operator=(Json other);      // obj = v;

	void Swap(Json& other);

	// 除了重载int、double、bool、std::string进行隐式类型转换外，我们还希望通过显式的调用成员函数来返回基本类型
	bool asBool() const;
	int asInt() const;
	double asDouble() const;
	std::string asString() const;

	// 针对array的特殊操作
	Json& operator [](size_t index);
	void append(const Json& other);

	// 针对object的特殊操作
	Json& operator [](const char* key);
	Json& operator [](const std::string& key);

	// 判断两个Json相等与否
	bool operator==(const Json&) const;
	bool operator!=(const Json&) const;

	using array_iterator = std::vector<Json>::iterator;
	using object_iterator = std::unordered_map<std::string, Json>::iterator;

	// 对array或object进行遍历
	array_iterator array_begin();
	array_iterator array_end();
	object_iterator object_begin();
	object_iterator object_end();

	// 判断一个Json的type_字段
	bool isNull() const { return type_ == JsonType::json_null; }
	bool isBool() const { return type_ == JsonType::json_bool; }
	bool isInt() const { return type_ == JsonType::json_int; }
	bool isDouble() const { return type_ == JsonType::json_double; }
	bool isNumber() const { return isInt() || isDouble(); }
	bool isString() const { return type_ == JsonType::json_string; }
	bool isArray() const { return type_ == JsonType::json_array; }
	bool isObject() const { return type_ == JsonType::json_object; }

	// 判断array或object中有没有对应的元素以及删除元素
	bool Has(int index) const;
	bool Has(const std::string& key) const;
	void Remove(int index);
	void Remove(const std::string& key);

	// 初始化一个Parser对象用于解析Json格式的字符串
	void Parse(const std::string_view& str);

	// 对该Json对象内的数据格式化输出
	std::string Str();

	static int& Count() { return format_level_; }

private:
	JsonType type_;
	std::variant<bool, int32_t, double, std::string, std::vector<Json>, std::unordered_map<std::string, Json>> value_;
	static int format_level_;
};

#endif  // JSON_H
# Json Parser

Json的类型：

- null
- bool
- number
- string
- array
- object

## class Json

###基本字段

工欲善其事，必先利其器，在开始解析Json文本之前我们希望在C++中能有与Json相对应的类型。当然C++标准库肯定没有这种类型，所以在开始解析之前先定义一种或多种C++类型与上述的Json类型对应，为了代码的简洁性我们选择前者。想要用一种自定义类型对应Json的6种类型就需要在类标记该Json类对应那种Json数据类型。

```c++
class Json{
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
    // ...

private:
    JsonType type_;
	std::variant<bool, int32_t, double, std::string, std::vector<Json>, std::unordered_map<std::string, Json>> value_;
    // ...
};
```

于是引出了`enum class JsonType`与`type_`，通过`type_`就可以得出该自定义类型对应那种JsonType，而对应的数据则选择`value_`存储。`value_`的类型是[variant](https://zh.cppreference.com/w/cpp/utility/variant)，表示一个类型安全的联合体。

### 类型转换

为了方便使用，我们希望自定义类型Json具有一下行为：

```c++
class Json;
int x = 1;
double y = 1.2;
const char* str1 = "hello";
std::string str2{"world"};

Json j1; // 定义一个空类型
Json j2 = true;  // 定义一个bool类型
Json j3 = x;  // 创建一个number类型
Json j4 = y;  // 创建一个number类型
Json j5 = str1;  // 创建一个string类型
Json j6 = str2  // 创建一个string类型
Json j7(Json::JsonType::json_array); // 指明Json属于那种类型

bool a = j2;
int b = j3;
double = j4;
std::string s = j5;
```

为了实现上述这些操作需要完成下面这些成员函数：

```c++
class Json{
public:
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
    
    // 除了重载int、double、bool、std::string进行隐式类型转换外，我们还希望通过显式的调用成员函数来返回基本类型
	bool asBool() const;
	int asInt() const;
	double asDouble() const;
	std::string asString() const;
    // ...
};
```

### 对复合类型的特殊处理

当Json对应的类型为array或object时我们希望可以进行这样的操作：

```c++
Object obj1(json_array);
// 通过append对j1进行插入操作，因为json_array对应的类型是std::vector<Json>，所以append是尾插
j1.append(true);
j1.append(-3.54);
j1.append("hello world");
j1.append(j1);
// 通过[]返回对应下标处的引用
Json j1 = obj1[0];
j1 = obj1[2];

// json_object对应的类型为std::unordered_map<std::string, Json>
Json obj2(Json::JsonType::json_object);
// []操作的行为：如果哈希表没有则插入，有则返回引用
obj2["a"] = "hello";
obj2["b"] = -43.43;
obj2["c"] = true;
obj2["d"] = obj1;
```


#include <iostream>
#include <fstream>
#include <sstream>

#include "unit_test.h"
#include "json.h"

void UnitTest::TestArray()
{
	Json obj(Json::JsonType::json_array);
	obj.append("hello");
	obj.append(true);
	obj.append(-24.445);
	obj.append(obj);
	for (auto it = obj.array_begin(); it != obj.array_end(); ++it)
	{
		std::cout << it->Str() << ", ";
	}
}

void UnitTest::TestObject()
{
	Json j1(Json::JsonType::json_array);
	j1.append("world");
	j1.append(false);
	j1.append(654);
	Json obj(Json::JsonType::json_object);
	obj["a"] = "hello";
	obj["b"] = -43.43;
	obj["c"] = true;
	obj["d"] = j1;
	std::cout << obj.Str();
}

void UnitTest::TestParse()
{
	std::string path = "./";
	std::string name = "test.json";
	std::ifstream ifs{ path + name };
	if (!ifs)
	{
		std::cout << "打开文件失败\n";
		exit(1);
	}
	std::stringstream ss;
	ss << ifs.rdbuf();
	Json obj(Json::JsonType::json_object);
	obj.Parse(ss.str());
	std::cout << obj.Str();
}
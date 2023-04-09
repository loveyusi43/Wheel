#ifndef PARSER_H
#define PARSER_H

#include <string>

#include "json.h"

class Parser
{
public:
	Parser() = default;

	Parser(const std::string_view& source) : source_(source) {}

	void Load(const std::string_view& str);

	Json Parse();

private:
	void IgnoreWhitespace();

	// 让index_指向当前正在解析的字符的下一个非空白字符
	// 即每次调用该函数时index_都会自动向后走
	char& GetNextToken();

	// 针对每种类型的Json进行解析
	Json ParseNull();
	Json ParseBool();
	Json ParseNumber();
	std::string ParseString();
	Json ParseArray();
	Json ParseObject();

protected:
	std::string source_;  // 保存一个被解析的字符串
	size_t index_{};     // 记录当前解析到的位置
};

#endif  // PARSER_H
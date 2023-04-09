#include <stdexcept>
#include <locale>

#include "parser.h"

void Parser::Load(const std::string_view& str)
{
	source_ = str;
	index_ = 0;
}

void Parser::IgnoreWhitespace()
{
	while (' '==source_[index_] || '\n'==source_[index_] || '\t'==source_[index_] || '\r'==source_[index_])
	{
		++index_;
	}
}

char& Parser::GetNextToken()
{
	IgnoreWhitespace();
	return source_[index_++];
}

Json Parser::Parse()
{
	char ch = GetNextToken();
	switch (ch)
	{
	case 'n':
		--index_;
		return ParseNull();
	case 't':
	case 'f':
		--index_;
		return ParseBool();
	case '+':
	case '-':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '0':
		--index_;
		return ParseNumber();
	case '\"':
		return Json(ParseString());
	case '[':
		return ParseArray();
	case '{':
		return ParseObject();
	default:
		throw std::logic_error("未定义的字符");
		return Json();
	}
}

Json Parser::ParseNull()
{
	static std::string cmp = "null";
	if (source_.compare(index_, cmp.size(), cmp) != 0)
	{
		throw std::logic_error("parse null error");
	}
	index_ += cmp.size();
	return Json();
}

Json Parser::ParseBool()
{
	static std::string t_str = "true";
	static std::string f_str = "false";
	if (source_.compare(index_, t_str.size(), t_str) == 0)
	{
		index_ += t_str.size();
		return Json(true);
	}
	else if (source_.compare(index_, f_str.size(), f_str) == 0)
	{
		index_ += f_str.size();
		return Json(false);
	}
	else
	{
		throw std::logic_error("parse bool error");
	}
	return Json();
}

Json Parser::ParseNumber()
{
	size_t pos = index_;
	if (source_[index_] == '-' || source_[index_] == '+')
	{
		++index_;
	}
	if (!isdigit(source_[index_]))
	{
		throw std::logic_error("parse number error");
	}
	while (isdigit(source_[index_]))
	{
		++index_;
	}
	if (source_[index_] != '.')
	{
		return Json(std::stoi(source_.substr(pos, index_ - pos)));
	}
	++index_;
	if (!isdigit(source_[index_]))
	{
		throw std::logic_error("parse number error");
	}
	while (isdigit(source_[index_]))
	{
		++index_;
	}
	return Json(std::stod(source_.substr(pos, index_ - pos)));
}

std::string Parser::ParseString()
{
	const size_t start = index_;
	const size_t finish = source_.find('\"', start);
	if(finish == std::string::npos)
	{
		throw std::logic_error("parse string error");
	}
	index_ = finish + 1;
	return source_.substr(start, finish - start);
}

Json Parser::ParseArray()
{
	Json array(Json::JsonType::json_array);
	char ch = GetNextToken();
	if (ch == ']')
	{
		return array;
	}
	--index_;
	while (true)
	{
		array.append(Parse());
		ch = GetNextToken();
		if (index_ > source_.size())
		{
			throw std::logic_error("parse array error");
		}
		if (ch == ']')
		{
			break;
		}
		if (ch != ',')
		{
			throw std::logic_error("parse array error");
		}
	}
	return array;
}

Json Parser::ParseObject()
{
	Json obj(Json::JsonType::json_object);
	char ch = GetNextToken();
	if (ch == '}')
	{
		return obj;
	}
	--index_;

	while (true)
	{
		ch = GetNextToken();
		if (index_ > source_.size())
		{
			throw std::logic_error("parse object error");
		}
		if (ch != '\"')
		{
			throw std::logic_error("parse object error");
		}
		std::string key = ParseString();
		ch = GetNextToken();
		if (ch != ':')
		{
			throw std::logic_error("parse object error");
		}
		obj[key] = Parse();
		ch = GetNextToken();
		if (ch == '}')
		{
			break;
		}
		if (ch != ',')
		{
			throw std::logic_error("parse object error");
		}
	}

	return obj;	
}
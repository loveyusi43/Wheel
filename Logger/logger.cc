#define _CRT_SECURE_NO_WARNINGS

#include <mutex>
#include <stdexcept>
#include <format>
#include <cstdarg>
#include <cstdio>

#include "logger.h"

const std::vector<std::string> Logger::level_str_{
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR",
	"FATAL"
};

Logger* Logger::instance_ = nullptr;


Logger* Logger::Instance()
{
	static std::mutex mtx;
	if (nullptr == instance_)
	{
		mtx.lock();
		if (nullptr == instance_)
		{
			instance_ = new Logger();
		}
		mtx.unlock();
	}
	return instance_;
}

void Logger::Open(const std::string_view& filename)
{
	file_name_ = filename;
	file_out_.open(file_name_, std::ios::app);
	if (file_out_.fail())
	{
		throw std::logic_error(std::format("打开文件：[{}]失败", filename));
	}

	file_out_.seekp(0, std::ios::end);
	len_ = static_cast<size_t>(file_out_.tellp());
}

void Logger::Close()
{
	file_out_.close();
}


void Logger::Log(Level level, const std::source_location& location, const char* format, ...)
{
	if (level < rank_)
	{
		return;
	}

	if (file_out_.fail())
	{
		throw std::logic_error(std::format("打开文件：[{}]失败", file_name_));
	}
	std::string msg = std::format("{} {} [ {} ] : {} : {}", CurrentTime(), level_str_[level], location.file_name(), location.line(), location.function_name()) + " --" + '\n' + "-->";
	len_ += msg.size();
	file_out_ << msg;

	va_list arg_ptr;
	va_start(arg_ptr, format);
	const int size = vsnprintf(nullptr, 0, format, arg_ptr);
	va_end(arg_ptr);
	if (size > 0)
	{
		char* content = new char[size + 1];
		va_start(arg_ptr, format);
		vsnprintf(content, size + 1, format, arg_ptr);
		va_end(arg_ptr);
		file_out_ << content << std::endl;
		len_ += size;
		delete[]content;
	}
	file_out_.flush();

	if (len_ >= max_ && max_ > 0)
	{
		Rotate();
	}
}

std::string Logger::CurrentTime() const
{
	const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

	const time_t ticks = std::chrono::system_clock::to_time_t(now);

	std::tm* now_tm = std::localtime(&ticks);

	return std::format("{:0>4}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}", now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
}

void Logger::Rotate()
{
	Close();
	file_out_.clear();
	std::string newname = file_name_ + '.' + CurrentTime();
	if (rename(file_name_.c_str(), newname.c_str()) != 0)
	{
		throw std::logic_error("文件重命名失败");
	}
	Open(file_name_);
}
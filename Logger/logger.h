#ifndef LOGGER_H
#define LOGGER_H

#include <source_location>
#include <string>
#include <format>
#include <fstream>
#include <vector>

#define debug(format, ...) \
	Logger::Instance()->Log(Logger::Level::DEBUG, std::source_location::current(), format, ##__VA_ARGS__)

#define info(format, ...) \
	Logger::Instance()->Log(Logger::Level::INFO, std::source_location::current(), format, ##__VA_ARGS__)

#define warning(format, ...) \
	Logger::Instance()->Log(Logger::Level::WARN, std::source_location::current(), format, ##__VA_ARGS__)

#define error(format, ...) \
	Logger::Instance()->Log(Logger::Level::ERROR, std::source_location::current(), format, ##__VA_ARGS__)

#define fatal(format, ...) \
	Logger::Instance()->Log(Logger::Level::FATAL, std::source_location::current(), format, ##__VA_ARGS__)

class Logger
{
public:
	enum Level
	{
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL
	};

	static Logger* Instance();

	void Open(const std::string_view& filename);
	void Close();

	std::string CurrentTime() const;

	void Log(Level level, const std::source_location& location, const char* format, ...);

	// 只记录level及其以上的等级的日志
	void Rank(Level level) { rank_ = level; }

	void Max(size_t bytes) { max_ = bytes; }

private:
	Logger() = default;
	void Rotate();

protected:
	size_t max_ = 0;
	size_t len_ = 0;
	std::string file_name_;
	std::ofstream file_out_;
	Level rank_ = Level::DEBUG;
	static const std::vector<std::string> level_str_;
	static Logger* instance_;
};

#endif  // LOGGER_H
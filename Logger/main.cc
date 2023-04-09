#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <format>
#include <source_location>
#include <fstream>

#include "logger.h"

int main()
{
	try
	{
		std::string filename = "./test.log";
		Logger::Instance()->Open(filename);
		Logger::Instance()->Max(200);
		debug("debug");
		info("info");
		warning("warning");
		error("error");
		fatal("fatal");
	}
	catch (const std::exception& e)
	{
		std::cout << strerror(errno) << std::endl;
		std::cout << e.what();
	}
    return 0;
}
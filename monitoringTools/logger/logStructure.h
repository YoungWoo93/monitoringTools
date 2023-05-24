#pragma once
#include <chrono>
#include <string>

#include "logger.h"

struct logStructure {
	logStructure(const logMessage& message);

	void timeParser(const std::chrono::system_clock::time_point& tp);
	void pointParser(const std::string& cppName, const int line, const int threadID);
	void logLevelParser(const logLevel level);

	std::string _time;
	std::string _where;
	std::string _level;
	std::string _message;
	unsigned int days;
};
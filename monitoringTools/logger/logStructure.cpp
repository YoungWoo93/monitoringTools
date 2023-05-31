#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>

#include "logger.h"
#include "logStructure.h"

logStructure::logStructure(const logMessage& msg) : _message(msg.message)
{
	timeParser(msg.time);
	pointParser(msg.logCppName, msg.line, msg.threadID);
	logLevelParser(msg.level);
	
	// 애초 로그 writeReq 시점에서 이 작업을 해서 줘도 되지 않을까?
	// 근데 그러면 직접 작업을 하는 스레드에서 실제 작업과 상관없는 퀀텀 소요가 늘지 않을까?
	// 누군가가 해야한다면 개념적으로 실제 작업 스레드에서는 작업을, 로그관련 작업은 로그 스레드에서 하는게 맞지 않을까?
}

void logStructure::timeParser(const std::chrono::system_clock::time_point& tp)
{
	std::stringstream ss;

	unsigned long long int msTime = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
	int ms = msTime % 1000;
	int sec = (msTime /= 1000) % 60;
	int min = (msTime /= 60) % 60;
	int hour = (msTime = (msTime /= 60) + 9) % 24;

	days = (unsigned int)(msTime / 24);

	ss << "[" << std::setw(2) << std::setfill('0') << hour <<
		":" << std::setw(2) << std::setfill('0') << min <<
		":" << std::setw(2) << std::setfill('0') << sec <<
		"." << std::setw(3) << std::setfill('0') << ms <<
		"]";

	_time = ss.str();
}
void logStructure::pointParser(const std::string& cppName, const int line, const int threadID)
{
	std::stringstream ss;
	ss << "[" << cppName << ":" << line << " (threadID :" << threadID << ")]";

	_where = ss.str();
}

void logStructure::logLevelParser(const logLevel level)
{
	switch (level)
	{
	case logLevel::Off:
		_level = "[ OFF ]";
		break;
	case logLevel::Fatal:
		_level = "[Fatal]";
		break;
	case logLevel::Error:
		_level = "[Error]";
		break;
	case logLevel::Warning:
		_level = "[Warning]";
		break;
	case logLevel::Info:
		_level = "[Info]";
		break;
	case logLevel::Debug:
		_level = "[Debug]";
		break;
	}
}


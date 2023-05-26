#pragma once

#include <sstream>
#include <string>

#include "logger/logger.h"

#define LOG(...)                            writeReq(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__);


#define LOGOUT(level, mode)                 do { logLevel __level = level; char __MODE = mode;  std::string __FILENAME = ""; std::stringstream __ss; __ss
#define LOGOUT_EX(level, mode, fileName)    do {logLevel __level = level; char __MODE = mode;  std::string __FILENAME = fileName; std::stringstream __ss; __ss

#define LOGEND                              ""; writeReq(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __level, __MODE, __ss.str(), __FILENAME); } while(0)



void writeReq(const std::string& _logCppName,
	const int _line,
	const logLevel _level,
	const char _writeMode = LO_TXT,
	const std::string _message = "",
	const std::string _logWriteFileName = "");


/////////////////////////////////////////////////////////////
// example
/////////////////////////////////////////////////////////////
//
//	#include "../monitoringTools/messageLogger.h"
//	
//	void main()
//	{
//		LOG(logLevel::Info, LO_TXT | LO_CMD, "test info log out text, cmd");
//		LOGOUT(logLevel::Info, LO_TXT | LO_CMD) << "test info log out text, cmd " << "by stream" << LOGEND;
//		LOGOUT_EX(logLevel::Info, LO_TXT | LO_CMD, "extraFile") << "test info log out text, cmd " << "by stream " << "to extra.txt file" << LOGEND;
//	
//		Sleep(100); //로깅 이벤트가 전달되기 전에 로깅 스레드가 종료되는것을 지연시키기 위한 sleep
//	}
//
/////////////////////////////////////////////////////////////
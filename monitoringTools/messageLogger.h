#pragma once

#include <sstream>
#include <string>

#include "logger/logger.h"

void writeReq(const std::string& _logCppName,
	const int _line,
	const logLevel _level,
	const char _writeMode = LO_TXT,
	const std::string _message = "",
	const std::string _logWriteFileName = "");



#define LOG(...)                            writeReq(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__);


#define LOGOUT(level, mode)                 do { logLevel __level = level; char __MODE = mode;  std::string __FILENAME = ""; std::stringstream __ss; __ss
#define LOGOUT_EX(level, mode, fileName)    do {logLevel __level = level; char __MODE = mode;  std::string __FILENAME = fileName; std::stringstream __ss; __ss

#define LOGEND                              ""; writeReq(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __level, __MODE, __ss.str(), __FILENAME); } while(0)

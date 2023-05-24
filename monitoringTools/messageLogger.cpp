#include "messageLogger.h"
#include "logger/logger.h"

extern logger g_mylogger;


void writeReq(const std::string& _logCppName,
	const int _line,
	const logLevel _level,
	const char _writeMode,
	const std::string _message,
	const std::string _logWriteFileName)
{
	if (!g_mylogger.runFlag)
		return;

	if ((int)g_mylogger.levelThreshold < (int)_level)
		return;

	EnterCriticalSection(&g_mylogger.poolCS);
	if (g_mylogger.logMessagePool.empty()){
		for (int i = 0; i < 1000; i++)
			g_mylogger.logMessagePool.push(new logMessage);
	}
	logMessage* msg = g_mylogger.logMessagePool.top();
	g_mylogger.logMessagePool.pop();
	LeaveCriticalSection(&g_mylogger.poolCS);

	msg->level = _level;
	msg->writeMode = _writeMode;
	msg->line = _line;
	msg->threadID = GetCurrentThreadId();
	msg->time = std::chrono::system_clock::now();
	msg->logCppName = _logCppName;
	msg->message = _message;
	msg->logWriteFileName = _logWriteFileName;

	EnterCriticalSection(&g_mylogger.queueCS);
	g_mylogger.logMessageQueue.push(msg);
	LeaveCriticalSection(&g_mylogger.queueCS);

	SetEvent(g_mylogger.loggingEvent);
}
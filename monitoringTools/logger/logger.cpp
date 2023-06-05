#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <sstream>
#include <thread>
#include <Windows.h>

#include "../dump.h"

#include "logger.h"
#include "logStructure.h"
logger g_mylogger;

void TEXTstringify(const logStructure& logStr, std::string& str)
{
	str = logStr._time + "\t" + logStr._where + "\t" + logStr._level + "\t" + logStr._message + "\n";
}
void CSVstringify(const logStructure& logStr, std::string& str)
{
	std::string tempMessage = logStr._message;
	str = logStr._time + "," + logStr._where + "," + logStr._level + ",";

	if (tempMessage.find(',') != std::string::npos || tempMessage.find('\"') != std::string::npos)
	{
		str += '\"';
		for (auto character : tempMessage)
		{
			if (character == '\"')
				str += '\"';

			str += character;
		}
		str += '\"';
		str += ",";
	}
	else
	{
		str += tempMessage;
		str += ",";
	}
	str += "\n";
}
bool createDirectoryNotExists(std::string& dir) {
	dir += "\\";

	for (size_t pos = dir.find('\\', 0); pos != std::string::npos; pos = dir.find('\\', pos + 1))
	{
		std::string temp = dir.substr(0, pos);
		if (CreateDirectoryA(dir.substr(0, pos).c_str(), NULL) == 0) {
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				return false;
		}
	}

	return true;
}

bool writeFile(const std::string& fileName, const std::string& line)
{
	HANDLE logFile = CreateFileA(
		fileName.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (logFile == INVALID_HANDLE_VALUE) {
		std::cout << "Error opening file: " << GetLastError() << std::endl;
		return false;
	}

	SetFilePointer(logFile, 0, NULL, FILE_END);
	DWORD bytesWritten = 0;
	if (!WriteFile(logFile, line.c_str(), (DWORD)line.size(), &bytesWritten, NULL)) {
		std::cout << "Error writing to file: " << GetLastError() << std::endl;
		return false;
	}

	CloseHandle(logFile);

	return true;
}

logger::logger(logLevel _loggingLevel) : loggingLevel(_loggingLevel)
{
	std::stringstream dirName;
	unsigned long long int msTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	dirName << "LOG\\" << msTime;

	days = (unsigned int)((msTime = (msTime /= 3600000) + 9) / 24);
	setLoggingDir(dirName.str());

	InitializeCriticalSection(&queueCS);
	InitializeCriticalSection(&poolCS);

	for (int i = 0; i < 1000; i++)
		logMessagePool.push(new logMessage);

	loggingEvent = CreateEventA(nullptr, true, false, "loggingEvent");
	if (loggingEvent == (HANDLE)(-1))
	{
		std::cout << "create logging event fail, Error " << GetLastError() << std::endl;
		dump::crash();
	}

	runFlag = true;
	loggingThread = new std::thread(&logger::loggingThreadProc, this);
}
logger::~logger() {
	runFlag = false;
	SetEvent(loggingEvent);
	// 이부분 경합 발생 가능
	// 1. 로거의 소멸자에서 flag를 끔
	// 2. 로깅 스레드에서 로그를 적기위해 queue락을 획득한 상태였음
	// 3. 그러나 임의의 스레드에서 찰나에 로그를 적기위해 queue락 획득 대기상태였음
	// 4. 로깅 스레드에서는 현재 있는 로그를 다 적은뒤 flag를 확인해서 join 됨
	// 5. 임의의 스레드는 그제아 락을 획득하기위해 CS에 접근함
	// 6. 그러나 해당 CS는 이미 삭제된 이후
	//		=> 문제발생
	// 
	// 하지만 logger 는 전역 객체이기 때문에 소멸자 호출 타이밍은 전체 프로그램이 종료될 때 임
	//  (logger 객체가 전역에 존재한다는 전제이므로 싱글톤이 전제되어야한다 vs 혼자 사용하는데 꼭 필요한가?)
	//		=> 로그를 요청하는 writeReq 자체를 전역을 대상으로 동작하게 강제하여 임시 해결

	loggingThread->join();

	while (!logMessagePool.empty()) {
		delete logMessagePool.top();
		logMessagePool.pop();
	}

	while (!logMessageQueue.empty()) {
		delete logMessageQueue.front();
		logMessageQueue.pop();
	}

	DeleteCriticalSection(&queueCS);
	DeleteCriticalSection(&poolCS);
 }


void logger::loggingThreadProc()
{
	std::queue<logMessage*> localQueue;
	logMessage* msg = nullptr;

	while (runFlag) {
		int ret = WaitForSingleObject(loggingEvent, INFINITE);

		if (ret != WAIT_OBJECT_0) {
			std::cout << "Error WaitForSingleObject : " << GetLastError() << "\t in logging thread" << std::endl;
			dump::crash();
			return;
		}

		do {
			EnterCriticalSection(&queueCS);
			while (!logMessageQueue.empty())
			{
				msg = logMessageQueue.front();
				logMessageQueue.pop();

				localQueue.push(msg);
			}
			ResetEvent(loggingEvent);
			LeaveCriticalSection(&queueCS);

			while (!localQueue.empty()) {
				msg = localQueue.front();
				localQueue.pop();

				if (!writeLog(*msg)) {
					dump::crash();
					return;
				}

				EnterCriticalSection(&poolCS);
				logMessagePool.push(msg);
				LeaveCriticalSection(&poolCS);
			}
		} while (!logMessageQueue.empty());
	}

	return;
}


bool logger::writeLog(const logMessage& msg)
{
	std::ofstream logFile;
	logStructure logBlock(msg);

	std::string loggingDir = logFileDir + "\\" + std::to_string(logBlock.days - days);
	if (!createDirectoryNotExists(loggingDir)) {
		std::cout << "Error opening dir: " << GetLastError() << std::endl << loggingDir << std::endl;
		return false;
	}

	if (msg.writeMode & LO_TXT)
	{
		std::string textLine;
		TEXTstringify(logBlock, textLine);

		//default Logging
		std::string defaultFileName(loggingDir + "defaultLog.txt");
		if (!writeFile(defaultFileName, textLine))
			return false;

		//target File logging
		if (msg.logWriteFileName != ""){
			std::string fileName(loggingDir + msg.logWriteFileName + ".txt");
			if(!writeFile(fileName, textLine))
				return false;
		}
	}
	if (msg.writeMode & LO_CSV)
	{
		std::string csvLine;
		std::string fileName;
		CSVstringify(logBlock, csvLine);

		if (msg.logWriteFileName == "")
			fileName = std::string(loggingDir + "default.csv");
		else
			fileName = std::string(loggingDir + msg.logWriteFileName + ".csv");

		if(!writeFile(fileName, csvLine))
			return false;
	}
	if (msg.writeMode & LO_CMD)
	{
		std::string textLine;
		TEXTstringify(logBlock, textLine);

		std::cout << textLine.c_str();
	}

	return true;
}

bool logger::setLoggingDir(const std::string& _logFileDir) {
	char fullPath[MAX_PATH];
	if(_fullpath(fullPath, _logFileDir.c_str(), MAX_PATH) == nullptr)
		return false;

	logFileDir = std::string(fullPath);

	return true;
}
std::string logger::getLoggingDir() {
	return logFileDir;
}

//bool			setLoggingDBA(const std::string logDBA);
//std::string	getLoggingDBA();

bool logger::setLoggingLevel(const logLevel _loggingLevel) {
	loggingLevel = _loggingLevel;

	return true;
}
logLevel logger::getLoggingLevel() {
	return loggingLevel;
}








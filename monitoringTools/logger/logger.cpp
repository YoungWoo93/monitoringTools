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
bool dirCheck(const std::string& dir)
{
	DWORD attribute = GetFileAttributesA(dir.c_str());

	if (attribute == INVALID_FILE_ATTRIBUTES || !(attribute & FILE_ATTRIBUTE_DIRECTORY))
	{
		std::string subDir(dir.begin(), dir.begin() + dir.rfind('\\'));

		if (dirCheck(subDir))
			return CreateDirectoryA(dir.c_str(), NULL) != 0;
		else
			return false;
	}

	return true;
}


logger::logger(logLevel _loggingLevel) : loggingLevel(_loggingLevel){
	unsigned long long int msTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	std::stringstream dirName;
	dirName << "LOG\\" << msTime;

	setLoggingDir(dirName.str());

	InitializeCriticalSection(&queueCS);
	InitializeCriticalSection(&poolCS);

	for (int i = 0; i < 1000; i++)
		logMessagePool.push(new logMessage);

	loggingEvent = CreateEventA(nullptr, true, false, "loggingEvent");
	if (loggingEvent == (HANDLE)(-1))
	{
		std::cout << "create logging event fail, Error " << GetLastError() << std::endl;
		abort();
	}

	runFlag = true;
	loggingThread = new std::thread(&logger::loggingThreadProc, this);
}
logger::~logger() {
	runFlag = false;
	SetEvent(loggingEvent);
	// �̺κ� ���� �߻� ����
	// 1. �ΰ��� �Ҹ��ڿ��� flag�� ��
	// 2. �α� �����忡�� �α׸� �������� queue���� ȹ���� ���¿���
	// 3. �׷��� ������ �����忡�� ������ �α׸� �������� queue�� ȹ�� �����¿���
	// 4. �α� �����忡���� ���� �ִ� �α׸� �� ������ flag�� Ȯ���ؼ� join ��
	// 5. ������ ������� ������ ���� ȹ���ϱ����� CS�� ������
	// 6. �׷��� �ش� CS�� �̹� ������ ����
	//		=> �����߻�
	// 
	// ������ logger �� ���� ��ü�̱� ������ �Ҹ��� ȣ�� Ÿ�̹��� ��ü ���α׷��� ����� �� ��
	//  (logger ��ü�� ������ �����Ѵٴ� �����̹Ƿ� �̱����� �����Ǿ���Ѵ� vs ȥ�� ����ϴµ� �� �ʿ��Ѱ�?)
	//		=> �α׸� ��û�ϴ� writeReq ��ü�� ������ ������� �����ϰ� �����Ͽ� �ӽ� �ذ�

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
			abort();
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

				if(!writeLog(*msg))
					return;

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
	logStructure LS(msg);

	std::string loggingDir = logFileDir + "\\" + std::to_string(LS.days - days);
	if (!dirCheck(loggingDir)) {
		std::cout << "Error opening dir: " << GetLastError() << std::endl << loggingDir << std::endl;
		abort();
		return false;
	}

	if (msg.writeMode & LO_TXT)
	{
		std::string textLine;
		TEXTstringify(LS, textLine);
		//default Logging
		std::string defaultFileName(loggingDir + "\\defaultLog.txt");
		{
			HANDLE logFile = CreateFileA(
				defaultFileName.c_str(),
				GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);
			if (logFile == INVALID_HANDLE_VALUE) {
				std::cout << "Error opening file: " << GetLastError() << std::endl;
				abort();
				return false;
			}

			SetFilePointer(logFile, 0, NULL, FILE_END);
			DWORD bytesWritten = 0;
			if (!WriteFile(logFile, textLine.c_str(), (DWORD)textLine.size(), &bytesWritten, NULL)) {
				std::cout << "Error writing to file: " << GetLastError() << std::endl;
				abort();
				return false;
			}
			CloseHandle(logFile);
		}


		std::string fileName(loggingDir + "\\" + msg.logWriteFileName + ".txt");
		//target File logging
		if (msg.logWriteFileName != "")
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
				abort();
				return false;
			}

			SetFilePointer(logFile, 0, NULL, FILE_END);
			DWORD bytesWritten = 0;
			if (!WriteFile(logFile, textLine.c_str(), (DWORD)textLine.size(), &bytesWritten, NULL)) {
				std::cout << "Error writing to file: " << GetLastError() << std::endl;
				abort();
			}
			CloseHandle(logFile);
		}
	}
	if (msg.writeMode & LO_CSV)
	{
		std::string csvLine;
		std::string fileName;
		CSVstringify(LS, csvLine);

		if (msg.logWriteFileName == "")
			fileName = std::string(loggingDir + "\\default.csv");
		else
			fileName = std::string(loggingDir + "\\" + msg.logWriteFileName + ".csv");

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
			abort();
			return false;
		}

		SetFilePointer(logFile, 0, NULL, FILE_END);
		DWORD bytesWritten = 0;
		if (!WriteFile(logFile, csvLine.c_str(), (DWORD)csvLine.size(), &bytesWritten, NULL)) {
			std::cout << "Error writing to file: " << GetLastError() << std::endl;
			abort();
			return false;
		}
		CloseHandle(logFile);
	}
	if (msg.writeMode & LO_CMD)
	{
		std::string textLine;

		TEXTstringify(LS, textLine);
		std::cout << textLine.c_str();
	}

	return true;
}

bool			logger::setLoggingDir(const std::string& _logFileDir) {
	char fullPath[MAX_PATH];
	_fullpath(fullPath, _logFileDir.c_str(), MAX_PATH);

	logFileDir = std::string(fullPath);

	return true;
}
std::string		logger::getLoggingDir() {
	return logFileDir;
}

//bool			setLoggingDBA(const std::string logDBA);
//std::string	getLoggingDBA();

bool			logger::setLoggingLevel(const logLevel _loggingLevel) {
	loggingLevel = _loggingLevel;

	return true;
}
logLevel		logger::getLoggingLevel() {
	return loggingLevel;
}







